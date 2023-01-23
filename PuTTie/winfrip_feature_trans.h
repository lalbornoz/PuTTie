/*
 * winfrip_feature_trans.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_FEATURE_TRANS_H
#define PUTTY_WINFRIP_FEATURE_TRANS_H

/*
 * Public defaults
 */

#define	WFF_TRANS_DEFAULT_CUSTOM	0
#define	WFF_TRANS_DEFAULT_OPAQUE_ON	1
#define	WFF_TRANS_DEFAULT_SETTING	0

/*
 * Public subroutine prototypes used by/in:
 * winfrip_putty_config.c
 */

void	WffTransConfigPanel(struct controlbox *b);

/*
 * Public type definitions and subroutine prototypes used by/in:
 * windows/window.c:{WinMain,WndProc}()
 */

typedef enum WffTransOp {
	WFF_TRANS_OP_FOCUS_KILL		= 1,
	WFF_TRANS_OP_FOCUS_SET		= 2,
} WffTransOp;

void	WffTransOperation(WffTransOp op, Conf *conf, HWND hwnd);

#endif // !PUTTY_WINFRIP_FEATURE_TRANS_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
