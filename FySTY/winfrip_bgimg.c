/*
 * winfrip_bgimg.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "winfrip.h"
#include "winfrip_priv.h"

#include <assert.h>
#include <gdiplus/gdiplus.h>
#include <gdiplus/gdiplusflat.h>

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
 * Private subroutine prototypes
 */

static BOOL winfrip_init_bgimg_get_fname(wchar_t **pbg_bmp_fname_w, BOOL *pbg_bmpfl);
static BOOL winfrip_init_bgimg_load_bmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc);
static BOOL winfrip_init_bgimg_load_nonbmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *pbmp_src, wchar_t *bmp_src_fname_w, HDC hdc);
static BOOL winfrip_init_bgimg_process(HDC bg_hdc, int bg_height, int bg_width, HBITMAP bmp_src, HDC hdc);
static BOOL winfrip_init_bgimg_process_blend(HDC bg_hdc, int bg_height, int bg_width);
static BOOL winfrip_init_bgimg(HDC hdc, BOOL force);

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
	free(bg_bmp_fname_w);
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
