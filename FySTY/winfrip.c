/*
 * winfrip.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "terminal.h"
#include "winfrip.h"

#include <assert.h>
#include <shlwapi.h>
#include <wtypes.h>
#include <gdiplus/gdiplus.h>
#include <gdiplus/gdiplusflat.h>

/*
 * Private enumerations
 */

/*
 * XXX document
 */
typedef enum WinFripBgImgState {
    WINFRIP_BGIMG_STATE_NONE		= 0,
    WINFRIP_BGIMG_STATE_FAILED		= 1,
    WINFRIP_BGIMG_STATE_INIT		= 2,
} WinFripBgImgState;

/*
 * XXX document
 */
typedef enum WinFripBgImgStyle {
    WINFRIP_BGIMG_STYLE_ABSOLUTE	= 0,
    WINFRIP_BGIMG_STYLE_CENTER		= 1,
    WINFRIP_BGIMG_STYLE_STRETCH		= 2,
    WINFRIP_BGIMG_STYLE_TILE		= 3,
} WinFripBgImgStyle;

/*
 * XXX document
 */
typedef enum WinFripBgImgType {
    WINFRIP_BGIMG_TYPE_SOLID		= 0,
    WINFRIP_BGIMG_TYPE_IMAGE		= 1,
} WinFripBgImgType;

/*
 * XXX document
 */
typedef enum WinFripHoverState {
    WINFRIP_HOVER_STATE_NONE		= 0,
    WINFRIP_HOVER_STATE_SELECT		= 1,
    WINFRIP_HOVER_STATE_CLICK		= 2,
    WINFRIP_HOVER_STATE_CLEAR		= 3,
} WinFripHoverState;

/*
 * XXX document
 */
typedef enum WinFripTranspLevel {
    WINFRIP_TRANSP_LEVEL_OFF		= 255,
    WINFRIP_TRANSP_LEVEL_LOW		= 255 - 16,
    WINFRIP_TRANSP_LEVEL_MEDIUM		= 255 - 32,
    WINFRIP_TRANSP_LEVEL_HIGH		= 255 - 48,
} WinFripTranspLevel;

/*
 * XXX document
 */
typedef enum WinFripTranspOpaqueOn {
    WINFRIP_TRANSP_OPAQUE_NEVER		= 0,
    WINFRIP_TRANSP_OPAQUE_FOCUS_KILL	= 1,
    WINFRIP_TRANSP_OPAQUE_FOCUS_SET	= 2,
} WinFripTranspOpaqueOn;

/*
 * XXX document
 */
typedef enum WinFripTranspSetting {
    WINFRIP_TRANSP_SETTING_OFF		= 0,
    WINFRIP_TRANSP_SETTING_LOW		= 1,
    WINFRIP_TRANSP_SETTING_MEDIUM	= 2,
    WINFRIP_TRANSP_SETTING_HIGH		= 3,
    WINFRIP_TRANSP_SETTING_CUSTOM	= 4,
} WinFripTranspSetting;

/*
 * Preprocessor macros
 */

/*
 * XXX document
 */
#define winfrip_posle(p1,px,py) ( (p1).y < (py) || ( (p1).y == (py) && (p1).x <= (px) ) )
#define winfrip_poslt(px,py,p2) ( (py) < (p2).y || ( (py) == (p2).y && (px) < (p2).x ) )

/*
 * XXX document
 */
#define WINFRIP_BGIMG_FILTER_IMAGE_FILES									\
	("Bitmap Files (*.bmp)\0*.bmp\0"									\
	"EMF (*.emf)\0*.emf\0"											\
	"GIF (*.gif)\0*.gif\0"											\
	"ICO (*.ico)\0*.ico\0"											\
	"JPEG (*.jpg;*.jpeg;*.jpe;*.jfif)\0*.jpg;*.jpeg;*.jpe;*.jfif\0"						\
	"PNG (*.png)\0*.png\0"											\
	"TIFF (*.tif;*.tiff)\0*.tif;*.tiff\0"									\
	"WMF (*.wmf)\0*.wmf\0"											\
	"All Picture Files\0*.bmp;*.emf;*.gif;*.ico;*.jpg;*.jpeg;*.jpe;*.jfif;*.png;*.tif;*.tiff;*.wmf\0"	\
	"All Files (*.*)\0*\0\0\0")

/*
 * {External,Static} variables
 */

/*
 * XXX document
 */
extern Conf *conf;

/*
 * XXX document
 */
static HDC winfrip_bgimg_hdc = NULL;
static HGDIOBJ winfrip_bgimg_hdc_old = NULL;

/*
 * XXX document
 */
static WinFripBgImgState winfrip_bgimg_state = WINFRIP_BGIMG_STATE_NONE;

/*
 * XXX document
 */
static pos winfrip_hover_end = {0, 0};
static pos winfrip_hover_start = {0, 0};
static WinFripHoverState winfrip_hover_state = WINFRIP_HOVER_STATE_NONE;

/*
 * XXX document
 */
static wchar_t *winfrip_hover_url_w = NULL;
static size_t winfrip_hover_url_w_size = 0;

/*
 * XXX document
 */
static char *winfrip_hover_match_spec_conf = NULL;
static wchar_t *winfrip_hover_match_spec_w = NULL;

/*
 * Private subroutine prototypes
 */

static BOOL winfrip_init_bgimg_get_fname(wchar_t **pbg_bmp_fname_w, BOOL *pbg_bmpfl);
static BOOL winfrip_init_bgimg_load_bmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc);
static BOOL winfrip_init_bgimg_load_nonbmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc);
static BOOL winfrip_init_bgimg_process(HDC bg_hdc, int bg_height, int bg_width, HBITMAP bmp_src, HDC hdc);
static BOOL winfrip_init_bgimg_process_blend(HDC bg_hdc, int bg_height, int bg_width);
static BOOL winfrip_init_bgimg(HDC hdc, BOOL force);
static BOOL winfrip_towcsdup(char *in, size_t in_size, wchar_t **pout_w);

/*
 * Private subroutines
 */

static BOOL winfrip_init_bgimg_get_fname(wchar_t **pbg_fname_w, BOOL *pbg_bmpfl)
{
    char *bg_fname;
    Filename *bg_fname_conf;
    size_t bg_fname_len;


    assert(pbg_fname_w != NULL);
    assert(pbg_bmpfl != NULL);

    /*
     * XXX document
     */
    bg_fname_conf = conf_get_filename(conf, CONF_frip_bgimg_filename);
    bg_fname = bg_fname_conf->path;
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
	    return winfrip_towcsdup(bg_fname, bg_fname_len + 1, pbg_fname_w);
	}
    }
    return FALSE;
}

static BOOL winfrip_init_bgimg_load_bmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc)
{
    HBITMAP bg_bmp = NULL;
    HDC bg_hdc = NULL;
    HGDIOBJ bg_hdc_old = NULL;
    int bg_height, bg_width;
    HBITMAP bmp_src = NULL;
    BOOL rc = FALSE;


    assert(pbg_hdc != NULL);
    assert(pbg_hdc_old != NULL);
    assert(pbg_height != NULL);
    assert(pbg_width != NULL);
    assert(pbmp_src != NULL);
    assert(bmp_src_fname_w != NULL);
    assert(hdc != NULL);

    /*
     * XXX document
     */
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

    /*
     * XXX document
     */
    *pbg_hdc = bg_hdc; *pbg_hdc_old = bg_hdc_old;
    *pbg_height = bg_height; *pbg_width = bg_width;
    *pbmp_src = bmp_src;
    return rc;
}

static BOOL winfrip_init_bgimg_load_nonbmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc)
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


    assert(pbg_hdc != NULL);
    assert(pbg_hdc_old != NULL);
    assert(pbg_height != NULL);
    assert(pbg_width != NULL);
    assert(pbmp_src != NULL);
    assert(bmp_src_fname_w != NULL);
    assert(hdc != NULL);

    /*
     * XXX document
     */
    gdip_si.GdiplusVersion = 2;
    gdip_si.DebugEventCallback = NULL;
    gdip_si.SuppressBackgroundThread = FALSE;
    gdip_si.SuppressExternalCodecs = FALSE;
    if ((gdip_status = GdiplusStartup(&gdip_token, &gdip_si, NULL)) != Ok) {
	return FALSE;
    } else if ((gdip_status = GdipCreateBitmapFromFile(bmp_src_fname_w, &gdip_bmp)) != Ok) {
	GdiplusShutdown(gdip_token);
	return FALSE;
    } else {
	gdip_status = GdipCreateHBITMAPFromBitmap(gdip_bmp, &bmp_src, 0);
	GdipDisposeImage(gdip_bmp);
	GdiplusShutdown(gdip_token);
	if (gdip_status != Ok) {
	    return FALSE;
	}
    }

    /*
     * XXX document
     */
    bg_height = GetDeviceCaps(hdc, VERTRES);
    bg_width = GetDeviceCaps(hdc, HORZRES);
    if ((bg_hdc = CreateCompatibleDC(hdc))) {
	if ((bg_bmp = CreateCompatibleBitmap(hdc, bg_width, bg_height))) {
	    bg_hdc_old = SelectObject(bg_hdc, bg_bmp);
	    rc = TRUE;
	}
    }

    /*
     * XXX document
     */
    *pbg_hdc = bg_hdc; *pbg_hdc_old = bg_hdc_old;
    *pbg_height = bg_height; *pbg_width = bg_width;
    *pbmp_src = bmp_src;
    return rc;
}

static BOOL winfrip_init_bgimg_process(HDC bg_hdc, int bg_height, int bg_width, HBITMAP bmp_src, HDC hdc)
{
    int bgimg_style;
    HBRUSH bg_brush;
    int bg_hdc_sb_mode;
    RECT bg_rect, cr;
    int bmp_offset_x, bmp_offset_y;
    HDC bmp_src_hdc;
    BITMAP bmp_src_obj;
    HGDIOBJ bmp_src_old;
    HWND hwnd;
    BOOL rc = FALSE;


    assert(bg_hdc != NULL);
    assert(bg_height > 0);
    assert(bg_width > 0);
    assert(bmp_src != NULL);

    /*
     * XXX document
     */
    SetRect(&bg_rect, 0, 0, bg_width, bg_height);
    bgimg_style = conf_get_int(conf, CONF_frip_bgimg_style);
    switch (bgimg_style) {
    default:
	break;

    /*
     * XXX document
     */
    case WINFRIP_BGIMG_STYLE_ABSOLUTE:
    case WINFRIP_BGIMG_STYLE_CENTER:
    case WINFRIP_BGIMG_STYLE_STRETCH:
	if ((bmp_src_hdc = CreateCompatibleDC(bg_hdc))) {
	    GetObject(bmp_src, sizeof(bmp_src_obj), &bmp_src_obj);
	    bmp_src_old = SelectObject(bmp_src_hdc, bmp_src);
	    switch (bgimg_style) {
	    /*
	     * XXX document
	     */
	    case WINFRIP_BGIMG_STYLE_ABSOLUTE:
		rc = BitBlt(bg_hdc, bg_rect.left, bg_rect.top,
			    bmp_src_obj.bmWidth, bmp_src_obj.bmHeight,
			    bmp_src_hdc, 0, 0, SRCCOPY) > 0;
		break;

	    /*
	     * XXX document
	     */
	    case WINFRIP_BGIMG_STYLE_CENTER:
		hwnd = WindowFromDC(hdc);
		GetClientRect(hwnd, &cr);
		if ((cr.right - cr.left) > bmp_src_obj.bmWidth) {
		    bmp_offset_x = ((cr.right - cr.left) - bmp_src_obj.bmWidth) / 2;
		    bg_rect.left += bmp_offset_x; bg_rect.right -= bmp_offset_x;
		}
		if ((cr.bottom - cr.top) > bmp_src_obj.bmHeight) {
		    bmp_offset_y = ((cr.bottom - cr.top) - bmp_src_obj.bmHeight) / 2;
		    bg_rect.top += bmp_offset_y; bg_rect.bottom -= bmp_offset_y;
		}
		rc = BitBlt(bg_hdc, bg_rect.left, bg_rect.top,
			    bg_rect.right - bg_rect.left,
			    bg_rect.bottom - bg_rect.top,
			    bmp_src_hdc, 0, 0, SRCCOPY) > 0;
		break;

	    /*
	     * XXX document
	     */
	    case WINFRIP_BGIMG_STYLE_STRETCH:
		bg_hdc_sb_mode = SetStretchBltMode(bg_hdc, HALFTONE);
		rc = StretchBlt(bg_hdc, bg_rect.left, bg_rect.top,
				bg_rect.right - bg_rect.left,
				bg_rect.bottom - bg_rect.top,
				bmp_src_hdc, 0, 0,
				bmp_src_obj.bmWidth, bmp_src_obj.bmHeight, SRCCOPY) > 0;
		SetStretchBltMode(bg_hdc, bg_hdc_sb_mode);
		break;
	    }

	    /*
	     * XXX document
	     */
	    SelectObject(bmp_src_hdc, bmp_src_old);
	    DeleteDC(bmp_src_hdc);
	}
	break;

    /*
     * XXX document
     */
    case WINFRIP_BGIMG_STYLE_TILE:
	if ((bg_brush = CreatePatternBrush(bmp_src))) {
	    rc = FillRect(bg_hdc, &bg_rect, bg_brush) > 0;
	    DeleteObject(bg_brush);
	}
	break;
    }

    return rc;
}

static BOOL winfrip_init_bgimg_process_blend(HDC bg_hdc, int bg_height, int bg_width)
{
    RECT bg_rect;
    HBITMAP blend_bmp = NULL;
    COLORREF blend_colour;
    BLENDFUNCTION blend_ftn;
    HDC blend_hdc = NULL;
    HGDIOBJ blend_old = NULL;
    BOOL rc = FALSE;


    assert(bg_hdc != NULL);
    assert(bg_height > 0);
    assert(bg_width > 0);

    if ((blend_hdc = CreateCompatibleDC(bg_hdc))) {
	if ((blend_bmp = CreateCompatibleBitmap(bg_hdc, 1, 1))) {
	    /*
	     * XXX document
	     */
	    blend_old = SelectObject(blend_hdc, blend_bmp);
	    blend_colour = RGB(0, 0, 0);
	    SetPixelV(blend_hdc, 0, 0, blend_colour);

	    /*
	     * XXX document
	     */
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

    /*
     * XXX document
     */
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

static BOOL winfrip_init_bgimg(HDC hdc, BOOL force)
{
    wchar_t *bg_bmp_fname_w;
    BOOL bg_bmpfl;
    HDC bg_hdc = NULL;
    HGDIOBJ bg_hdc_cur, bg_hdc_old = NULL;
    int bg_height, bg_width;
    HBITMAP bmp_src = NULL;
    BOOL rc = FALSE;


    assert(hdc != NULL);

    /*
     * XXX document
     */
    switch (winfrip_bgimg_state) {
    default:
    case WINFRIP_BGIMG_STATE_NONE:
	break;
    case WINFRIP_BGIMG_STATE_FAILED:
	if (!force) {
	    return FALSE;
	} else {
	    break;
	}
    case WINFRIP_BGIMG_STATE_INIT:
	if (!force) {
	    return TRUE;
	} else {
	    break;
	}
    }

    /*
     * XXX document
     */
    if (winfrip_bgimg_hdc) {
	bg_hdc_cur = GetCurrentObject(winfrip_bgimg_hdc, OBJ_BITMAP);
	SelectObject(winfrip_bgimg_hdc, winfrip_bgimg_hdc_old); winfrip_bgimg_hdc_old = NULL;
	DeleteObject(bg_hdc_cur);
	DeleteDC(winfrip_bgimg_hdc); winfrip_bgimg_hdc = NULL;
    }

    /*
     * XXX document
     */
    if ((winfrip_init_bgimg_get_fname(&bg_bmp_fname_w, &bg_bmpfl))) {
	switch (bg_bmpfl) {
	case TRUE:
	    rc = winfrip_init_bgimg_load_bmp(&bg_hdc, &bg_hdc_old, &bg_height, &bg_width, &bmp_src, bg_bmp_fname_w, hdc);
	    break;
	case FALSE:
	    rc = winfrip_init_bgimg_load_nonbmp(&bg_hdc, &bg_hdc_old, &bg_height, &bg_width, &bmp_src, bg_bmp_fname_w, hdc);
	    break;
	}
    }

    /*
     * XXX document
     */
    if (rc) {
	if (winfrip_init_bgimg_process(bg_hdc, bg_height, bg_width, bmp_src, hdc)) {
	    if (winfrip_init_bgimg_process_blend(bg_hdc, bg_height, bg_width)) {
		DeleteObject(bmp_src);
		winfrip_bgimg_hdc = bg_hdc; winfrip_bgimg_hdc_old = bg_hdc_old;
		winfrip_bgimg_state = WINFRIP_BGIMG_STATE_INIT;
		return TRUE;
	    }
	}
    }

    /*
     * XXX document
     */
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
    winfrip_bgimg_state = WINFRIP_BGIMG_STATE_FAILED;
    return FALSE;
}

static BOOL winfrip_towcsdup(char *in, size_t in_size, wchar_t **pout_w)
{
    size_t out_w_len, out_w_size;
    wchar_t *out_w;


    /*
     * XXX document
     */
    out_w_len = MultiByteToWideChar(CP_ACP, 0, in, in_size, NULL, 0);
    if (out_w_len > 0) {
	out_w_size = out_w_len * sizeof(*out_w);
	out_w = malloc(out_w_size);
	ZeroMemory(out_w, out_w_size);
	if (MultiByteToWideChar(CP_ACP, 0, in, in_size, out_w, out_w_size) == out_w_len) {
	    *pout_w = out_w;
	    return TRUE;
	}
    }
    return FALSE;
}

/*
 * Public subroutines
 */

BOOL winfrip_bgimg_op(WinFripBgImgOp op, HDC hdc_in, HWND hwnd, int char_width, int font_height, int len, int nbg, int rc_width, int x, int y)
{
    HDC hdc;
    BOOL rc = FALSE;


    /*
     * XXX document
     */
    if (conf_get_int(conf, CONF_frip_bgimg_type) != WINFRIP_BGIMG_TYPE_IMAGE) {
	return FALSE;
    } else if (hdc_in) {
	hdc = hdc_in;
    } else if (!hdc_in) {
	hdc = GetDC(hwnd);
    }

    switch (op) {
    default:
	break;

    /*
     * XXX document
     */
    case WINFRIP_BGIMG_OP_DRAW:
	if ((nbg == 258) || (nbg == 259)) {
	    if (winfrip_bgimg_hdc || winfrip_init_bgimg(hdc, FALSE)) {
		if (rc_width > 0) {
		    rc = BitBlt(hdc, x, y, rc_width, font_height,
				winfrip_bgimg_hdc, x, y, SRCCOPY) > 0;
		} else {
		    rc = BitBlt(hdc, x, y, char_width * len, font_height,
				winfrip_bgimg_hdc, x, y, SRCCOPY) > 0;
		}
	    }
	}
	break;

    /*
     * XXX document
     */
    case WINFRIP_BGIMG_OP_RECONF:
	rc = winfrip_init_bgimg(hdc, TRUE);
	break;

    /*
     * XXX document
     */
    case WINFRIP_BGIMG_OP_SIZE:
	switch (conf_get_int(conf, CONF_frip_bgimg_style)) {
	default:
	    rc = TRUE; break;
	case WINFRIP_BGIMG_STYLE_CENTER:
	case WINFRIP_BGIMG_STYLE_STRETCH:
	    rc = winfrip_init_bgimg(hdc, TRUE);
	    break;
	}
	break;
    }

    /*
     * XXX document
     */
    if (!hdc_in) {
	ReleaseDC(hwnd, hdc);
    }
    return rc;
}

void winfrip_config_panel(struct controlbox *b)
{
    struct controlset *s;


    /*
     * The Window/Frippery panel (#1).
     */

    ctrl_settitle(b, "Window/Frippery", "Configure pointless frippery");

    /*
     * Background image
     */
    s = ctrl_getset(b, "Window/Frippery", "frip_bgimg", "Background image settings");
    ctrl_filesel(s, "Image file:", NO_SHORTCUT,
		 WINFRIP_BGIMG_FILTER_IMAGE_FILES, FALSE, "Select background image file",
		 HELPCTX(appearance_frippery), conf_filesel_handler, I(CONF_frip_bgimg_filename));
    ctrl_editbox(s, "Opacity (0-100):", NO_SHORTCUT, 20, HELPCTX(appearance_frippery),
		 conf_editbox_handler, I(CONF_frip_bgimg_opacity), I(-1));
    ctrl_radiobuttons(s, "Style:", NO_SHORTCUT, 4, HELPCTX(appearance_frippery),
		      conf_radiobutton_handler, I(CONF_frip_bgimg_style),
		      "Absolute",	NO_SHORTCUT,	I(WINFRIP_BGIMG_STYLE_ABSOLUTE),
		      "Center",		NO_SHORTCUT,	I(WINFRIP_BGIMG_STYLE_CENTER),
		      "Stretch",	NO_SHORTCUT,	I(WINFRIP_BGIMG_STYLE_STRETCH),
		      "Tile",		NO_SHORTCUT,	I(WINFRIP_BGIMG_STYLE_TILE), NULL);
    ctrl_radiobuttons(s, "Type:", NO_SHORTCUT, 2, HELPCTX(appearance_frippery),
		      conf_radiobutton_handler, I(CONF_frip_bgimg_type),
		      "Solid",		NO_SHORTCUT,	I(WINFRIP_BGIMG_TYPE_SOLID),
		      "Image",		NO_SHORTCUT,	I(WINFRIP_BGIMG_TYPE_IMAGE), NULL);

    /*
     * Transparency
     */
    s = ctrl_getset(b, "Window/Frippery", "frip_transp", "Transparency settings");
    ctrl_radiobuttons(s, "Setting", NO_SHORTCUT, 2, HELPCTX(appearance_frippery),
		      conf_radiobutton_handler, I(CONF_frip_transp_setting),
		      "Off",		NO_SHORTCUT,	I(WINFRIP_TRANSP_SETTING_OFF),
		      "Low",		NO_SHORTCUT,	I(WINFRIP_TRANSP_SETTING_LOW),
		      "Medium",		NO_SHORTCUT,	I(WINFRIP_TRANSP_SETTING_MEDIUM),
		      "High",		NO_SHORTCUT,	I(WINFRIP_TRANSP_SETTING_HIGH),
		      "Custom",		NO_SHORTCUT,	I(WINFRIP_TRANSP_SETTING_CUSTOM), NULL);
    ctrl_editbox(s, "Custom (0-255):", NO_SHORTCUT, 20, HELPCTX(appearance_frippery),
		 conf_editbox_handler, I(CONF_frip_transp_custom), I(-1));
    ctrl_radiobuttons(s, "Opaque on", NO_SHORTCUT, 2, HELPCTX(appearance_frippery),
		      conf_radiobutton_handler, I(CONF_frip_transp_opaque_on),
		      "Never",		NO_SHORTCUT,	I(WINFRIP_TRANSP_OPAQUE_NEVER),
		      "Focus loss",	NO_SHORTCUT,	I(WINFRIP_TRANSP_OPAQUE_FOCUS_KILL),
		      "Focus",		NO_SHORTCUT,	I(WINFRIP_TRANSP_OPAQUE_FOCUS_SET), NULL);

    /*
     * The Window/Frippery panel (#2).
     */

    ctrl_settitle(b, "Window/Frippery, #2", "Configure pointless frippery (#2)");

    /*
     * Clickable URLs
     */
    s = ctrl_getset(b, "Window/Frippery, #2", "frip_hover", "Clickable URLs settings");
    ctrl_editbox(s, "Match string", NO_SHORTCUT, 20, HELPCTX(appearance_frippery),
		 conf_editbox_handler, I(CONF_frip_hover_match_spec), I(1));
}

void winfrip_debug_init(void)
{
    /*
     * XXX document
     */
#ifdef WINFRIP_DEBUG
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
#endif
}

BOOL winfrip_hover_op(WinFripHoverOp op, HWND hwnd, UINT message, unsigned long *tattr, Terminal *term, WPARAM wParam, int x, int y)
{
    size_t hover_len, idx, new_buf_w_size;
    char *match_spec_conf;
    wchar_t *new_buf_w;
    wchar_t wch;
    int x_end, x_start;


    /*
     * XXX document
     */
    if (op == WINFRIP_HOVER_OP_CTRL_EVENT) {
	if (wParam & MK_CONTROL) {
	    op = WINFRIP_HOVER_OP_CTRL_DOWN;
	} else if (!(wParam & MK_CONTROL)) {
	    op = WINFRIP_HOVER_OP_CTRL_UP;
	}
    } else if (op == WINFRIP_HOVER_OP_MOUSE_EVENT) {
	if ((message == WM_LBUTTONDOWN) && (wParam & MK_CONTROL)) {
	    op = WINFRIP_HOVER_OP_MOUSE_DOWN;
	} else if ((message == WM_LBUTTONUP) && (wParam & MK_CONTROL)) {
	    op = WINFRIP_HOVER_OP_MOUSE_UP;
	} else {
	    return FALSE;
	}
    }

    switch (op) {
    default:
	break;

    /*
     * XXX document
     */
    case WINFRIP_HOVER_OP_CLEAR:
	if (winfrip_hover_state == WINFRIP_HOVER_STATE_CLEAR) {
	    winfrip_hover_state = WINFRIP_HOVER_STATE_NONE;
	    winfrip_hover_start.x = winfrip_hover_start.y = 0;
	    winfrip_hover_end.x = winfrip_hover_end.y = 0;
	}
	break;

    /*
     * XXX document
     */
    case WINFRIP_HOVER_OP_CTRL_UP:
	if ((winfrip_hover_state == WINFRIP_HOVER_STATE_SELECT) ||
	    (winfrip_hover_state == WINFRIP_HOVER_STATE_CLICK))
	{
	    winfrip_hover_state = WINFRIP_HOVER_STATE_CLEAR;
	    term_update(term);
	}
	break;

    /*
     * XXX document
     */
    case WINFRIP_HOVER_OP_DRAW:
	if (winfrip_posle(winfrip_hover_start, x, y) &&
	    winfrip_poslt(x, y, winfrip_hover_end))
	{
	    switch (winfrip_hover_state) {
	    case WINFRIP_HOVER_STATE_NONE:
		break;
	    case WINFRIP_HOVER_STATE_CLEAR:
		*tattr &= ~(ATTR_REVERSE | ATTR_UNDER); break;
	    case WINFRIP_HOVER_STATE_SELECT:
		*tattr |= ATTR_UNDER; break;
	    case WINFRIP_HOVER_STATE_CLICK:
		*tattr |= ATTR_REVERSE; break;
	    }
	}
	break;

    /*
     * XXX document
     */
    case WINFRIP_HOVER_OP_CTRL_DOWN:
	if ((x < term->cols) && (y < term->rows)) {
	    /*
	     * XXX document
	     */
	    for (x_start = x; x_start >= 0; x_start--) {
		wch = term->disptext[y]->chars[x_start].chr;
		if (DIRECT_FONT(wch)) {
		    wch &= 0xFF;
		}
		if (wch == 0x20) {
		    x_start++; break;
		} else if (x_start == 0) {
		    break;
		}
	    }
	    if ((x_start < 0) || (x_start > x)) {
		break;
	    }

	    /*
	     * XXX document
	     */
	    for (x_end = x; x_end < term->cols; x_end++) {
		wch = term->disptext[y]->chars[x_end].chr;
		if (DIRECT_FONT(wch)) {
		    wch &= 0xFF;
		}
		if (wch == 0x20) {
		    break;
		}
	    }
	    if (x_end == x) {
		break;
	    } else {
		winfrip_hover_state = WINFRIP_HOVER_STATE_SELECT;
		winfrip_hover_end.x = x_end;
		winfrip_hover_end.y = winfrip_hover_start.y = y;
		winfrip_hover_start.x = x_start;
		term_update(term);
	    }
	}
	break;

    /*
     * XXX document
     */
    case WINFRIP_HOVER_OP_MOUSE_DOWN:
    case WINFRIP_HOVER_OP_MOUSE_UP:
	if (winfrip_hover_state == WINFRIP_HOVER_STATE_SELECT) {
	    if (winfrip_hover_end.x > winfrip_hover_start.x) {
		/*
		 * XXX document
		*/
		hover_len = winfrip_hover_end.x - winfrip_hover_start.x;
		new_buf_w_size = (hover_len + 1) * sizeof(*winfrip_hover_url_w);
		if (new_buf_w_size > winfrip_hover_url_w_size) {
		    if ((new_buf_w = malloc(new_buf_w_size))) {
			winfrip_hover_url_w = new_buf_w;
			winfrip_hover_url_w_size = new_buf_w_size;
		    } else {
			return TRUE;
		    }
		}

		/*
		 * XXX document
		*/
		for (idx = 0; idx < hover_len; idx++) {
		    wch = term->disptext[winfrip_hover_start.y]->chars[winfrip_hover_start.x + idx].chr;
		    if (DIRECT_FONT(wch)) {
			wch &= 0xFF;
		    }
		    winfrip_hover_url_w[idx] = wch;
		}
		winfrip_hover_url_w[hover_len] = L'\0';

		/*
		 * XXX document
		*/
		match_spec_conf = conf_get_str(conf, CONF_frip_hover_match_spec);
		if (!winfrip_hover_match_spec_conf ||
		    (winfrip_hover_match_spec_conf != match_spec_conf))
		{
		    if (!winfrip_towcsdup(match_spec_conf, strlen(match_spec_conf) + 1, &winfrip_hover_match_spec_w)) {
			return TRUE;
		    } else {
			winfrip_hover_match_spec_conf = match_spec_conf;
		    }
		}

		/*
 		 * XXX document
 		 */
		if (PathMatchSpecW(winfrip_hover_url_w, winfrip_hover_match_spec_w)) {
		    winfrip_hover_state = WINFRIP_HOVER_STATE_CLICK;
		    term_update(term);
		} else {
		    ZeroMemory(winfrip_hover_url_w, winfrip_hover_url_w_size);
		}
	    }
	    return TRUE;
	} else if (winfrip_hover_state == WINFRIP_HOVER_STATE_CLICK) {
	    ShellExecuteW(NULL, L"open", winfrip_hover_url_w, NULL, NULL, SW_SHOWNORMAL);
	    ZeroMemory(winfrip_hover_url_w, winfrip_hover_url_w_size);
	    winfrip_hover_state = WINFRIP_HOVER_STATE_NONE;
	    term_update(term);
	    return TRUE;
	}
	break;
    }
    return FALSE;
}

void winfrip_transp_op(WinFripTranspOp op, HWND hwnd)
{
    LONG_PTR ex_style;
    int opacity;


    /*
     * XXX document
     */
    switch (conf_get_int(conf, CONF_frip_transp_setting)) {
    case WINFRIP_TRANSP_SETTING_OFF:
	opacity = WINFRIP_TRANSP_LEVEL_OFF; break;
    case WINFRIP_TRANSP_SETTING_LOW:
	opacity = WINFRIP_TRANSP_LEVEL_LOW; break;
    case WINFRIP_TRANSP_SETTING_MEDIUM:
	opacity = WINFRIP_TRANSP_LEVEL_MEDIUM; break;
    case WINFRIP_TRANSP_SETTING_HIGH:
	opacity = WINFRIP_TRANSP_LEVEL_HIGH; break;
    case WINFRIP_TRANSP_SETTING_CUSTOM:
	opacity = conf_get_int(conf, CONF_frip_transp_custom); break;
    default:
	return;
    }

    /*
     * XXX document
     */
    switch (op) {
    case WINFRIP_TRANSP_OP_FOCUS_KILL:
	switch (conf_get_int(conf, CONF_frip_transp_opaque_on)) {
	case WINFRIP_TRANSP_OPAQUE_FOCUS_KILL:
	    ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED;
	    opacity = 255;
	    break;
	case WINFRIP_TRANSP_OPAQUE_NEVER:
	case WINFRIP_TRANSP_OPAQUE_FOCUS_SET:
	    ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED;
	    break;
	}
	break;
    case WINFRIP_TRANSP_OP_FOCUS_SET:
	switch (conf_get_int(conf, CONF_frip_transp_opaque_on)) {
	case WINFRIP_TRANSP_OPAQUE_FOCUS_SET:
	    ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED;
	    opacity = 255;
	    break;
	case WINFRIP_TRANSP_OPAQUE_NEVER:
	case WINFRIP_TRANSP_OPAQUE_FOCUS_KILL:
	    ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED;
	    break;
	}
	break;
    }
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, ex_style);
    SetLayeredWindowAttributes(hwnd, 0, opacity, LWA_ALPHA);
}
