/*
 * winfrip_feature_general.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_FEATURE_GENERAL_H
#define PUTTY_WINFRIP_FEATURE_GENERAL_H

/*
 * Public type definitions used by/in:
 * windows/window.c:WndProc()
 */

typedef enum WinFripGeneralOp {
	WINFRIP_GENERAL_OP_CONFIG_DIALOG	= 1,
	WINFRIP_GENERAL_OP_FOCUS_SET		= 2,
	WINFRIP_GENERAL_OP_SYSTRAY_INIT		= 3,
	WINFRIP_GENERAL_OP_SYSTRAY_MINIMISE	= 4,
	WINFRIP_GENERAL_OP_SYSTRAY_WM_MENU	= 5,
	WINFRIP_GENERAL_OP_SYSTRAY_WM_OTHER	= 6,
} WinFripGeneralOp;

/*
 * Public subroutine prototypes used by/in:
 * windows/window.c:WndProc()
 */

UINT winfripp_general_get_wm_systray(void);

WinFripReturn winfrip_general_op(WinFripGeneralOp op, Conf *conf, HINSTANCE hinst, HWND hwnd, LPARAM lParam, int reconfiguring, WPARAM wParam);

#endif // !PUTTY_WINFRIP_FEATURE_GENERAL_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
