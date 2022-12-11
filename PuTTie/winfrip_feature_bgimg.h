/*
 * winfrip_feature_bgimg.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_FEATURE_BGIMG_H
#define PUTTY_WINFRIP_FEATURE_BGIMG_H

/*
 * Public type definitions used by/in:
 * windows/window.c:{do_text_internal,WndProc}()
 */

typedef enum WinFripBgImgOp {
	WINFRIP_BGIMG_OP_DRAW				= 1,
	WINFRIP_BGIMG_OP_INIT				= 2,
	WINFRIP_BGIMG_OP_RECONF				= 3,
	WINFRIP_BGIMG_OP_SIZE				= 4,
} WinFripBgImgOp;

/*
 * Public subroutine prototypes used by/in:
 * windows/window.c:{do_text_internal,WndProc}()
 */

WinFripReturn winfrip_bgimg_op(WinFripBgImgOp op, BOOL *pbgfl, Conf *conf, HDC hdc_in, HWND hwnd, int char_width, int font_height, int len, int nbg, int rc_width, int x, int y);

#endif // !PUTTY_WINFRIP_FEATURE_BGIMG_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
