/*
 * winfrip_feature_mouse.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_FEATURE_MOUSE_H
#define PUTTY_WINFRIP_FEATURE_MOUSE_H

/*
 * Public subroutine prototypes used by/in:
 * winfrip_putty_config.c
 */

void WffMouseConfigPanel(struct controlbox *b);

/*
 * Public type definitions used by/in:
 * windows/window.c:WndProc()
 */

typedef enum WffMouseOp {
	WFF_MOUSE_OP_MOUSE_EVENT	= 1,
	WFF_MOUSE_OP_WHEEL			= 2,
} WffMouseOp;

/*
 * Public subroutine prototypes used by/in:
 * windows/window.c:WndProc()
 */

WfReturn WffMouseOperation(WffMouseOp op, Conf *conf, UINT message, WPARAM wParam);

#endif // !PUTTY_WINFRIP_FEATURE_MOUSE_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
