/*
 * winfrip_feature_trans.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_FEATURE_TRANS_H
#define PUTTY_WINFRIP_FEATURE_TRANS_H

/*
 * Public subroutine prototypes used by/in:
 * winfrip_putty_config.c
 */

void WffTransConfigPanel(struct controlbox *b);

/*
 * Public type definitions used by/in:
 * windows/window.c:{WinMain,WndProc}()
 */

typedef enum WffTransOp {
	WFF_TRANS_OP_FOCUS_KILL		= 1,
	WFF_TRANS_OP_FOCUS_SET		= 2,
} WffTransOp;

/*
 * Public subroutine prototypes used by/in:
 * windows/window.c:{WinMain,WndProc}()
 */

void WffTransOperation(WffTransOp op, Conf *conf, HWND hwnd);

#endif // !PUTTY_WINFRIP_FEATURE_TRANS_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
