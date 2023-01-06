/*
 * winfrip_feature_urls.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_FEATURE_URLS_H
#define PUTTY_WINFRIP_FEATURE_URLS_H

/*
 * Public subroutine prototypes used by/in:
 * winfrip_putty_config.c
 */

void WffUrlsConfigPanel(struct controlbox *b);

/*
 * Public type definitions used by/in:
 * terminal/terminal.c:do_paint()
 * windows/window.c:WndProc()
 */

typedef enum WffUrlsOp {
	WFF_URLS_OP_DRAW				= 1,
	WFF_URLS_OP_FOCUS_KILL			= 2,
	WFF_URLS_OP_MOUSE_BUTTON_EVENT	= 3,
	WFF_URLS_OP_MOUSE_MOTION_EVENT	= 4,
	WFF_URLS_OP_RECONFIG			= 5,
} WffUrlsOp;

/*
 * Public subroutine prototypes used by/in:
 * terminal/terminal.c:do_paint()
 * windows/window.c:WndProc()
 */

WfReturn WffUrlsOperation(WffUrlsOp op, Conf *conf, HWND hwnd, UINT message, unsigned long *tattr, Terminal *term, WPARAM wParam, int x, int y);

#endif // !PUTTY_WINFRIP_FEATURE_URLS_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
