/*
 * winfrip_feature_trans.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_FEATURE_TRANS_H
#define PUTTY_WINFRIP_FEATURE_TRANS_H

/*
 * Public subroutine prototypes used by/in:
 * winfrip_putty_config.c
 */

void winfripp_trans_config_panel(struct controlbox *b);

/*
 * Public type definitions used by/in:
 * windows/window.c:{WinMain,WndProc}()
 */

typedef enum WinFripTransOp {
	WINFRIP_TRANS_OP_FOCUS_KILL		= 1,
	WINFRIP_TRANS_OP_FOCUS_SET		= 2,
} WinFripTransOp;

/*
 * Public subroutine prototypes used by/in:
 * windows/window.c:{WinMain,WndProc}()
 */

void winfrip_trans_op(WinFripTransOp op, Conf *conf, HWND hwnd);

#endif // !PUTTY_WINFRIP_FEATURE_TRANS_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
