/*
 * winfrip_feature_general.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_FEATURE_GENERAL_H
#define PUTTY_WINFRIP_FEATURE_GENERAL_H

/*
 * Public defaults
 */

#define	WFF_GENERAL_DEFAULT_ALWAYS_ON_TOP		false
#define	WFF_GENERAL_DEFAULT_MINIMISE_TO_SYSTRAY		true
#define	WFF_GENERAL_DEFAULT_RESTART_SESSION		false

/*
 * Public subroutine prototypes used by/in:
 * winfrip_putty_config.c
 */

void		WffGeneralConfigPanel(struct controlbox *b);

/*
 * Public type definitions and subroutine prototypes used by/in:
 * windows/window.c:WndProc()
 */

typedef enum WffGeneralOp {
	WFF_GENERAL_OP_CONFIG_DIALOG		= 1,
	WFF_GENERAL_OP_FOCUS_SET		= 2,
	WFF_GENERAL_OP_SYSTRAY_INIT		= 3,
	WFF_GENERAL_OP_SYSTRAY_MINIMISE		= 4,
	WFF_GENERAL_OP_SYSTRAY_WM_MENU		= 5,
	WFF_GENERAL_OP_SYSTRAY_WM_OTHER		= 6,
	WFF_GENERAL_OP_RESTART_SESSION		= 7,
	WFF_GENERAL_OP_POSITION_RESTORE		= 8,
	WFF_GENERAL_OP_SIZE_RESTORE		= 9,
	WFF_GENERAL_OP_POSITION_SET		= 10,
	WFF_GENERAL_OP_SIZE_SET			= 11,
} WffGeneralOp;

UINT		WffGeneralGetWmSysTray(void);
WfReturn	WffGeneralOperation(WffGeneralOp op, Conf *conf, HINSTANCE hinst, HWND hwnd, LPARAM lParam, int reconfiguring, void *wgs, WPARAM wParam);

#endif // !PUTTY_WINFRIP_FEATURE_GENERAL_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
