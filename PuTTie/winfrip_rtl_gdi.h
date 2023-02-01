/*
 * winfrip_rtl_gdi.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_GDI_H
#define PUTTY_WINFRIP_RTL_GDI_H

/*
 * Public type definitions private to PuTTie/winfrip*.c
 */

typedef enum WfrTransferImageStyle {
	WFR_TI_STYLE_ABSOLUTE		= 0,
	WFR_TI_STYLE_CENTER		= 1,
	WFR_TI_STYLE_FIT		= 2,
	WFR_TI_STYLE_STRETCH		= 3,
	WFR_TI_STYLE_TILE		= 4,

	WFR_TI_STYLE_DEFAULT		= WFR_TI_STYLE_ABSOLUTE,
} WfrTransferImageStyle;

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

WfrStatus WfrBlendImage(HDC bg_hdc, int bg_height, int bg_width, int opacity);
WfrStatus WfrFreeBitmapDC(HDC *phdc, HGDIOBJ *phdc_old);
WfrStatus WfrLoadImage(HDC *pbg_hdc, HGDIOBJ *pbg_hdc_old, int *pbg_height, int *pbg_width, HBITMAP *psrc, wchar_t *src_fname_w, HDC hdc);
WfrStatus WfrTransferImage(HDC bg_hdc, int bg_height, int bg_width, HBITMAP bmp_src, HDC hdc, int padding, WfrTransferImageStyle style);

#endif // !PUTTY_WINFRIP_RTL_GDI_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
