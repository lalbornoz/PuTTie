/*
 * winfrip_rtl_gdi.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <windows.h>

#include <gdiplus/gdiplus.h>
#include <gdiplus/gdiplusflat.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_gdi.h"

/*
 * Private subroutine prototypes
 */

static WfrStatus WfrpLoadImageBmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *psrc, wchar_t *src_fname_w, HDC hdc);
static WfrStatus WfrpLoadImageNonBmp(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *psrc, wchar_t *src_fname_w, HDC hdc);
static WfrStatus WfrpTransferImageAbsolute(HDC bg_hdc, int bg_height, int bg_width, HBITMAP bmp_src);
static WfrStatus WfrpTransferImageCenter(HDC bg_hdc, int bg_height, int bg_width, HBITMAP bmp_src, HDC hdc);
static WfrStatus WfrpTransferImageFit(HDC bg_hdc, int bg_height, int bg_width, HBITMAP bmp_src, HDC hdc, int padding);
static WfrStatus WfrpTransferImageStretch(HDC bg_hdc, int bg_height, int bg_width, HBITMAP bmp_src);
static WfrStatus WfrpTransferImageTile(HDC bg_hdc, int bg_height, int bg_width, HBITMAP bmp_src);

/*
 * Private subroutines
 */

static WfrStatus
WfrpLoadImageBmp(
	HDC *		pbg_hdc,
	HGDIOBJ *	pbg_hdc_old,
	int *		pbg_height,
	int *		pbg_width,
	HBITMAP *	psrc,
	wchar_t *	src_fname_w,
	HDC		hdc
	)
{
	HBITMAP		bg_bmp;
	HDC		bg_hdc;
	HGDIOBJ		bg_hdc_old;
	int		bg_height, bg_width;
	HBITMAP		src;
	WfrStatus	status;


	bg_height = GetDeviceCaps(hdc, VERTRES);
	bg_width = GetDeviceCaps(hdc, HORZRES);

	if ((src = LoadImageW(0, src_fname_w, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE))
	&&  (bg_hdc = CreateCompatibleDC(hdc))
	&&  (bg_bmp = CreateCompatibleBitmap(hdc, bg_width, bg_height)))
	{
		bg_hdc_old = SelectObject(bg_hdc, bg_bmp);

		*pbg_hdc = bg_hdc; *pbg_hdc_old = bg_hdc_old;
		*pbg_height = bg_height; *pbg_width = bg_width;
		*psrc = src;

		status = WFR_STATUS_CONDITION_SUCCESS;
	} else {
		status = WFR_STATUS_FROM_WINDOWS();
	}

	return status;
}

static WfrStatus
WfrpLoadImageNonBmp(
	HDC *		pbg_hdc,
	HGDIOBJ *	pbg_hdc_old,
	int *		pbg_height,
	int *		pbg_width,
	HBITMAP *	psrc,
	wchar_t *	src_fname_w,
	HDC		hdc
	)
{
	HBITMAP			bg_bmp;
	HDC			bg_hdc = NULL;
	HGDIOBJ			bg_hdc_old;
	int			bg_height, bg_width;
	HBITMAP			src;

	GpBitmap *		gdip_bmp = NULL;
	bool			gdip_initfl;
	GdiplusStartupInput	gdip_si;
	GpStatus		gdip_status;
	ULONG_PTR		gdip_token;

	WfrStatus		status;


	gdip_si.GdiplusVersion = 2;
	gdip_si.DebugEventCallback = NULL;
	gdip_si.SuppressBackgroundThread = FALSE;
	gdip_si.SuppressExternalCodecs = FALSE;

	gdip_initfl = false;

	if (((gdip_status = GdiplusStartup(&gdip_token, &gdip_si, NULL)) == Ok)
	&&  ((gdip_status = GdipCreateBitmapFromFile(src_fname_w, &gdip_bmp)) == Ok)
	&&  ((gdip_status = GdipCreateHBITMAPFromBitmap(gdip_bmp, &src, 0) == Ok)))
	{
		bg_height = GetDeviceCaps(hdc, VERTRES);
		bg_width = GetDeviceCaps(hdc, HORZRES);

		if ((bg_hdc = CreateCompatibleDC(hdc))
		&&  (bg_bmp = CreateCompatibleBitmap(hdc, bg_width, bg_height)))
		{
			bg_hdc_old = SelectObject(bg_hdc, bg_bmp);
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else {
			status = WFR_STATUS_FROM_WINDOWS();
		}
	} else {
		status = WFR_STATUS_FROM_GDI_PLUS(gdip_status);
	}


	if (gdip_bmp) {
		(void)GdipDisposeImage(gdip_bmp);
	}
	if (gdip_initfl) {
		GdiplusShutdown(gdip_token);
	}

	if (WFR_STATUS_SUCCESS(status)) {
		*pbg_hdc = bg_hdc; *pbg_hdc_old = bg_hdc_old;
		*pbg_height = bg_height; *pbg_width = bg_width;
		*psrc = src;
	} else {
		if (bg_hdc) {
			(void)DeleteDC(bg_hdc);
		}
	}

	return status;
}

static WfrStatus
WfrpTransferImageAbsolute(
	HDC		bg_hdc,
	int		bg_height,
	int		bg_width,
	HBITMAP		bmp_src
	)
{
	RECT		bg_rect;
	HDC		bmp_src_hdc;
	BITMAP		bmp_src_obj;
	HGDIOBJ		bmp_src_old;
	WfrStatus	status;


	if ((bmp_src_hdc = CreateCompatibleDC(bg_hdc))) {
		(void)SetRect(&bg_rect, 0, 0, bg_width, bg_height);
		(void)GetObject(bmp_src, sizeof(bmp_src_obj), &bmp_src_obj);
		bmp_src_old = SelectObject(bmp_src_hdc, bmp_src);

		if (BitBlt(
				bg_hdc, bg_rect.left, bg_rect.top,
				bmp_src_obj.bmWidth, bmp_src_obj.bmHeight,
				bmp_src_hdc, 0, 0, SRCCOPY) == TRUE)
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else {
			status = WFR_STATUS_FROM_WINDOWS();
		}

		(void)SelectObject(bmp_src_hdc, bmp_src_old);
		(void)DeleteDC(bmp_src_hdc);
	} else {
		status = WFR_STATUS_FROM_WINDOWS();
	}

	return status;
}

static WfrStatus
WfrpTransferImageCenter(
	HDC		bg_hdc,
	int		bg_height,
	int		bg_width,
	HBITMAP		bmp_src,
	HDC		hdc
	)
{
	RECT		bg_rect;
	int		bmp_offset_x, bmp_offset_y;
	HDC		bmp_src_hdc;
	BITMAP		bmp_src_obj;
	HGDIOBJ		bmp_src_old;
	RECT		cr;
	int		cr_height, cr_width;
	HWND		hwnd;
	WfrStatus	status;


	if ((bmp_src_hdc = CreateCompatibleDC(bg_hdc))) {
		(void)SetRect(&bg_rect, 0, 0, bg_width, bg_height);
		(void)GetObject(bmp_src, sizeof(bmp_src_obj), &bmp_src_obj);
		bmp_src_old = SelectObject(bmp_src_hdc, bmp_src);

		hwnd = WindowFromDC(hdc);
		(void)GetClientRect(hwnd, &cr);
		cr_height = (cr.bottom - cr.top);
		cr_width = (cr.right - cr.left);

		if (cr_width > bmp_src_obj.bmWidth) {
			bmp_offset_x = (cr_width - bmp_src_obj.bmWidth) / 2;
			bg_rect.left += bmp_offset_x;
			bg_rect.right -= bmp_offset_x;
		}
		if (cr_height > bmp_src_obj.bmHeight) {
			bmp_offset_y = (cr_height - bmp_src_obj.bmHeight) / 2;
			bg_rect.top += bmp_offset_y;
			bg_rect.bottom -= bmp_offset_y;
		}

		if (BitBlt(
				bg_hdc, bg_rect.left, bg_rect.top,
				bg_rect.right - bg_rect.left,
				bg_rect.bottom - bg_rect.top,
				bmp_src_hdc, 0, 0, SRCCOPY) == TRUE)
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else {
			status = WFR_STATUS_FROM_WINDOWS();
		}

		(void)SelectObject(bmp_src_hdc, bmp_src_old);
		(void)DeleteDC(bmp_src_hdc);
	} else {
		status = WFR_STATUS_FROM_WINDOWS();
	}

	return status;
}

static WfrStatus
WfrpTransferImageFit(
	HDC		bg_hdc,
	int		bg_height,
	int		bg_width,
	HBITMAP		bmp_src,
	HDC		hdc,
	int		padding
	)
{

	int		bg_hdc_sb_mode;
	RECT		bg_rect;
	HDC		bmp_src_hdc;
	BITMAP		bmp_src_obj;
	HGDIOBJ		bmp_src_old;
	RECT		cr;
	int		cr_height, cr_width;
	HWND		hwnd;
	float		ratio, ratioH, ratioW;
	WfrStatus	status;
	int		x, y;


	if ((bmp_src_hdc = CreateCompatibleDC(bg_hdc))) {
		(void)SetRect(&bg_rect, 0, 0, bg_width, bg_height);
		(void)GetObject(bmp_src, sizeof(bmp_src_obj), &bmp_src_obj);
		bmp_src_old = SelectObject(bmp_src_hdc, bmp_src);

		hwnd = WindowFromDC(hdc);
		GetClientRect(hwnd, &cr);
		cr_height = (cr.bottom - cr.top);
		cr_width = (cr.right - cr.left);

		ratioH = (float)cr_height / (float)bmp_src_obj.bmHeight;
		ratioW = (float)cr_width / (float)bmp_src_obj.bmWidth;
		if ((padding % 100) > 0) {
			ratioH -= ((float)padding / 100) * ratioH;
			ratioW -= ((float)padding / 100) * ratioW;
		}
		ratio = ratioH < ratioW ? ratioH : ratioW;

		x = (int)(((float)cr_width - ((float)bmp_src_obj.bmWidth * ratio)) / 2);
		y = (int)(((float)cr_height - ((float)bmp_src_obj.bmHeight * ratio)) / 2);

		bg_hdc_sb_mode = SetStretchBltMode(bg_hdc, HALFTONE);
		if (StretchBlt(
				bg_hdc, x, y,
				(int)((float)bmp_src_obj.bmWidth * ratio),
				(int)((float)bmp_src_obj.bmHeight * ratio),
				bmp_src_hdc,
				0, 0,
				bmp_src_obj.bmWidth, bmp_src_obj.bmHeight,
				SRCCOPY) == TRUE)
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else {
			status = WFR_STATUS_FROM_WINDOWS();
		}
		(void)SetStretchBltMode(bg_hdc, bg_hdc_sb_mode);

		(void)SelectObject(bmp_src_hdc, bmp_src_old);
		(void)DeleteDC(bmp_src_hdc);
	} else {
		status = WFR_STATUS_FROM_WINDOWS();
	}

	return status;
}

static WfrStatus
WfrpTransferImageStretch(
	HDC		bg_hdc,
	int		bg_height,
	int		bg_width,
	HBITMAP		bmp_src
	)
{
	int		bg_hdc_sb_mode;
	RECT		bg_rect;
	HDC		bmp_src_hdc;
	BITMAP		bmp_src_obj;
	HGDIOBJ		bmp_src_old;
	WfrStatus	status;


	if ((bmp_src_hdc = CreateCompatibleDC(bg_hdc))) {
		(void)SetRect(&bg_rect, 0, 0, bg_width, bg_height);
		(void)GetObject(bmp_src, sizeof(bmp_src_obj), &bmp_src_obj);
		bmp_src_old = SelectObject(bmp_src_hdc, bmp_src);

		bg_hdc_sb_mode = SetStretchBltMode(bg_hdc, HALFTONE);
		if (StretchBlt(
				bg_hdc, bg_rect.left, bg_rect.top,
				bg_rect.right - bg_rect.left,
				bg_rect.bottom - bg_rect.top,
				bmp_src_hdc, 0, 0,
				bmp_src_obj.bmWidth, bmp_src_obj.bmHeight, SRCCOPY) == TRUE)
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else {
			status = WFR_STATUS_FROM_WINDOWS();
		}

		(void)SetStretchBltMode(bg_hdc, bg_hdc_sb_mode);

		(void)SelectObject(bmp_src_hdc, bmp_src_old);
		(void)DeleteDC(bmp_src_hdc);
	} else {
		status = WFR_STATUS_FROM_WINDOWS();
	}

	return status;
}

static WfrStatus
WfrpTransferImageTile(
	HDC		bg_hdc,
	int		bg_height,
	int		bg_width,
	HBITMAP		bmp_src
	)
{
	HBRUSH		bg_brush;
	RECT		bg_rect;
	WfrStatus	status;


	(void)SetRect(&bg_rect, 0, 0, bg_width, bg_height);
	if ((bg_brush = CreatePatternBrush(bmp_src))) {
		if (FillRect(bg_hdc, &bg_rect, bg_brush) == TRUE) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else {
			status = WFR_STATUS_FROM_WINDOWS();
		}
		(void)DeleteObject(bg_brush);
	} else {
		status = WFR_STATUS_FROM_WINDOWS();
	}

	return status;
}

/*
 * Public subroutines
 */

WfrStatus
WfrBlendImage(
	HDC	bg_hdc,
	int	bg_height,
	int	bg_width,
	int	opacity
	)
{
	RECT		bg_rect;
	HBITMAP		blend_bmp = NULL;
	COLORREF	blend_colour;
	BLENDFUNCTION	blend_ftn;
	HDC		blend_hdc = NULL;
	HGDIOBJ		blend_old = NULL;
	WfrStatus	status;


	if ((blend_hdc = CreateCompatibleDC(bg_hdc))
	&&  (blend_bmp = CreateCompatibleBitmap(bg_hdc, 1, 1)))
	{
		blend_old = SelectObject(blend_hdc, blend_bmp);
		blend_colour = RGB(0, 0, 0);
		(void)SetPixelV(blend_hdc, 0, 0, blend_colour);

		(void)SetRect(&bg_rect, 0, 0, bg_width, bg_height);
		blend_ftn.AlphaFormat = 0;
		blend_ftn.BlendFlags = 0;
		blend_ftn.BlendOp = AC_SRC_OVER;
		blend_ftn.SourceConstantAlpha = (0xff * opacity) / 100;

		if (AlphaBlend(
				bg_hdc,
				bg_rect.left, bg_rect.top,
				bg_rect.right - bg_rect.left,
				bg_rect.bottom - bg_rect.top,
				blend_hdc, 0, 0, 1, 1, blend_ftn) == TRUE)
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else {
			status = WFR_STATUS_FROM_WINDOWS();
		}
	} else {
		status = WFR_STATUS_FROM_WINDOWS();
	}


	if (blend_hdc) {
		if (blend_old) {
			(void)SelectObject(blend_hdc, blend_old);
		}
		(void)DeleteDC(blend_hdc);
	}
	if (blend_bmp) {
		(void)DeleteObject(blend_bmp);
	}

	return status;
}

WfrStatus
WfrFreeBitmapDC(
	HDC *		phdc,
	HGDIOBJ *	phdc_old
)
{
	HGDIOBJ		hdc_cur;
	WfrStatus	status;


	if (*phdc) {
		hdc_cur = GetCurrentObject(*phdc, OBJ_BITMAP);
		if (*phdc_old) {
			(void)SelectObject(*phdc, *phdc_old); *phdc_old = NULL;
		}
		(void)DeleteObject(hdc_cur);
		(void)DeleteDC(*phdc); *phdc = NULL;
		status = WFR_STATUS_CONDITION_SUCCESS;
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfrLoadImage(
	HDC *		pbg_hdc,
	HGDIOBJ *	pbg_hdc_old,
	int *		pbg_height,
	int *		pbg_width,
	HBITMAP *	psrc,
	wchar_t *	src_fname_w,
	HDC		hdc
	)
{
	size_t		src_fname_w_len;
	WfrStatus	status;


	src_fname_w_len = wcslen(src_fname_w);
	if ((src_fname_w_len > (sizeof(L".bmp") - 1))
	&&  (        src_fname_w[src_fname_w_len - 4] == L'.')
	&&  (towlower(src_fname_w[src_fname_w_len - 3]) == L'b')
	&&  (towlower(src_fname_w[src_fname_w_len - 2]) == L'm')
	&&  (towlower(src_fname_w[src_fname_w_len - 1]) == L'p'))
	{
		status = WfrpLoadImageBmp(
			pbg_hdc, pbg_hdc_old, pbg_height,
			pbg_width, psrc, src_fname_w, hdc);
	} else {
		status = WfrpLoadImageNonBmp(
			pbg_hdc, pbg_hdc_old, pbg_height,
			pbg_width, psrc, src_fname_w, hdc);
	}

	return status;
}

WfrStatus
WfrTransferImage(
	HDC			bg_hdc,
	int			bg_height,
	int			bg_width,
	HBITMAP			bmp_src,
	HDC			hdc,
	int			padding,
	WfrTransferImageStyle	style
	)
{
	WfrStatus	status;


	switch (style) {
	case WFR_TI_STYLE_ABSOLUTE:
		status = WfrpTransferImageAbsolute(bg_hdc, bg_height, bg_width, bmp_src);
		break;

	case WFR_TI_STYLE_CENTER:
		status = WfrpTransferImageCenter(bg_hdc, bg_height, bg_width, bmp_src, hdc);
		break;

	case WFR_TI_STYLE_FIT:
		status = WfrpTransferImageFit(bg_hdc, bg_height, bg_width, bmp_src, hdc, padding);
		break;

	case WFR_TI_STYLE_STRETCH:
		status = WfrpTransferImageStretch(bg_hdc, bg_height, bg_width, bmp_src);
		break;

	case WFR_TI_STYLE_TILE:
		status = WfrpTransferImageTile(bg_hdc, bg_height, bg_width, bmp_src);
		break;

	default:
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
