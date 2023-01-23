/*
 * winfrip_feature_bgimg.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "dialog.h"
#include "windows/win-gui-seat.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_feature_bgimg.h"
#include "PuTTie/winfrip_rtl.h"

#include <bcrypt.h>
#include <ntstatus.h>

#include <gdiplus/gdiplus.h>
#include <gdiplus/gdiplusflat.h>

/*
 * Private preprocessor macros
 */

#define WFFBP_FILTER_IMAGE_FILES (										\
	"All Picture Files\0*.bmp;*.emf;*.gif;*.ico;*.jpg;*.jpeg;*.jpe;*.jfif;*.png;*.tif;*.tiff;*.wmf\0"	\
	"Bitmap Files (*.bmp)\0*.bmp\0"										\
	"EMF (*.emf)\0*.emf\0"											\
	"GIF (*.gif)\0*.gif\0"											\
	"ICO (*.ico)\0*.ico\0"											\
	"JPEG (*.jpg;*.jpeg;*.jpe;*.jfif)\0*.jpg;*.jpeg;*.jpe;*.jfif\0"						\
	"PNG (*.png)\0*.png\0"											\
	"TIFF (*.tif;*.tiff)\0*.tif;*.tiff\0"									\
	"WMF (*.wmf)\0*.wmf\0"											\
	"All Files (*.*)\0*\0"											\
	"\0\0")

/*
 * Private type definitions
 */

typedef struct WffbpContext {
	Conf *	conf;
} WffbpContext;

typedef enum WffbpState {
	WFFBP_STATE_NONE		= 0,
	WFFBP_STATE_FAILED		= 1,
	WFFBP_STATE_INIT		= 2,
} WffbpState;

typedef enum WffbpSlideshow {
	WFFBP_SLIDESHOW_SINGLE_IMAGE	= 0,
	WFFBP_SLIDESHOW_DEFAULT		= WFFBP_SLIDESHOW_SINGLE_IMAGE,
	WFFBP_SLIDESHOW_SHUFFLE		= 1,
} WffbpSlideshow;

typedef enum WffbpStyle {
	WFFBP_STYLE_ABSOLUTE		= 0,
	WFFBP_STYLE_DEFAULT		= WFFBP_STYLE_ABSOLUTE,
	WFFBP_STYLE_CENTER		= 1,
	WFFBP_STYLE_FIT			= 2,
	WFFBP_STYLE_STRETCH		= 3,
	WFFBP_STYLE_TILE		= 4,
} WffbpStyle;

typedef enum WffbpType {
	WFFBP_TYPE_SOLID		= 0,
	WFFBP_TYPE_DEFAULT		= WFFBP_TYPE_SOLID,
	WFFBP_TYPE_IMAGE		= 1,
} WffbpType;

/*
 * External variables
 */

// window.c
extern WinGuiSeat			wgs;

/*
 * Private variables
 */

static char *			WffbpDname = NULL;
static size_t			WffbpDnameLen = 0;
static size_t			WffbpDnameFileC = 0;
static char **			WffbpDnameFileV = NULL;
static char *			WffbpFname = NULL;

static BCRYPT_ALG_HANDLE	WffbhAlgorithm = NULL;

static HDC			WffbphDC = NULL;
static HGDIOBJ			WffbphDCOld = NULL;

static WffbpState		WffbpStateCurrent = WFFBP_STATE_NONE;

static WffbpContext *		WffbpTimerContext = NULL;

/*
 * External subroutine prototypes
 */

// window.c
void		reset_window(int);

/*
 * Private subroutine prototypes
 */

static void	WffbpConfigPanelSlideshow(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static void	WffbpConfigPanelStyle(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static void	WffbpConfigPanelType(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);

static BOOL	WffbpSetGetFname(Conf *conf, BOOL reshuffle, wchar_t **pbg_bmp_fname_w, BOOL *pbg_bmpfl);
static BOOL	WffbpSetLoadBmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc);
static BOOL	WffbpSetLoadNonBmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc);
static BOOL	WffbpSetProcess(HDC bg_hdc, int bg_height, int bg_width, HBITMAP bmp_src, Conf *conf, HDC hdc);
static BOOL	WffbpSetProcessBlend(HDC bg_hdc, int bg_height, int bg_width, Conf *conf);
static BOOL	WffbpSet(Conf *conf, HDC hdc, BOOL force, BOOL reshuffle);

static void	WffbpSlideshowReconf(Conf *conf);
static BOOL	WffbpSlideshowShuffle(void);

static void	WffbpTimerFunction(void *ctx, unsigned long now);

/*
 * Private subroutines
 */

static void
WffbpConfigPanelSlideshow(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	void *		data,
	int		event
	)
{
	Conf *	conf = (Conf *)data;


	switch (event) {
	case EVENT_REFRESH:
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);
		dlg_listbox_addwithid(ctrl, dlg, "Single image", WFFBP_SLIDESHOW_SINGLE_IMAGE);
		dlg_listbox_addwithid(ctrl, dlg, "Melbourne shuffle!", WFFBP_SLIDESHOW_SHUFFLE);

		switch (conf_get_int(conf, CONF_frip_bgimg_slideshow)) {
		case WFFBP_SLIDESHOW_SINGLE_IMAGE:
			dlg_listbox_select(ctrl, dlg, 0); break;
		case WFFBP_SLIDESHOW_SHUFFLE:
			dlg_listbox_select(ctrl, dlg, 1); break;
		default:
			WFR_DEBUG_FAIL(); break;
		}
		dlg_update_done(ctrl, dlg);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		conf_set_int(
			conf, CONF_frip_bgimg_slideshow,
			dlg_listbox_getid(ctrl, dlg,
			dlg_listbox_index(ctrl, dlg)));
		break;
	}
}

static void
WffbpConfigPanelStyle(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	void *		data,
	int		event
	)
{
	Conf *	conf = (Conf *)data;


	switch (event) {
	case EVENT_REFRESH:
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);
		dlg_listbox_addwithid(ctrl, dlg, "Absolute", WFFBP_STYLE_ABSOLUTE);
		dlg_listbox_addwithid(ctrl, dlg, "Center", WFFBP_STYLE_CENTER);
		dlg_listbox_addwithid(ctrl, dlg, "Fit", WFFBP_STYLE_FIT);
		dlg_listbox_addwithid(ctrl, dlg, "Stretch", WFFBP_STYLE_STRETCH);
		dlg_listbox_addwithid(ctrl, dlg, "Tile", WFFBP_STYLE_TILE);

		switch (conf_get_int(conf, CONF_frip_bgimg_style)) {
		case WFFBP_STYLE_ABSOLUTE:
			dlg_listbox_select(ctrl, dlg, 0); break;
		case WFFBP_STYLE_CENTER:
			dlg_listbox_select(ctrl, dlg, 1); break;
		case WFFBP_STYLE_FIT:
			dlg_listbox_select(ctrl, dlg, 2); break;
		case WFFBP_STYLE_STRETCH:
			dlg_listbox_select(ctrl, dlg, 3); break;
		case WFFBP_STYLE_TILE:
			dlg_listbox_select(ctrl, dlg, 4); break;
		default:
			WFR_DEBUG_FAIL(); break;
		}
		dlg_update_done(ctrl, dlg);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		conf_set_int(
			conf, CONF_frip_bgimg_style,
			dlg_listbox_getid(ctrl, dlg,
			dlg_listbox_index(ctrl, dlg)));
		break;
	}
}

static void
WffbpConfigPanelType(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	void *		data,
	int		event
	)
{
	Conf *	conf = (Conf *)data;


	switch (event) {
	case EVENT_REFRESH:
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);
		dlg_listbox_addwithid(ctrl, dlg, "Solid", WFFBP_TYPE_SOLID);
		dlg_listbox_addwithid(ctrl, dlg, "Image", WFFBP_TYPE_IMAGE);

		switch (conf_get_int(conf, CONF_frip_bgimg_type)) {
		case WFFBP_TYPE_SOLID:
			dlg_listbox_select(ctrl, dlg, 0); break;
		case WFFBP_TYPE_IMAGE:
			dlg_listbox_select(ctrl, dlg, 1); break;
		default:
			WFR_DEBUG_FAIL(); break;
		}
		dlg_update_done(ctrl, dlg);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		conf_set_int(
			conf, CONF_frip_bgimg_type,
			dlg_listbox_getid(ctrl, dlg,
			dlg_listbox_index(ctrl, dlg)));
		break;
	}
}

static BOOL
WffbpSetGetFname(
	Conf *		conf,
	BOOL		reshuffle,
	wchar_t **	pbg_fname_w,
	BOOL *		pbg_bmpfl
	)
{
	char *		bg_fname = NULL;
	Filename *	bg_fname_conf;
	size_t		bg_fname_len;

	WfrStatus	status;


	switch (conf_get_int(conf, CONF_frip_bgimg_slideshow)) {
	default:
		WFR_DEBUG_FAIL(); break;

	case WFFBP_SLIDESHOW_SINGLE_IMAGE:
		bg_fname_conf = conf_get_filename(conf, CONF_frip_bgimg_filename);
		bg_fname = bg_fname_conf->path;
		break;

	case WFFBP_SLIDESHOW_SHUFFLE:
		if (reshuffle || !WffbpFname) {
			if (!WffbpSlideshowShuffle()) {
				WFR_DEBUG_FAIL();
				return FALSE;
			}
		}
		bg_fname = WffbpFname;
		break;
	}

	if (bg_fname) {
		bg_fname_len = strlen(bg_fname);
		if (bg_fname_len > (sizeof(".bmp")-1)) {
			if ((bg_fname[bg_fname_len-4] == '.')
			&&  (tolower(bg_fname[bg_fname_len-3]) == 'b')
			&&  (tolower(bg_fname[bg_fname_len-2]) == 'm')
			&&  (tolower(bg_fname[bg_fname_len-1]) == 'p')) {
				*pbg_bmpfl = TRUE;
			} else {
				*pbg_bmpfl = FALSE;
			}
			status = WfrToWcsDup(bg_fname, bg_fname_len + 1, pbg_fname_w);
			if (WFR_STATUS_SUCCESS(status)) {
				switch (conf_get_int(conf, CONF_frip_bgimg_slideshow)) {
				case WFFBP_SLIDESHOW_SHUFFLE:
					if (bg_fname && (bg_fname != WffbpFname)) {
						sfree(bg_fname);
					}
					break;
				}
			}

			return WFR_STATUS_SUCCESS(status);
		}
	}

	switch (conf_get_int(conf, CONF_frip_bgimg_slideshow)) {
	case WFFBP_SLIDESHOW_SHUFFLE:
		if (bg_fname && (bg_fname != WffbpFname)) {
			sfree(bg_fname);
		}
		break;
	}

	return FALSE;
}

static BOOL
WffbpSetLoadBmp(
	HDC *		pbg_hdc,
	HGDIOBJ *	pbg_hdc_old,
	int *		pbg_height,
	int *		pbg_width,
	HBITMAP *	pbmp_src,
	wchar_t *	bmp_src_fname_w,
	HDC		hdc
	)
{
	HBITMAP		bg_bmp = NULL;
	HDC		bg_hdc = NULL;
	HGDIOBJ		bg_hdc_old = NULL;
	int		bg_height, bg_width;
	HBITMAP		bmp_src = NULL;
	BOOL		rc = FALSE;


	bg_height = GetDeviceCaps(hdc, VERTRES);
	bg_width = GetDeviceCaps(hdc, HORZRES);
	if ((bmp_src = LoadImageW(0, bmp_src_fname_w, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE))) {
		if ((bg_hdc = CreateCompatibleDC(hdc))) {
			if ((bg_bmp = CreateCompatibleBitmap(hdc, bg_width, bg_height))) {
				bg_hdc_old = SelectObject(bg_hdc, bg_bmp);
				rc = TRUE;
			}
		}
	}

	*pbg_hdc = bg_hdc; *pbg_hdc_old = bg_hdc_old;
	*pbg_height = bg_height; *pbg_width = bg_width;
	*pbmp_src = bmp_src;

	return rc;
}

static BOOL
WffbpSetLoadNonBmp(
	HDC *		pbg_hdc,
	HGDIOBJ *	pbg_hdc_old,
	int *		pbg_height,
	int *		pbg_width,
	HBITMAP *	pbmp_src,
	wchar_t *	bmp_src_fname_w,
	HDC		hdc
	)
{
	HBITMAP			bg_bmp = NULL;
	HDC			bg_hdc = NULL;
	HGDIOBJ			bg_hdc_old = NULL;
	int			bg_height, bg_width;
	HBITMAP			bmp_src = NULL;

	GpBitmap *		gdip_bmp;
	GdiplusStartupInput	gdip_si;
	GpStatus		gdip_status;
	ULONG_PTR		gdip_token;

	BOOL			rc = FALSE;


	gdip_si.GdiplusVersion = 2;
	gdip_si.DebugEventCallback = NULL;
	gdip_si.SuppressBackgroundThread = FALSE;
	gdip_si.SuppressExternalCodecs = FALSE;
	if ((gdip_status = GdiplusStartup(&gdip_token, &gdip_si, NULL)) != Ok) {
		WFR_DEBUG_FAIL();
		return FALSE;
	} else if ((gdip_status = GdipCreateBitmapFromFile(bmp_src_fname_w, &gdip_bmp)) != Ok) {
		WFR_DEBUG_FAIL();
		GdiplusShutdown(gdip_token);
		return FALSE;
	} else {
		gdip_status = GdipCreateHBITMAPFromBitmap(gdip_bmp, &bmp_src, 0);
		GdipDisposeImage(gdip_bmp);
		GdiplusShutdown(gdip_token);
		if (gdip_status != Ok) {
			WFR_DEBUG_FAIL();
			return FALSE;
		}
	}

	bg_height = GetDeviceCaps(hdc, VERTRES);
	bg_width = GetDeviceCaps(hdc, HORZRES);
	if ((bg_hdc = CreateCompatibleDC(hdc))) {
		if ((bg_bmp = CreateCompatibleBitmap(hdc, bg_width, bg_height))) {
			bg_hdc_old = SelectObject(bg_hdc, bg_bmp);
			rc = TRUE;
		}
	}

	*pbg_hdc = bg_hdc; *pbg_hdc_old = bg_hdc_old;
	*pbg_height = bg_height; *pbg_width = bg_width;
	*pbmp_src = bmp_src;

	return rc;
}

static BOOL
WffbpSetProcess(
	HDC		bg_hdc,
	int		bg_height,
	int		bg_width,
	HBITMAP		bmp_src,
	Conf *		conf,
	HDC		hdc
	)
{
	HBRUSH		bg_brush;
	int		bg_hdc_sb_mode;
	RECT		bg_rect, cr;
	int		bgimg_style;

	int		bmp_offset_x, bmp_offset_y;
	HDC		bmp_src_hdc;
	BITMAP		bmp_src_obj;
	HGDIOBJ		bmp_src_old;

	int		cr_height, cr_width;

	HWND		hwnd;

	int		padding;
	float		ratio, ratioH, ratioW;

	BOOL		rc = FALSE;

	int		x, y;


	SetRect(&bg_rect, 0, 0, bg_width, bg_height);
	bgimg_style = conf_get_int(conf, CONF_frip_bgimg_style);
	switch (bgimg_style) {
	default:
		WFR_DEBUG_FAIL(); break;

	case WFFBP_STYLE_ABSOLUTE:
	case WFFBP_STYLE_CENTER:
	case WFFBP_STYLE_FIT:
	case WFFBP_STYLE_STRETCH:
		if ((bmp_src_hdc = CreateCompatibleDC(bg_hdc))) {
			GetObject(bmp_src, sizeof(bmp_src_obj), &bmp_src_obj);
			bmp_src_old = SelectObject(bmp_src_hdc, bmp_src);

			switch (bgimg_style) {
			default:
				WFR_DEBUG_FAIL(); break;

			case WFFBP_STYLE_ABSOLUTE:
				rc = BitBlt(
					bg_hdc, bg_rect.left, bg_rect.top,
					bmp_src_obj.bmWidth, bmp_src_obj.bmHeight,
					bmp_src_hdc, 0, 0, SRCCOPY) > 0;
				break;

			case WFFBP_STYLE_CENTER:
				hwnd = WindowFromDC(hdc);
				GetClientRect(hwnd, &cr);
				cr_height = (cr.bottom - cr.top), cr_width = (cr.right - cr.left);

				if (cr_width > bmp_src_obj.bmWidth) {
					bmp_offset_x = (cr_width - bmp_src_obj.bmWidth) / 2;
					bg_rect.left += bmp_offset_x; bg_rect.right -= bmp_offset_x;
				}
				if (cr_height > bmp_src_obj.bmHeight) {
					bmp_offset_y = (cr_height - bmp_src_obj.bmHeight) / 2;
					bg_rect.top += bmp_offset_y; bg_rect.bottom -= bmp_offset_y;
				}
				rc = BitBlt(
					bg_hdc, bg_rect.left, bg_rect.top,
					bg_rect.right - bg_rect.left,
					bg_rect.bottom - bg_rect.top,
					bmp_src_hdc, 0, 0, SRCCOPY) > 0;
				break;

			case WFFBP_STYLE_FIT:
				hwnd = WindowFromDC(hdc);
				GetClientRect(hwnd, &cr);
				cr_height = (cr.bottom - cr.top), cr_width = (cr.right - cr.left);

				ratioH = (float)cr_height / (float)bmp_src_obj.bmHeight;
				ratioW = (float)cr_width / (float)bmp_src_obj.bmWidth;
				if ((padding = (conf_get_int(conf, CONF_frip_bgimg_padding) % 100)) > 0) {
					ratioH -= ((float)padding / 100) * ratioH;
					ratioW -= ((float)padding / 100) * ratioW;
				}
				ratio = ratioH < ratioW ? ratioH : ratioW;

				x = (int)(((float)cr_width - ((float)bmp_src_obj.bmWidth * ratio)) / 2);
				y = (int)(((float)cr_height - ((float)bmp_src_obj.bmHeight * ratio)) / 2);

				bg_hdc_sb_mode = SetStretchBltMode(bg_hdc, HALFTONE);
				rc = StretchBlt(
					bg_hdc, x, y,
					(int)((float)bmp_src_obj.bmWidth * ratio),
					(int)((float)bmp_src_obj.bmHeight * ratio),
					bmp_src_hdc,
					0, 0,
					bmp_src_obj.bmWidth, bmp_src_obj.bmHeight,
					SRCCOPY) > 0;
				SetStretchBltMode(bg_hdc, bg_hdc_sb_mode);
				break;

			case WFFBP_STYLE_STRETCH:
				bg_hdc_sb_mode = SetStretchBltMode(bg_hdc, HALFTONE);
				rc = StretchBlt(
					bg_hdc, bg_rect.left, bg_rect.top,
					bg_rect.right - bg_rect.left,
					bg_rect.bottom - bg_rect.top,
					bmp_src_hdc, 0, 0,
					bmp_src_obj.bmWidth, bmp_src_obj.bmHeight, SRCCOPY) > 0;
				SetStretchBltMode(bg_hdc, bg_hdc_sb_mode);
				break;
			}

			SelectObject(bmp_src_hdc, bmp_src_old);
			DeleteDC(bmp_src_hdc);
		}
		break;

	case WFFBP_STYLE_TILE:
		if ((bg_brush = CreatePatternBrush(bmp_src))) {
			rc = FillRect(bg_hdc, &bg_rect, bg_brush) > 0;
			DeleteObject(bg_brush);
		}
		break;
	}

	return rc;
}

static BOOL
WffbpSetProcessBlend(
	HDC	bg_hdc,
	int	bg_height,
	int	bg_width,
	Conf *	conf
	)
{
	RECT		bg_rect;
	HBITMAP		blend_bmp = NULL;
	COLORREF	blend_colour;
	BLENDFUNCTION	blend_ftn;
	HDC		blend_hdc = NULL;
	HGDIOBJ		blend_old = NULL;
	BOOL		rc = FALSE;


	if ((blend_hdc = CreateCompatibleDC(bg_hdc))) {
		if ((blend_bmp = CreateCompatibleBitmap(bg_hdc, 1, 1))) {
			blend_old = SelectObject(blend_hdc, blend_bmp);
			blend_colour = RGB(0, 0, 0);
			SetPixelV(blend_hdc, 0, 0, blend_colour);

			SetRect(&bg_rect, 0, 0, bg_width, bg_height);
			blend_ftn.AlphaFormat = 0;
			blend_ftn.BlendFlags = 0;
			blend_ftn.BlendOp = AC_SRC_OVER;
			blend_ftn.SourceConstantAlpha = (0xff * conf_get_int(conf, CONF_frip_bgimg_opacity)) / 100;
			rc = AlphaBlend(
				bg_hdc,
				bg_rect.left, bg_rect.top,
				bg_rect.right - bg_rect.left,
				bg_rect.bottom - bg_rect.top,
				blend_hdc, 0, 0, 1, 1, blend_ftn) > 0;
		}
	}

	if (blend_hdc) {
		if (blend_old) {
			SelectObject(blend_hdc, blend_old);
		}
		DeleteDC(blend_hdc);
	}
	if (blend_bmp) {
		DeleteObject(blend_bmp);
	}

	return rc;
}

static BOOL
WffbpSet(
	Conf *	conf,
	HDC	hdc,
	BOOL	force,
	BOOL	reshuffle
	)
{
	wchar_t *	bg_bmp_fname_w;
	BOOL		bg_bmpfl;
	HDC		bg_hdc = NULL;
	HGDIOBJ		bg_hdc_cur, bg_hdc_old = NULL;
	int		bg_height, bg_width;
	HBITMAP		bmp_src = NULL;
	BOOL		rc = FALSE;


	switch (WffbpStateCurrent) {
	default:
		WFR_DEBUG_FAIL(); break;

	case WFFBP_STATE_NONE:
		break;

	case WFFBP_STATE_FAILED:
		if (!force) {
			WFR_DEBUG_FAIL();
			return FALSE;
		} else {
			break;
		}

	case WFFBP_STATE_INIT:
		if (!force) {
			return TRUE;
		} else {
			break;
		}
	}

	if (WffbphDC) {
		bg_hdc_cur = GetCurrentObject(WffbphDC, OBJ_BITMAP);
		SelectObject(WffbphDC, WffbphDCOld); WffbphDCOld = NULL;
		DeleteObject(bg_hdc_cur);
		DeleteDC(WffbphDC); WffbphDC = NULL;
	}

	if ((WffbpSetGetFname(conf, reshuffle, &bg_bmp_fname_w, &bg_bmpfl))) {
		switch (bg_bmpfl) {
		case TRUE:
			rc = WffbpSetLoadBmp(&bg_hdc, &bg_hdc_old, &bg_height, &bg_width, &bmp_src, bg_bmp_fname_w, hdc);
			break;

		default:
		case FALSE:
			rc = WffbpSetLoadNonBmp(&bg_hdc, &bg_hdc_old, &bg_height, &bg_width, &bmp_src, bg_bmp_fname_w, hdc);
			break;
		}
		sfree(bg_bmp_fname_w);
	}

	if (rc) {
		if (WffbpSetProcess(bg_hdc, bg_height, bg_width, bmp_src, conf, hdc)) {
			if (WffbpSetProcessBlend(bg_hdc, bg_height, bg_width, conf)) {
				DeleteObject(bmp_src);
				WffbphDC = bg_hdc; WffbphDCOld = bg_hdc_old;
				WffbpStateCurrent = WFFBP_STATE_INIT;
				return TRUE;
			}
		}
	}

	if (bg_hdc) {
		if (bg_hdc_old) {
			bg_hdc_cur = GetCurrentObject(bg_hdc, OBJ_BITMAP);
			SelectObject(bg_hdc, bg_hdc_old);
			DeleteObject(bg_hdc_cur);
		}
		DeleteDC(bg_hdc);
	}
	if (bmp_src) {
		DeleteObject(bmp_src);
	}
	WffbpStateCurrent = WFFBP_STATE_FAILED;
	WFR_DEBUG_FAIL();

	return FALSE;
}

static void
WffbpSlideshowReconf(
	Conf *	conf
	)
{
	char *			bg_dname;
	Filename *		bg_dname_conf;
	size_t			bg_dname_len;

	size_t			bg_fname_len;

	size_t			dname_filec_new = 0;
	char **			dname_filev_new = NULL;

	char *			dname_new = NULL;
	WIN32_FIND_DATA		dname_new_ffd;
	HANDLE			dname_new_hFind = INVALID_HANDLE_VALUE;

	char *			p, **pp;

	WffbpContext *		timer_ctx_new = NULL;


	switch (conf_get_int(conf, CONF_frip_bgimg_slideshow)) {
	default:
		WFR_DEBUG_FAIL(); break;

	case WFFBP_SLIDESHOW_SINGLE_IMAGE:
		WFR_SFREE_IF_NOTNULL(WffbpDname);
		if (WffbpDnameFileV) {
			for (size_t nfile = 0; nfile < WffbpDnameFileC; nfile++) {
				WFR_SFREE_IF_NOTNULL(WffbpDnameFileV[nfile]);
			}

			sfree(WffbpDnameFileV);
			WFR_SFREE_IF_NOTNULL(WffbpFname); WffbpFname = NULL;
		}

		WffbpDname = NULL;
		WffbpDnameLen = 0;
		WffbpDnameFileC = 0;
		WffbpDnameFileV = NULL;

		if (WffbpTimerContext) {
			expire_timer_context(WffbpTimerContext);
			sfree(WffbpTimerContext);
			WffbpTimerContext = NULL;
		}
		break;

	case WFFBP_SLIDESHOW_SHUFFLE:
		bg_dname_conf = conf_get_filename(conf, CONF_frip_bgimg_filename);
		bg_dname = bg_dname_conf->path;
		bg_dname_len = strlen(bg_dname);

		if (bg_dname_len == 0) {
			goto fail;
		} else {
			for (ssize_t nch = (bg_dname_len - 1); nch >= 0; nch--) {
				if ((bg_dname[nch] == '\\') || (bg_dname[nch] == '/')) {
					bg_dname_len = nch; break;
				}
			}
		}

		if (!(dname_filev_new = snewn(1, char *))
		||  !(dname_new = snewn(bg_dname_len + (sizeof("\\*") - 1) + 1, char))) {
			goto fail;
		} else {
			strncpy(dname_new, bg_dname, bg_dname_len); dname_new[bg_dname_len] = '\0'; strcat(dname_new, "\\*");
			dname_new_hFind = FindFirstFile(dname_new, &dname_new_ffd);
			if (dname_new_hFind != INVALID_HANDLE_VALUE) {
				do {
					if (((dname_new_ffd.dwFileAttributes & (FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_DIRECTORY)) == FILE_ATTRIBUTE_ARCHIVE)
					&&  (!((dname_new_ffd.cFileName[0] == '.') && (dname_new_ffd.cFileName[1] == '\0')))) {
						bg_fname_len = strlen(dname_new_ffd.cFileName);
						if (!(pp = sresize(dname_filev_new, dname_filec_new + 1 + 1, char *))) {
							goto fail;
						} else {
							dname_filec_new += 1, dname_filev_new = pp;
							if (!(p = snewn(bg_fname_len + 1, char))) {
								goto fail;
							} else {
								strncpy(p, dname_new_ffd.cFileName, bg_fname_len + 1); p[bg_fname_len] = '\0';
								dname_filev_new[dname_filec_new - 1] = p;
								dname_filev_new[dname_filec_new] = NULL;
							}
						}
					}
				} while (FindNextFile(dname_new_hFind, &dname_new_ffd) != 0);

				WFR_SFREE_IF_NOTNULL(WffbpDname);
				if (WffbpDnameFileV) {
					for (size_t nfile = 0; nfile < WffbpDnameFileC; nfile++) {
						WFR_SFREE_IF_NOTNULL(WffbpDnameFileV[nfile]);
					}
					sfree(WffbpDnameFileV);
				}

				WffbpDname = dname_new;
				WffbpDnameLen = bg_dname_len;
				WffbpDnameFileC = dname_filec_new;
				WffbpDnameFileV = dname_filev_new;

				if (!(timer_ctx_new = snew(WffbpContext))) {
					goto fail;
				} else if (WffbpTimerContext) {
					expire_timer_context(WffbpTimerContext);
					sfree(WffbpTimerContext);
				}
				WffbpTimerContext = timer_ctx_new;
				WffbpTimerContext->conf = conf;

				schedule_timer(
					conf_get_int(conf, CONF_frip_bgimg_slideshow_freq) * 1000,
					WffbpTimerFunction, (void *)WffbpTimerContext);
			} else {
				WFR_DEBUG_FAIL();
			}
		}
		break;
	}

	return;

fail:
	if (dname_filev_new) {
		for (size_t nfile = 0; nfile < dname_filec_new; nfile++) {
			WFR_SFREE_IF_NOTNULL(dname_filev_new[nfile]);
		}

		sfree(dname_filev_new);
	}
	WFR_SFREE_IF_NOTNULL(dname_new);
	WFR_SFREE_IF_NOTNULL(timer_ctx_new);

	WFR_DEBUG_FAIL();
}

static BOOL
WffbpSlideshowShuffle(
	void
	)
{
	char *		bg_fname = NULL;
	size_t		bg_fname_idx, bg_fname_len;
	NTSTATUS	status;


	if ((WffbpDnameFileC > 0) && (WffbpDnameFileV != NULL)) {
		if (WffbhAlgorithm == NULL) {
			if ((status = BCryptOpenAlgorithmProvider(
					&WffbhAlgorithm,
					BCRYPT_RNG_ALGORITHM, NULL, 0)) != STATUS_SUCCESS) {
				WFR_DEBUG_FAIL();
				return FALSE;
			}
		}
		if ((status = BCryptGenRandom(
				WffbhAlgorithm, (PUCHAR)&bg_fname_idx,
				sizeof(bg_fname_idx), 0) != STATUS_SUCCESS)) {
			WFR_DEBUG_FAIL();
			return FALSE;
		} else {
			bg_fname_idx %= WffbpDnameFileC;
			bg_fname_len = strlen(
				WffbpDnameFileV[bg_fname_idx]
				? WffbpDnameFileV[bg_fname_idx] : "");
		}

		if ((WffbpDnameLen == 0) || (WffbpDnameFileV[bg_fname_idx] == NULL) || (bg_fname_len == 0)) {
			WFR_DEBUG_FAIL();
			return FALSE;
		} else if (!(bg_fname = snewn(WffbpDnameLen + 1 + bg_fname_len + 1, char))) {
			WFR_DEBUG_FAIL();
			return FALSE;
		} else {
			snprintf(
				bg_fname, WffbpDnameLen + 1 + bg_fname_len + 1, "%*.*s\\%s",
				(int)WffbpDnameLen, (int)WffbpDnameLen, WffbpDname,
				WffbpDnameFileV[bg_fname_idx]);

			WFR_SFREE_IF_NOTNULL(WffbpFname);
			WffbpFname = dupstr(bg_fname);
			return TRUE;
		}
	} else {
		WFR_DEBUG_FAIL();
		return FALSE;
	}
}

static void
WffbpTimerFunction(
	void *		ctx,
	unsigned long	now
	)
{
	WffbpContext *	context = (WffbpContext *)ctx;
	HDC		hDC;


	(void)now;

	schedule_timer(
		conf_get_int(context->conf, CONF_frip_bgimg_slideshow_freq) * 1000,
		WffbpTimerFunction, ctx);
	hDC = GetDC(wgs.term_hwnd);
	WffbpSet(context->conf, hDC, TRUE, TRUE);
	ReleaseDC(wgs.term_hwnd, hDC);
	InvalidateRect(wgs.term_hwnd, NULL, TRUE);
	reset_window(2);
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
WffBgImgConfigPanel(
	struct controlbox *	b
	)
{
	dlgcontrol *		c;
	struct controlset *	s_bgimg_settings, *s_bgimg_params, *s_slideshow;


	/*
	 * The Frippery: background panel.
	 */

	ctrl_settitle(b, "Frippery/Background", "Configure pointless frippery: background image");

	/*
	 * The Frippery: Background image settings controls box.
	 */

	s_bgimg_settings = ctrl_getset(b, "Frippery/Background", "frip_bgimg_settings", "Background image settings");
	c = ctrl_filesel(
		s_bgimg_settings, "Image file/directory:", 'i',
		 WFFBP_FILTER_IMAGE_FILES, FALSE, "Select background image file/directory",
		 WFP_HELP_CTX, conf_filesel_handler, I(CONF_frip_bgimg_filename));
	c->fileselect.just_button = false;
	ctrl_text(
		s_bgimg_settings,
		"In order to select an image directory for slideshows, select an arbitrary file inside the directory in question.",
		WFP_HELP_CTX);
	ctrl_droplist(s_bgimg_settings, "Type:", 't', 45, WFP_HELP_CTX, WffbpConfigPanelType, P(NULL));
	ctrl_droplist(s_bgimg_settings, "Style:", 's', 45, WFP_HELP_CTX, WffbpConfigPanelStyle, P(NULL));

	/*
	 * The Frippery: Background image parameters control box.
	 */

	s_bgimg_params = ctrl_getset(b, "Frippery/Background", "frip_bgimg_params", "Background image parameters");
	ctrl_editbox(s_bgimg_params, "Opacity (0-100):", 'p', 15, WFP_HELP_CTX, conf_editbox_handler, I(CONF_frip_bgimg_opacity), ED_INT);
	ctrl_editbox(s_bgimg_params, "Fit padding (0-100):", 'n', 15, WFP_HELP_CTX, conf_editbox_handler, I(CONF_frip_bgimg_padding), ED_INT);

	/*
	 * The Frippery: Slideshow settings control box.
	 */

	s_slideshow = ctrl_getset(b, "Frippery/Background", "frip_bgimg_slideshow", "Slideshow settings");
	ctrl_droplist(s_slideshow, "Slideshow:", 'd', 45, WFP_HELP_CTX, WffbpConfigPanelSlideshow, P(NULL));
	ctrl_editbox(s_slideshow, "Slideshow frequency (in seconds):", 'f', 20, WFP_HELP_CTX, conf_editbox_handler, I(CONF_frip_bgimg_slideshow_freq), ED_INT);
}

/*
 * Public subroutines
 */

WfReturn
WffBgImgOperation(
	WffBgImgOp	op,
	BOOL *		pbgfl,
	Conf *		conf,
	HDC		hdc_in,
	HWND		hwnd,
	int		char_width,
	int		font_height,
	int		len,
	int		nbg,
	int		rc_width,
	int		x,
	int		y
	)
{
	Filename *	bg_fname_conf;
	HDC		hdc;
	WfReturn	rc;


	if (conf_get_int(conf, CONF_frip_bgimg_type) != WFFBP_TYPE_IMAGE) {
		return WF_RETURN_CONTINUE;
	}
	if (!(bg_fname_conf = conf_get_filename(conf, CONF_frip_bgimg_filename))) {
		return WF_RETURN_CONTINUE;
	} else if (!bg_fname_conf->path || (strlen(bg_fname_conf->path) == 0)) {
		return WF_RETURN_CONTINUE;
	}

	if (hdc_in) {
		hdc = hdc_in;
	} else if (!hdc_in) {
		hdc = GetDC(hwnd);
	}

	switch (op) {
	default:
		WFR_DEBUG_FAIL();
		rc = WF_RETURN_FAILURE;
		goto out;

	case WFF_BGIMG_OP_DRAW:
		if ((nbg == 258) || (nbg == 259)) {
			if (WffbphDC || WffbpSet(conf, hdc, FALSE, TRUE)) {
				if (rc_width > 0) {
					rc = BitBlt(
						hdc, x, y, rc_width, font_height,
						WffbphDC, x, y, SRCCOPY) > 0;
				} else {
					rc = BitBlt(
						hdc, x, y, char_width * len,
						font_height, WffbphDC, x, y, SRCCOPY) > 0;
				}
				if (rc) {
					*pbgfl = TRUE;
					rc = WF_RETURN_CONTINUE;
					goto out;
				} else {
					*pbgfl = FALSE;
					WFR_DEBUG_FAIL();
					rc = WF_RETURN_FAILURE;
					goto out;
				}
			} else {
				WFR_DEBUG_FAIL();
				rc = WF_RETURN_FAILURE;
				goto out;
			}
		} else {
			rc = WF_RETURN_CONTINUE;
			goto out;
		}

	case WFF_BGIMG_OP_INIT:
		WffbpSlideshowReconf(conf);
		rc = WF_RETURN_CONTINUE;
		break;

	case WFF_BGIMG_OP_RECONF:
		WffbpSlideshowReconf(conf);
		if (WffbpSet(conf, hdc, TRUE, TRUE)) {
			rc = WF_RETURN_CONTINUE;
			goto out;
		} else {
			WFR_DEBUG_FAIL();
			rc = WF_RETURN_FAILURE;
			goto out;
		}

	case WFF_BGIMG_OP_SIZE:
		switch (conf_get_int(conf, CONF_frip_bgimg_style)) {
		default:
			WFR_DEBUG_FAIL();
			rc = WF_RETURN_FAILURE;
			goto out;

		case WFFBP_STYLE_ABSOLUTE:
		case WFFBP_STYLE_TILE:
			rc = WF_RETURN_CONTINUE;
			goto out;

		case WFFBP_STYLE_CENTER:
		case WFFBP_STYLE_FIT:
		case WFFBP_STYLE_STRETCH:
			if (WffbpSet(conf, hdc, TRUE, FALSE)) {
				rc = WF_RETURN_CONTINUE;
				goto out;
			} else {
				WFR_DEBUG_FAIL();
				rc = WF_RETURN_FAILURE;
				goto out;
			}
		}
	}

out:
	if (!hdc_in) {
		ReleaseDC(hwnd, hdc);
	}
	return rc;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
