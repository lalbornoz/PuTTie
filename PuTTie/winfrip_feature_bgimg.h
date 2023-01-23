/*
 * winfrip_feature_bgimg.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_FEATURE_BGIMG_H
#define PUTTY_WINFRIP_FEATURE_BGIMG_H

/*
 * Public defaults
 */

#define	WFF_BGIMG_DEFAULT_OPACITY		75
#define	WFF_BGIMG_DEFAULT_PADDING		10
#define	WFF_BGIMG_DEFAULT_SLIDESHOW		0
#define	WFF_BGIMG_DEFAULT_SLIDESHOW_FREQ	3600
#define	WFF_BGIMG_DEFAULT_STYLE			0
#define	WFF_BGIMG_DEFAULT_TYPE			0

/*
 * Public subroutine prototypes used by/in:
 * winfrip_putty_config.c
 */

void		WffBgImgConfigPanel(struct controlbox *b);

/*
 * Public type definitions and subroutine prototypes used by/in:
 * windows/window.c:{do_text_internal,WndProc}()
 */

typedef enum WffBgImgOp {
	WFF_BGIMG_OP_DRAW	= 1,
	WFF_BGIMG_OP_INIT	= 2,
	WFF_BGIMG_OP_RECONF	= 3,
	WFF_BGIMG_OP_SIZE	= 4,
} WffBgImgOp;

WfReturn	WffBgImgOperation(WffBgImgOp op, BOOL *pbgfl, Conf *conf, HDC hdc_in, HWND hwnd, int char_width, int font_height, int len, int nbg, int rc_width, int x, int y);

#endif // !PUTTY_WINFRIP_FEATURE_BGIMG_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
