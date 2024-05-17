/*
 * winfrip_subr_putty_dialog.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_SUBR_PUTTY_DIALOG_H
#define PUTTY_WINFRIP_SUBR_PUTTY_DIALOG_H

/*
 * Public subroutine prototypes private to windows/dialog.c
 */

void WfPuttyDialogResizeControls(struct controlbox *ctrlbox, struct winctrls *ctrltrees, HWND hwnd, HWND hwnd_treeview, char *path, size_t which_tree, int ypos_initial);

#endif // !PUTTY_WINFRIP_SUBR_PUTTY_DIALOG_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
