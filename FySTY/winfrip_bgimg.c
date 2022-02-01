/*
 * winfrip_bgimg.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "FySTY/winfrip.h"
#include "FySTY/winfrip_priv.h"

#include <gdiplus/gdiplus.h>
#include <gdiplus/gdiplusflat.h>

#include <stdlib.h>
#include <time.h>

/*
 * Preprocessor macros
 */

#define WINFRIPP_BGIMG_FILTER_IMAGE_FILES (									\
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

typedef struct WinfrippBgimgContext_s {
    Conf *conf;
} WinfrippBgimgContext;

/*
 * {External,Static} variables
 */

static char *winfripp_bgimg_dname = NULL;
static size_t winfripp_bgimg_dname_len = 0;
static size_t winfripp_bgimg_dname_filec = 0;
static char **winfripp_bgimg_dname_filev = NULL;
static char *winfripp_bgimg_fname = NULL;

static HDC winfripp_bgimg_hdc = NULL;
static HGDIOBJ winfripp_bgimg_hdc_old = NULL;

static WinFrippBgImgState winfripp_bgimg_state = WINFRIPP_BGIMG_STATE_NONE;

static WinfrippBgimgContext *winfripp_bgimg_timer_ctx = NULL;

/*
 * External subroutine prototypes
 */

/* window.c */
void reset_window(int);

/*
 * Private subroutine prototypes
 */

static void winfripp_bgimg_config_panel_slideshow(union control *ctrl, dlgparam *dlg, void *data, int event);
static void winfripp_bgimg_config_panel_style(union control *ctrl, dlgparam *dlg, void *data, int event);
static void winfripp_bgimg_config_panel_type(union control *ctrl, dlgparam *dlg, void *data, int event);

static void winfripp_bgimg_reconf_slideshow(Conf *conf);

static BOOL winfripp_bgimg_set_get_fname(Conf *conf, BOOL reshuffle, wchar_t **pbg_bmp_fname_w, BOOL *pbg_bmpfl);
static BOOL winfripp_bgimg_set_load_bmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc);
static BOOL winfripp_bgimg_set_load_nonbmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc);
static BOOL winfripp_bgimg_set_process(HDC bg_hdc, int bg_height, int bg_width, HBITMAP bmp_src, Conf *conf, HDC hdc);
static BOOL winfripp_bgimg_set_process_blend(HDC bg_hdc, int bg_height, int bg_width, Conf *conf);
static BOOL winfripp_bgimg_set(Conf *conf, HDC hdc, BOOL force, BOOL reshuffle);

static void winfripp_bgimg_timer_fn(void *ctx, unsigned long now);

/*
 * Private subroutines
 */

static void winfripp_bgimg_config_panel_slideshow(union control *ctrl, dlgparam *dlg, void *data, int event)
{
    Conf *conf = (Conf *)data;

    switch (event) {
    case EVENT_REFRESH:
	dlg_update_start(ctrl, dlg);
	dlg_listbox_clear(ctrl, dlg);
	dlg_listbox_addwithid(ctrl, dlg, "Single image", WINFRIPP_BGIMG_SLIDESHOW_SINGLE_IMAGE);
	dlg_listbox_addwithid(ctrl, dlg, "Melbourne shuffle!", WINFRIPP_BGIMG_SLIDESHOW_SHUFFLE);

	switch (conf_get_int(conf, CONF_frip_bgimg_slideshow)) {
	case WINFRIPP_BGIMG_SLIDESHOW_SINGLE_IMAGE:
	    dlg_listbox_select(ctrl, dlg, 0); break;
	case WINFRIPP_BGIMG_SLIDESHOW_SHUFFLE:
	    dlg_listbox_select(ctrl, dlg, 1); break;
	default:
	    WINFRIPP_DEBUG_FAIL(); break;
	}
	dlg_update_done(ctrl, dlg);
	break;

    case EVENT_SELCHANGE:
    case EVENT_VALCHANGE:
	conf_set_int(conf, CONF_frip_bgimg_slideshow,
		     dlg_listbox_getid(ctrl, dlg,
				       dlg_listbox_index(ctrl, dlg)));
	break;
    }
}

static void winfripp_bgimg_config_panel_style(union control *ctrl, dlgparam *dlg, void *data, int event)
{
    Conf *conf = (Conf *)data;

    switch (event) {
    case EVENT_REFRESH:
	dlg_update_start(ctrl, dlg);
	dlg_listbox_clear(ctrl, dlg);
	dlg_listbox_addwithid(ctrl, dlg, "Absolute", WINFRIPP_BGIMG_STYLE_ABSOLUTE);
	dlg_listbox_addwithid(ctrl, dlg, "Center", WINFRIPP_BGIMG_STYLE_CENTER);
	dlg_listbox_addwithid(ctrl, dlg, "Fit", WINFRIPP_BGIMG_STYLE_FIT);
	dlg_listbox_addwithid(ctrl, dlg, "Stretch", WINFRIPP_BGIMG_STYLE_STRETCH);
	dlg_listbox_addwithid(ctrl, dlg, "Tile", WINFRIPP_BGIMG_STYLE_TILE);

	switch (conf_get_int(conf, CONF_frip_bgimg_style)) {
	case WINFRIPP_BGIMG_STYLE_ABSOLUTE:
	    dlg_listbox_select(ctrl, dlg, 0); break;
	case WINFRIPP_BGIMG_STYLE_CENTER:
	    dlg_listbox_select(ctrl, dlg, 1); break;
	case WINFRIPP_BGIMG_STYLE_FIT:
	    dlg_listbox_select(ctrl, dlg, 2); break;
	case WINFRIPP_BGIMG_STYLE_STRETCH:
	    dlg_listbox_select(ctrl, dlg, 3); break;
	case WINFRIPP_BGIMG_STYLE_TILE:
	    dlg_listbox_select(ctrl, dlg, 4); break;
	default:
	    WINFRIPP_DEBUG_FAIL(); break;
	}
	dlg_update_done(ctrl, dlg);
	break;

    case EVENT_SELCHANGE:
    case EVENT_VALCHANGE:
	conf_set_int(conf, CONF_frip_bgimg_style,
		     dlg_listbox_getid(ctrl, dlg,
				       dlg_listbox_index(ctrl, dlg)));
	break;
    }
}

static void winfripp_bgimg_config_panel_type(union control *ctrl, dlgparam *dlg, void *data, int event)
{
    Conf *conf = (Conf *)data;

    switch (event) {
    case EVENT_REFRESH:
	dlg_update_start(ctrl, dlg);
	dlg_listbox_clear(ctrl, dlg);
	dlg_listbox_addwithid(ctrl, dlg, "Solid", WINFRIPP_BGIMG_TYPE_SOLID);
	dlg_listbox_addwithid(ctrl, dlg, "Image", WINFRIPP_BGIMG_TYPE_IMAGE);

	switch (conf_get_int(conf, CONF_frip_bgimg_type)) {
	case WINFRIPP_BGIMG_TYPE_SOLID:
	    dlg_listbox_select(ctrl, dlg, 0); break;
	case WINFRIPP_BGIMG_TYPE_IMAGE:
	    dlg_listbox_select(ctrl, dlg, 1); break;
	default:
	    WINFRIPP_DEBUG_FAIL(); break;
	}
	dlg_update_done(ctrl, dlg);
	break;

    case EVENT_SELCHANGE:
    case EVENT_VALCHANGE:
	conf_set_int(conf, CONF_frip_bgimg_type,
		     dlg_listbox_getid(ctrl, dlg,
				       dlg_listbox_index(ctrl, dlg)));
	break;
    }
}

static BOOL winfripp_bgimg_set_get_fname(Conf *conf, BOOL reshuffle, wchar_t **pbg_fname_w, BOOL *pbg_bmpfl)
{
    char *bg_fname = NULL;
    Filename *bg_fname_conf;
    size_t bg_fname_idx, bg_fname_len;

    BOOL rc;


    WINFRIPP_DEBUG_ASSERT(pbg_fname_w);
    WINFRIPP_DEBUG_ASSERT(pbg_bmpfl);

    switch (conf_get_int(conf, CONF_frip_bgimg_slideshow)) {
    default:
	WINFRIPP_DEBUG_FAIL(); break;

    case WINFRIPP_BGIMG_SLIDESHOW_SINGLE_IMAGE:
	bg_fname_conf = conf_get_filename(conf, CONF_frip_bgimg_filename);
	bg_fname = bg_fname_conf->path;
	break;

    case WINFRIPP_BGIMG_SLIDESHOW_SHUFFLE:
	if (reshuffle || !winfripp_bgimg_fname) {
	    if ((winfripp_bgimg_dname_filec > 0) && (winfripp_bgimg_dname_filev != NULL)) {
		srand((unsigned)time(NULL));
		bg_fname_idx = (rand() % winfripp_bgimg_dname_filec);
		bg_fname_len = strlen(winfripp_bgimg_dname_filev[bg_fname_idx] ? winfripp_bgimg_dname_filev[bg_fname_idx] : "");

		if ((winfripp_bgimg_dname_len == 0) || (winfripp_bgimg_dname_filev[bg_fname_idx] == NULL) || (bg_fname_len == 0)) {
		    WINFRIPP_DEBUG_FAIL();
		    return FALSE;
		} else if (!(bg_fname = snewn(winfripp_bgimg_dname_len + 1 + bg_fname_len + 1, char))) {
		    WINFRIPP_DEBUG_FAIL();
		    return FALSE;
		} else {
		    snprintf(bg_fname, winfripp_bgimg_dname_len + 1 + bg_fname_len + 1, "%*.*s\\%s",
			     winfripp_bgimg_dname_len, winfripp_bgimg_dname_len, winfripp_bgimg_dname,
			     winfripp_bgimg_dname_filev[bg_fname_idx]);

		    if (winfripp_bgimg_fname) {
			sfree(winfripp_bgimg_fname);
		    }
		    winfripp_bgimg_fname = dupstr(bg_fname);
		}
	    } else {
		WINFRIPP_DEBUG_FAIL();
		return FALSE;
	    }
	} else {
	    bg_fname = winfripp_bgimg_fname;
	}
	break;
    }

    WINFRIPP_DEBUG_ASSERT(bg_fname);
    if (bg_fname) {
	bg_fname_len = strlen(bg_fname);
	if (bg_fname_len > (sizeof(".bmp")-1)) {
	    if ((bg_fname[bg_fname_len-4] == '.') &&
		(tolower(bg_fname[bg_fname_len-3]) == 'b') &&
		(tolower(bg_fname[bg_fname_len-2]) == 'm') &&
		(tolower(bg_fname[bg_fname_len-1]) == 'p'))
	    {
		*pbg_bmpfl = TRUE;
	    } else {
		*pbg_bmpfl = FALSE;
	    }
	    rc = winfripp_towcsdup(bg_fname, bg_fname_len + 1, pbg_fname_w);
	    WINFRIPP_DEBUG_ASSERT(rc == TRUE);

	    switch (conf_get_int(conf, CONF_frip_bgimg_slideshow)) {
	    case WINFRIPP_BGIMG_SLIDESHOW_SHUFFLE:
		if (bg_fname && (bg_fname != winfripp_bgimg_fname)) {
		    sfree(bg_fname);
		}
		break;
	    }

	    return rc;
	}
    }

    switch (conf_get_int(conf, CONF_frip_bgimg_slideshow)) {
    case WINFRIPP_BGIMG_SLIDESHOW_SHUFFLE:
	if (bg_fname) {
	    sfree(bg_fname);
	}
	break;
    }

    return FALSE;
}

static BOOL winfripp_bgimg_set_load_bmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc)
{
    HBITMAP bg_bmp = NULL;
    HDC bg_hdc = NULL;
    HGDIOBJ bg_hdc_old = NULL;
    int bg_height, bg_width;
    HBITMAP bmp_src = NULL;
    BOOL rc = FALSE;


    WINFRIPP_DEBUG_ASSERT(pbg_hdc);
    WINFRIPP_DEBUG_ASSERT(pbg_hdc_old);
    WINFRIPP_DEBUG_ASSERT(pbg_height);
    WINFRIPP_DEBUG_ASSERT(pbg_width);
    WINFRIPP_DEBUG_ASSERT(pbmp_src);
    WINFRIPP_DEBUG_ASSERT(bmp_src_fname_w);
    WINFRIPP_DEBUG_ASSERT(hdc);

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
    WINFRIPP_DEBUG_ASSERT(rc == TRUE);
    return rc;
}

static BOOL winfripp_bgimg_set_load_nonbmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc)
{
    HBITMAP bg_bmp = NULL;
    HDC bg_hdc = NULL;
    HGDIOBJ bg_hdc_old = NULL;
    int bg_height, bg_width;
    HBITMAP bmp_src = NULL;

    GpBitmap *gdip_bmp;
    GdiplusStartupInput gdip_si;
    GpStatus gdip_status;
    ULONG_PTR gdip_token;

    BOOL rc = FALSE;


    WINFRIPP_DEBUG_ASSERT(pbg_hdc);
    WINFRIPP_DEBUG_ASSERT(pbg_hdc_old);
    WINFRIPP_DEBUG_ASSERT(pbg_height);
    WINFRIPP_DEBUG_ASSERT(pbg_width);
    WINFRIPP_DEBUG_ASSERT(pbmp_src);
    WINFRIPP_DEBUG_ASSERT(bmp_src_fname_w);
    WINFRIPP_DEBUG_ASSERT(hdc);

    gdip_si.GdiplusVersion = 2;
    gdip_si.DebugEventCallback = NULL;
    gdip_si.SuppressBackgroundThread = FALSE;
    gdip_si.SuppressExternalCodecs = FALSE;
    if ((gdip_status = GdiplusStartup(&gdip_token, &gdip_si, NULL)) != Ok) {
	WINFRIPP_DEBUG_FAIL();
	return FALSE;
    } else if ((gdip_status = GdipCreateBitmapFromFile(bmp_src_fname_w, &gdip_bmp)) != Ok) {
	WINFRIPP_DEBUG_FAIL();
	GdiplusShutdown(gdip_token);
	return FALSE;
    } else {
	gdip_status = GdipCreateHBITMAPFromBitmap(gdip_bmp, &bmp_src, 0);
	GdipDisposeImage(gdip_bmp);
	GdiplusShutdown(gdip_token);
	if (gdip_status != Ok) {
	    WINFRIPP_DEBUG_FAIL();
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
    WINFRIPP_DEBUG_ASSERT(rc == TRUE);
    return rc;
}

static BOOL winfripp_bgimg_set_process(HDC bg_hdc, int bg_height, int bg_width, HBITMAP bmp_src, Conf *conf, HDC hdc)
{
    HBRUSH bg_brush;
    int bg_hdc_sb_mode;
    RECT bg_rect, cr;
    int bgimg_style;

    int bmp_offset_x, bmp_offset_y;
    HDC bmp_src_hdc;
    BITMAP bmp_src_obj;
    HGDIOBJ bmp_src_old;

    int cr_height, cr_width;

    HWND hwnd;

    int padding;
    float ratio, ratioH, ratioW;

    BOOL rc = FALSE;

    int x, y;


    WINFRIPP_DEBUG_ASSERT(bg_hdc);
    WINFRIPP_DEBUG_ASSERT(bg_height > 0);
    WINFRIPP_DEBUG_ASSERT(bg_width > 0);
    WINFRIPP_DEBUG_ASSERT(bmp_src);
    WINFRIPP_DEBUG_ASSERT(hdc);

    SetRect(&bg_rect, 0, 0, bg_width, bg_height);
    bgimg_style = conf_get_int(conf, CONF_frip_bgimg_style);
    switch (bgimg_style) {
    default:
	WINFRIPP_DEBUG_FAIL(); break;

    case WINFRIPP_BGIMG_STYLE_ABSOLUTE:
    case WINFRIPP_BGIMG_STYLE_CENTER:
    case WINFRIPP_BGIMG_STYLE_FIT:
    case WINFRIPP_BGIMG_STYLE_STRETCH:
	if ((bmp_src_hdc = CreateCompatibleDC(bg_hdc))) {
	    GetObject(bmp_src, sizeof(bmp_src_obj), &bmp_src_obj);
	    bmp_src_old = SelectObject(bmp_src_hdc, bmp_src);

	    switch (bgimg_style) {
	    default:
		WINFRIPP_DEBUG_FAIL(); break;

	    case WINFRIPP_BGIMG_STYLE_ABSOLUTE:
		rc = BitBlt(bg_hdc, bg_rect.left, bg_rect.top,
			    bmp_src_obj.bmWidth, bmp_src_obj.bmHeight,
			    bmp_src_hdc, 0, 0, SRCCOPY) > 0;
		break;

	    case WINFRIPP_BGIMG_STYLE_CENTER:
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
		rc = BitBlt(bg_hdc, bg_rect.left, bg_rect.top,
			    bg_rect.right - bg_rect.left,
			    bg_rect.bottom - bg_rect.top,
			    bmp_src_hdc, 0, 0, SRCCOPY) > 0;
		break;

	    case WINFRIPP_BGIMG_STYLE_FIT:
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
		rc = StretchBlt(bg_hdc, x, y,
				(int)((float)bmp_src_obj.bmWidth * ratio),
				(int)((float)bmp_src_obj.bmHeight * ratio),
				bmp_src_hdc,
				0, 0,
				bmp_src_obj.bmWidth, bmp_src_obj.bmHeight,
				SRCCOPY) > 0;
		SetStretchBltMode(bg_hdc, bg_hdc_sb_mode);
		break;

	    case WINFRIPP_BGIMG_STYLE_STRETCH:
		bg_hdc_sb_mode = SetStretchBltMode(bg_hdc, HALFTONE);
		rc = StretchBlt(bg_hdc, bg_rect.left, bg_rect.top,
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

    case WINFRIPP_BGIMG_STYLE_TILE:
	if ((bg_brush = CreatePatternBrush(bmp_src))) {
	    rc = FillRect(bg_hdc, &bg_rect, bg_brush) > 0;
	    DeleteObject(bg_brush);
	}
	break;
    }

    WINFRIPP_DEBUG_ASSERT(rc == TRUE);
    return rc;
}

static BOOL winfripp_bgimg_set_process_blend(HDC bg_hdc, int bg_height, int bg_width, Conf *conf)
{
    RECT bg_rect;
    HBITMAP blend_bmp = NULL;
    COLORREF blend_colour;
    BLENDFUNCTION blend_ftn;
    HDC blend_hdc = NULL;
    HGDIOBJ blend_old = NULL;
    BOOL rc = FALSE;


    WINFRIPP_DEBUG_ASSERT(bg_hdc);
    WINFRIPP_DEBUG_ASSERT(bg_height > 0);
    WINFRIPP_DEBUG_ASSERT(bg_width > 0);

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
	    rc = AlphaBlend(bg_hdc,
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
    WINFRIPP_DEBUG_ASSERT(rc == TRUE);
    return rc;
}

static BOOL winfripp_bgimg_set(Conf *conf, HDC hdc, BOOL force, BOOL reshuffle)
{
    wchar_t *bg_bmp_fname_w;
    BOOL bg_bmpfl;
    HDC bg_hdc = NULL;
    HGDIOBJ bg_hdc_cur, bg_hdc_old = NULL;
    int bg_height, bg_width;
    HBITMAP bmp_src = NULL;
    BOOL rc = FALSE;


    WINFRIPP_DEBUG_ASSERT(hdc);

    switch (winfripp_bgimg_state) {
    default:
	WINFRIPP_DEBUG_FAIL(); break;

    case WINFRIPP_BGIMG_STATE_NONE:
	break;
    case WINFRIPP_BGIMG_STATE_FAILED:
	if (!force) {
	    WINFRIPP_DEBUG_FAIL();
	    return FALSE;
	} else {
	    break;
	}
    case WINFRIPP_BGIMG_STATE_INIT:
	if (!force) {
	    return TRUE;
	} else {
	    break;
	}
    }

    if (winfripp_bgimg_hdc) {
	bg_hdc_cur = GetCurrentObject(winfripp_bgimg_hdc, OBJ_BITMAP);
	SelectObject(winfripp_bgimg_hdc, winfripp_bgimg_hdc_old); winfripp_bgimg_hdc_old = NULL;
	DeleteObject(bg_hdc_cur);
	DeleteDC(winfripp_bgimg_hdc); winfripp_bgimg_hdc = NULL;
    }

    if ((winfripp_bgimg_set_get_fname(conf, reshuffle, &bg_bmp_fname_w, &bg_bmpfl))) {
	switch (bg_bmpfl) {
	case TRUE:
	    rc = winfripp_bgimg_set_load_bmp(&bg_hdc, &bg_hdc_old, &bg_height, &bg_width, &bmp_src, bg_bmp_fname_w, hdc);
	    break;

	default:
	case FALSE:
	    rc = winfripp_bgimg_set_load_nonbmp(&bg_hdc, &bg_hdc_old, &bg_height, &bg_width, &bmp_src, bg_bmp_fname_w, hdc);
	    break;
	}
	sfree(bg_bmp_fname_w);
    }

    if (rc) {
	if (winfripp_bgimg_set_process(bg_hdc, bg_height, bg_width, bmp_src, conf, hdc)) {
	    if (winfripp_bgimg_set_process_blend(bg_hdc, bg_height, bg_width, conf)) {
		DeleteObject(bmp_src);
		winfripp_bgimg_hdc = bg_hdc; winfripp_bgimg_hdc_old = bg_hdc_old;
		winfripp_bgimg_state = WINFRIPP_BGIMG_STATE_INIT;
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
    winfripp_bgimg_state = WINFRIPP_BGIMG_STATE_FAILED;
    WINFRIPP_DEBUG_FAIL();
    return FALSE;
}

static void winfripp_bgimg_timer_fn(void *ctx, unsigned long now)
{
    WinfrippBgimgContext *context = (WinfrippBgimgContext *)ctx;
    HDC hDC;
    HWND hWnd = GetActiveWindow();

    (void)now;

    schedule_timer(conf_get_int(context->conf,
		   CONF_frip_bgimg_slideshow_freq) * 1000,
		   winfripp_bgimg_timer_fn, ctx);
    hDC = GetDC(hWnd);
    winfripp_bgimg_set(context->conf, hDC, TRUE, TRUE);
    ReleaseDC(hWnd, hDC);
    InvalidateRect(hWnd, NULL, TRUE);
    reset_window(2);
}

/*
 * Public subroutines private to FySTY/winfrip*.c
 */

void winfripp_bgimg_config_panel(struct controlbox *b)
{
    struct controlset *s_bgimg_settings, *s_bgimg_params, *s_slideshow;

    WINFRIPP_DEBUG_ASSERT(b);

    /*
     * The Frippery: background panel.
     */

    ctrl_settitle(b, "Frippery/Background", "Configure pointless frippery: background image");

    /*
     * The Frippery: Background image settings controls box.
     */

    s_bgimg_settings = ctrl_getset(b, "Frippery/Background", "frip_bgimg_settings", "Background image settings");
    ctrl_filesel(s_bgimg_settings, "Image file/directory:", 'i',
		 WINFRIPP_BGIMG_FILTER_IMAGE_FILES, FALSE, "Select background image file/directory",
		 P(WINFRIPP_HELP_CTX), conf_filesel_handler, I(CONF_frip_bgimg_filename));
    ctrl_text(s_bgimg_settings, "In order to select an image directory for slideshows, select "
				"an arbitrary file inside the directory in question.", P(WINFRIPP_HELP_CTX));
    ctrl_droplist(s_bgimg_settings, "Type:", 't', 45, P(WINFRIPP_HELP_CTX),
		  winfripp_bgimg_config_panel_type, P(NULL));
    ctrl_droplist(s_bgimg_settings, "Style:", 's', 45, P(WINFRIPP_HELP_CTX),
		  winfripp_bgimg_config_panel_style, P(NULL));

    /*
     * The Frippery: Background image parameters control box.
     */

    s_bgimg_params = ctrl_getset(b, "Frippery/Background", "frip_bgimg_params", "Background image parameters");
    ctrl_editbox(s_bgimg_params, "Opacity (0-100):", 'p', 15, P(WINFRIPP_HELP_CTX),
		 conf_editbox_handler, I(CONF_frip_bgimg_opacity), I(-1));
    ctrl_editbox(s_bgimg_params, "Fit padding (0-100):", 'n', 15, P(WINFRIPP_HELP_CTX),
		 conf_editbox_handler, I(CONF_frip_bgimg_padding), I(-1));

    /*
     * The Frippery: Slideshow settings control box.
     */

    s_slideshow = ctrl_getset(b, "Frippery/Background", "frip_bgimg_slideshow", "Slideshow settings");
    ctrl_droplist(s_slideshow, "Slideshow:", 'd', 45, P(WINFRIPP_HELP_CTX),
		  winfripp_bgimg_config_panel_slideshow, P(NULL));
    ctrl_editbox(s_slideshow, "Slideshow frequency (in seconds):", 'f', 20, P(WINFRIPP_HELP_CTX),
		 conf_editbox_handler, I(CONF_frip_bgimg_slideshow_freq), I(-1));
}

static void winfripp_bgimg_reconf_slideshow(Conf *conf)
{
    char *bg_dname;
    Filename *bg_dname_conf;
    size_t bg_dname_len;

    char *bg_fname;
    size_t bg_fname_len;

    size_t dname_filec_new = 0;
    char **dname_filev_new = NULL;

    char *dname_new = NULL;
    WIN32_FIND_DATA dname_new_ffd;
    HANDLE dname_new_hFind = INVALID_HANDLE_VALUE;

    char *p, **pp;

    WinfrippBgimgContext *timer_ctx_new = NULL;


    switch (conf_get_int(conf, CONF_frip_bgimg_slideshow)) {
    default:
	WINFRIPP_DEBUG_FAIL(); break;

    case WINFRIPP_BGIMG_SLIDESHOW_SINGLE_IMAGE:
	if (winfripp_bgimg_dname) {
	    sfree(winfripp_bgimg_dname);
	}
	if (winfripp_bgimg_dname_filev) {
	    for (size_t nfile = 0; nfile < winfripp_bgimg_dname_filec; nfile++) {
		if (winfripp_bgimg_dname_filev[nfile]) {
		    sfree(winfripp_bgimg_dname_filev[nfile]);
		}
	    }

	    sfree(winfripp_bgimg_dname_filev);

	    if (winfripp_bgimg_fname) {
		sfree(winfripp_bgimg_fname);
		winfripp_bgimg_fname = NULL;
	    }
	}

	winfripp_bgimg_dname = NULL;
	winfripp_bgimg_dname_len = 0;
	winfripp_bgimg_dname_filec = 0;
	winfripp_bgimg_dname_filev = NULL;

	if (winfripp_bgimg_timer_ctx) {
	    expire_timer_context(winfripp_bgimg_timer_ctx);
	    sfree(winfripp_bgimg_timer_ctx);
	    winfripp_bgimg_timer_ctx = NULL;
	}
	break;

    case WINFRIPP_BGIMG_SLIDESHOW_SHUFFLE:
	bg_dname_conf = conf_get_filename(conf, CONF_frip_bgimg_filename);
	bg_dname = bg_dname_conf->path;
	bg_dname_len = strlen(bg_dname);

	if (bg_dname_len == 0) {
	    goto fail;
	} else {
	    for (size_t nch = (bg_dname_len - 1); nch >= 0; nch--) {
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
				strncpy(p, dname_new_ffd.cFileName, bg_fname_len); p[bg_fname_len] = '\0';
				dname_filev_new[dname_filec_new - 1] = p;
				dname_filev_new[dname_filec_new] = NULL;
			    }
			}
		    }
		} while (FindNextFile(dname_new_hFind, &dname_new_ffd) != 0);

		if (winfripp_bgimg_dname) {
		    sfree(winfripp_bgimg_dname);
		}
		if (winfripp_bgimg_dname_filev) {
		    for (size_t nfile = 0; nfile < winfripp_bgimg_dname_filec; nfile++) {
			if (winfripp_bgimg_dname_filev[nfile]) {
			    sfree(winfripp_bgimg_dname_filev[nfile]);
			}
		    }

		    sfree(winfripp_bgimg_dname_filev);
		}

		winfripp_bgimg_dname = dname_new;
		winfripp_bgimg_dname_len = bg_dname_len;
		winfripp_bgimg_dname_filec = dname_filec_new;
		winfripp_bgimg_dname_filev = dname_filev_new;

		if (!(timer_ctx_new = snew(WinfrippBgimgContext))) {
		    goto fail;
		} else if (winfripp_bgimg_timer_ctx) {
		    expire_timer_context(winfripp_bgimg_timer_ctx);
		    sfree(winfripp_bgimg_timer_ctx);
		}
		winfripp_bgimg_timer_ctx = timer_ctx_new;
		winfripp_bgimg_timer_ctx->conf = conf;

		schedule_timer(conf_get_int(conf, CONF_frip_bgimg_slideshow_freq) * 1000,
			       winfripp_bgimg_timer_fn, (void *)winfripp_bgimg_timer_ctx);
	    } else {
		WINFRIPP_DEBUG_FAIL();
	    }
	}
	break;
    }

    return;

fail:
    if (dname_filev_new) {
	for (size_t nfile = 0; nfile < dname_filec_new; nfile++) {
	    if (dname_filev_new[nfile]) {
		sfree(dname_filev_new[nfile]);
	    }
	}

	sfree(dname_filev_new);
    }
    if (dname_new) {
	sfree(dname_new);
    }
    if (timer_ctx_new) {
	sfree(timer_ctx_new);
    }

    WINFRIPP_DEBUG_FAIL();
}

/*
 * Public subroutines
 */

WinFripReturn winfrip_bgimg_op(WinFripBgImgOp op, BOOL *pbgfl, Conf *conf, HDC hdc_in, HWND hwnd, int char_width, int font_height, int len, int nbg, int rc_width, int x, int y)
{
    HDC hdc;
    WinFripReturn rc;


    if (conf_get_int(conf, CONF_frip_bgimg_type) != WINFRIPP_BGIMG_TYPE_IMAGE) {
	return WINFRIP_RETURN_NOOP;
    } else if (hdc_in) {
	hdc = hdc_in;
    } else if (!hdc_in) {
	hdc = GetDC(hwnd);
	WINFRIPP_DEBUG_ASSERT(hdc);
    }

    switch (op) {
    default:
	WINFRIPP_DEBUG_FAIL();
	rc = WINFRIP_RETURN_FAILURE;
	goto out;

    case WINFRIP_BGIMG_OP_DRAW:
	WINFRIPP_DEBUG_ASSERT(hdc);
	WINFRIPP_DEBUG_ASSERT(font_height > 0);
	WINFRIPP_DEBUG_ASSERT(len > 0);
	if ((nbg == 258) || (nbg == 259)) {
	    if (winfripp_bgimg_hdc || winfripp_bgimg_set(conf, hdc, FALSE, TRUE)) {
		if (rc_width > 0) {
		    rc = BitBlt(hdc, x, y, rc_width, font_height,
				winfripp_bgimg_hdc, x, y, SRCCOPY) > 0;
		} else {
		    rc = BitBlt(hdc, x, y, char_width * len, font_height,
				winfripp_bgimg_hdc, x, y, SRCCOPY) > 0;
		}
		if (rc) {
		    *pbgfl = TRUE;
		    rc = WINFRIP_RETURN_CONTINUE;
		    goto out;
		} else {
		    *pbgfl = FALSE;
		    WINFRIPP_DEBUG_FAIL();
		    rc = WINFRIP_RETURN_FAILURE;
		    goto out;
		}
	    } else {
		WINFRIPP_DEBUG_FAIL();
		rc = WINFRIP_RETURN_FAILURE;
		goto out;
	    }
	} else {
	    rc = WINFRIP_RETURN_CONTINUE;
	    goto out;
	}

    case WINFRIP_BGIMG_OP_INIT:
	winfripp_bgimg_reconf_slideshow(conf);
	break;

    case WINFRIP_BGIMG_OP_RECONF:
	winfripp_bgimg_reconf_slideshow(conf);
	if (winfripp_bgimg_set(conf, hdc, TRUE, TRUE)) {
	    rc = WINFRIP_RETURN_CONTINUE;
	    goto out;
	} else {
	    WINFRIPP_DEBUG_FAIL();
	    rc = WINFRIP_RETURN_FAILURE;
	    goto out;
	}

    case WINFRIP_BGIMG_OP_SIZE:
	switch (conf_get_int(conf, CONF_frip_bgimg_style)) {
	default:
	    WINFRIPP_DEBUG_FAIL();
	    rc = WINFRIP_RETURN_FAILURE;
	    goto out;

	case WINFRIPP_BGIMG_STYLE_ABSOLUTE:
	case WINFRIPP_BGIMG_STYLE_TILE:
	    rc = WINFRIP_RETURN_CONTINUE;
	    goto out;
	case WINFRIPP_BGIMG_STYLE_CENTER:
	case WINFRIPP_BGIMG_STYLE_FIT:
	case WINFRIPP_BGIMG_STYLE_STRETCH:
	    if (winfripp_bgimg_set(conf, hdc, TRUE, FALSE)) {
		rc = WINFRIP_RETURN_CONTINUE;
		goto out;
	    } else {
		WINFRIPP_DEBUG_FAIL();
		rc = WINFRIP_RETURN_FAILURE;
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
