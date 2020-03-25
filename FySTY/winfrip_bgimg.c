/*
 * winfrip_bgimg.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "FySTY/winfrip.h"
#include "FySTY/winfrip_priv.h"

#include <gdiplus/gdiplus.h>
#include <gdiplus/gdiplusflat.h>

/*
 * Preprocessor macros
 */

/*
 * XXX document
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
 * {External,Static} variables
 */

/*
 * XXX document
 */
static HDC winfripp_bgimg_hdc = NULL;
static HGDIOBJ winfripp_bgimg_hdc_old = NULL;

/*
 * XXX document
 */
static WinFrippBgImgState winfripp_bgimg_state = WINFRIPP_BGIMG_STATE_NONE;

/*
 * Private subroutine prototypes
 */

static BOOL winfripp_init_bgimg_get_fname(Conf *conf, wchar_t **pbg_bmp_fname_w, BOOL *pbg_bmpfl);
static BOOL winfripp_init_bgimg_load_bmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc);
static BOOL winfripp_init_bgimg_load_nonbmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc);
static BOOL winfripp_init_bgimg_process(HDC bg_hdc, int bg_height, int bg_width, HBITMAP bmp_src, Conf *conf, HDC hdc);
static BOOL winfripp_init_bgimg_process_blend(HDC bg_hdc, int bg_height, int bg_width, Conf *conf);
static BOOL winfripp_init_bgimg(Conf *conf, HDC hdc, BOOL force);
/*
 * Private subroutines
 */

static BOOL winfripp_init_bgimg_get_fname(Conf *conf, wchar_t **pbg_fname_w, BOOL *pbg_bmpfl)
{
    char *bg_fname;
    Filename *bg_fname_conf;
    size_t bg_fname_len;
    BOOL rc;


    WINFRIPP_DEBUG_ASSERT(pbg_fname_w);
    WINFRIPP_DEBUG_ASSERT(pbg_bmpfl);

    /*
     * XXX document
     */
    bg_fname_conf = conf_get_filename(conf, CONF_frip_bgimg_filename);
    bg_fname = bg_fname_conf->path;
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
	    return rc;
	}
    }
    return FALSE;
}

static BOOL winfripp_init_bgimg_load_bmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc)
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
    WINFRIPP_DEBUG_ASSERT(rc == TRUE);
    return rc;
}

static BOOL winfripp_init_bgimg_load_nonbmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc)
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

    /*
     * XXX document
     */
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
    WINFRIPP_DEBUG_ASSERT(rc == TRUE);
    return rc;
}

static BOOL winfripp_init_bgimg_process(HDC bg_hdc, int bg_height, int bg_width, HBITMAP bmp_src, Conf *conf, HDC hdc)
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


    WINFRIPP_DEBUG_ASSERT(bg_hdc);
    WINFRIPP_DEBUG_ASSERT(bg_height > 0);
    WINFRIPP_DEBUG_ASSERT(bg_width > 0);
    WINFRIPP_DEBUG_ASSERT(bmp_src);
    WINFRIPP_DEBUG_ASSERT(hdc);

    /*
     * XXX document
     */
    SetRect(&bg_rect, 0, 0, bg_width, bg_height);
    bgimg_style = conf_get_int(conf, CONF_frip_bgimg_style);
    switch (bgimg_style) {
    default:
	WINFRIPP_DEBUG_FAIL(); break;

    /*
     * XXX document
     */
    case WINFRIPP_BGIMG_STYLE_ABSOLUTE:
    case WINFRIPP_BGIMG_STYLE_CENTER:
    case WINFRIPP_BGIMG_STYLE_STRETCH:
	if ((bmp_src_hdc = CreateCompatibleDC(bg_hdc))) {
	    GetObject(bmp_src, sizeof(bmp_src_obj), &bmp_src_obj);
	    bmp_src_old = SelectObject(bmp_src_hdc, bmp_src);
	    switch (bgimg_style) {
	    default:
		WINFRIPP_DEBUG_FAIL(); break;

	    /*
	     * XXX document
	     */
	    case WINFRIPP_BGIMG_STYLE_ABSOLUTE:
		rc = BitBlt(bg_hdc, bg_rect.left, bg_rect.top,
			    bmp_src_obj.bmWidth, bmp_src_obj.bmHeight,
			    bmp_src_hdc, 0, 0, SRCCOPY) > 0;
		break;

	    /*
	     * XXX document
	     */
	    case WINFRIPP_BGIMG_STYLE_CENTER:
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

static BOOL winfripp_init_bgimg_process_blend(HDC bg_hdc, int bg_height, int bg_width, Conf *conf)
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
    WINFRIPP_DEBUG_ASSERT(rc == TRUE);
    return rc;
}

static BOOL winfripp_init_bgimg(Conf *conf, HDC hdc, BOOL force)
{
    wchar_t *bg_bmp_fname_w;
    BOOL bg_bmpfl;
    HDC bg_hdc = NULL;
    HGDIOBJ bg_hdc_cur, bg_hdc_old = NULL;
    int bg_height, bg_width;
    HBITMAP bmp_src = NULL;
    BOOL rc = FALSE;


    WINFRIPP_DEBUG_ASSERT(hdc);

    /*
     * XXX document
     */
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

    /*
     * XXX document
     */
    if (winfripp_bgimg_hdc) {
	bg_hdc_cur = GetCurrentObject(winfripp_bgimg_hdc, OBJ_BITMAP);
	SelectObject(winfripp_bgimg_hdc, winfripp_bgimg_hdc_old); winfripp_bgimg_hdc_old = NULL;
	DeleteObject(bg_hdc_cur);
	DeleteDC(winfripp_bgimg_hdc); winfripp_bgimg_hdc = NULL;
    }

    /*
     * XXX document
     */
    if ((winfripp_init_bgimg_get_fname(conf, &bg_bmp_fname_w, &bg_bmpfl))) {
	switch (bg_bmpfl) {
	case TRUE:
	    rc = winfripp_init_bgimg_load_bmp(&bg_hdc, &bg_hdc_old, &bg_height, &bg_width, &bmp_src, bg_bmp_fname_w, hdc);
	    break;

	default:
	case FALSE:
	    rc = winfripp_init_bgimg_load_nonbmp(&bg_hdc, &bg_hdc_old, &bg_height, &bg_width, &bmp_src, bg_bmp_fname_w, hdc);
	    break;
	}
	sfree(bg_bmp_fname_w);
    }

    /*
     * XXX document
     */
    if (rc) {
	if (winfripp_init_bgimg_process(bg_hdc, bg_height, bg_width, bmp_src, conf, hdc)) {
	    if (winfripp_init_bgimg_process_blend(bg_hdc, bg_height, bg_width, conf)) {
		DeleteObject(bmp_src);
		winfripp_bgimg_hdc = bg_hdc; winfripp_bgimg_hdc_old = bg_hdc_old;
		winfripp_bgimg_state = WINFRIPP_BGIMG_STATE_INIT;
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
    winfripp_bgimg_state = WINFRIPP_BGIMG_STATE_FAILED;
    WINFRIPP_DEBUG_FAIL();
    return FALSE;
}

/*
 * Public subroutines private to FySTY/winfrip*.c
 */

void winfripp_bgimg_config_panel(struct controlbox *b)
{
    struct controlset *s;


    WINFRIPP_DEBUG_ASSERT(b);

    /*
     * The Frippery: background panel.
     */

    ctrl_settitle(b, "Frippery/Background", "Configure pointless frippery: background image");
    s = ctrl_getset(b, "Frippery/Background", "frip_bgimg", "Background image settings");
    ctrl_filesel(s, "Image file:", NO_SHORTCUT,
		 WINFRIPP_BGIMG_FILTER_IMAGE_FILES, FALSE, "Select background image file",
		 P(WINFRIPP_HELP_CTX), conf_filesel_handler, I(CONF_frip_bgimg_filename));
    ctrl_editbox(s, "Opacity (0-100):", NO_SHORTCUT, 20, P(WINFRIPP_HELP_CTX),
		 conf_editbox_handler, I(CONF_frip_bgimg_opacity), I(-1));
    ctrl_radiobuttons(s, "Style:", NO_SHORTCUT, 4, P(WINFRIPP_HELP_CTX),
		      conf_radiobutton_handler, I(CONF_frip_bgimg_style),
		      "Absolute",	NO_SHORTCUT,	I(WINFRIPP_BGIMG_STYLE_ABSOLUTE),
		      "Center",		NO_SHORTCUT,	I(WINFRIPP_BGIMG_STYLE_CENTER),
		      "Stretch",	NO_SHORTCUT,	I(WINFRIPP_BGIMG_STYLE_STRETCH),
		      "Tile",		NO_SHORTCUT,	I(WINFRIPP_BGIMG_STYLE_TILE), NULL);
    ctrl_radiobuttons(s, "Type:", NO_SHORTCUT, 2, P(WINFRIPP_HELP_CTX),
		      conf_radiobutton_handler, I(CONF_frip_bgimg_type),
		      "Solid",		NO_SHORTCUT,	I(WINFRIPP_BGIMG_TYPE_SOLID),
		      "Image",		NO_SHORTCUT,	I(WINFRIPP_BGIMG_TYPE_IMAGE), NULL);
}

/*
 * Public subroutines
 */

WinFripReturn winfrip_bgimg_op(WinFripBgImgOp op, BOOL *pbgfl, Conf *conf, HDC hdc_in, HWND hwnd, int char_width, int font_height, int len, int nbg, int rc_width, int x, int y)
{
    HDC hdc;
    WinFripReturn rc;


    /*
     * XXX document
     */
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

    /*
     * XXX document
     */
    case WINFRIP_BGIMG_OP_DRAW:
	WINFRIPP_DEBUG_ASSERT(hdc);
	WINFRIPP_DEBUG_ASSERT(font_height > 0);
	WINFRIPP_DEBUG_ASSERT(len > 0);
	if ((nbg == 258) || (nbg == 259)) {
	    if (winfripp_bgimg_hdc || winfripp_init_bgimg(conf, hdc, FALSE)) {
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

    /*
     * XXX document
     */
    case WINFRIP_BGIMG_OP_RECONF:
	if (winfripp_init_bgimg(conf, hdc, TRUE)) {
	    rc = WINFRIP_RETURN_CONTINUE;
	    goto out;
	} else {
	    WINFRIPP_DEBUG_FAIL();
	    rc = WINFRIP_RETURN_FAILURE;
	    goto out;
	}

    /*
     * XXX document
     */
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
	case WINFRIPP_BGIMG_STYLE_STRETCH:
	    if (winfripp_init_bgimg(conf, hdc, TRUE)) {
		rc = WINFRIP_RETURN_CONTINUE;
		goto out;
	    } else {
		WINFRIPP_DEBUG_FAIL();
		rc = WINFRIP_RETURN_FAILURE;
		goto out;
	    }
	}
    }

    /*
     * XXX document
     */
out:
    if (!hdc_in) {
	ReleaseDC(hwnd, hdc);
    }
    return rc;
}
