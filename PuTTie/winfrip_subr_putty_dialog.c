/*
 * winfrip_subr_putty_dialog.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#pragma GCC diagnostic pop
#include "dialog.h"

#include "PuTTie/winfrip_subr_putty_dialog.h"

/*
 * Private constants
 */

#define WINFRIPP_DIALOG_YMAGIC1		100
#define WINFRIPP_DIALOG_YMAGIC2		14
#define WINFRIPP_DIALOG_YMAGIC3		27
#define WINFRIPP_DIALOG_YMAGIC4		13
#define WINFRIPP_DIALOG_YMAGIC5		7
#define WINFRIPP_DIALOG_YMAGIC6		85

/*
 * Private subroutine prototypes
 */

static void WfPuttyDialogpGetControlRect(HWND hwnd, int id, HWND *phwnd_ctrl, RECT *prect_ctrl);
static void WfPuttyDialogpMoveSingleControl(HWND hwnd, int id, int left_offset, int top);
static void WfPuttyDialogpResizeSingleControl(HWND hwnd, int id, int left_offset, int bottom);
static void WfPuttyDialogpResizeControl(dlgcontrol *ctrl, HWND hwnd, struct winctrl *thisc_ctrl, int *pypos);
static bool WfPuttyDialogpResizeListbox(LONG bottom_extra, dlgcontrol *ctrl, struct winctrls *ctrltrees, HWND hwnd, struct controlset *s, struct winctrl *thisc_ctrl, size_t which_tree, int *pypos);

/*
 * Private subroutines
 */

static void
WfPuttyDialogpGetControlRect(
	HWND	hwnd,
	int	id,
	HWND *	phwnd_ctrl,
	RECT *	prect_ctrl
)
{
	HWND	hwnd_ctrl;
	RECT	rect_ctrl;


	hwnd_ctrl = GetDlgItem(hwnd, id);
	(void)GetWindowRect(hwnd_ctrl, &rect_ctrl);
	(void)MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rect_ctrl, 2);

	*phwnd_ctrl = hwnd_ctrl;
	*prect_ctrl = rect_ctrl;
}

static void
WfPuttyDialogpMoveSingleControl(
	HWND	hwnd,
	int	id,
	int	left_offset,
	int	top
)
{
	HWND	hwnd_ctrl;
	RECT	rect_ctrl;


	WfPuttyDialogpGetControlRect(hwnd, id, &hwnd_ctrl, &rect_ctrl);
	(void)SetWindowPos(
		hwnd_ctrl, NULL,
		rect_ctrl.left + left_offset,
		top,
		0, 0,
		SWP_NOSIZE | SWP_NOZORDER);
}

static void
WfPuttyDialogpResizeSingleControl(
	HWND	hwnd,
	int	id,
	int	left_offset,
	int	bottom
)
{
	HWND	hwnd_ctrl;
	RECT	rect_ctrl;


	WfPuttyDialogpGetControlRect(hwnd, id, &hwnd_ctrl, &rect_ctrl);
	(void)SetWindowPos(
		hwnd_ctrl, NULL,
		0, 0,
		(rect_ctrl.right - rect_ctrl.left) + left_offset,
		bottom,
		SWP_NOMOVE | SWP_NOZORDER);
}

static void
WfPuttyDialogpResizeControl(
	dlgcontrol *		ctrl,
	HWND			hwnd,
	struct winctrl *	thisc_ctrl,
	int *			pypos
)
{
	HWND	hwnd_ctrl;
	RECT	rect_ctrl;


	WfPuttyDialogpGetControlRect(
		hwnd, thisc_ctrl->base_id,
		&hwnd_ctrl, &rect_ctrl);
	WfPuttyDialogpMoveSingleControl(
		hwnd, thisc_ctrl->base_id, 0,
		*pypos + WINFRIPP_DIALOG_YMAGIC2);
	*pypos += (rect_ctrl.bottom - rect_ctrl.top) + WINFRIPP_DIALOG_YMAGIC2;

	if (ctrl->type == CTRL_RADIO) {
		for (int nbutton = 0;
			 nbutton < ctrl->radio.nbuttons;
			 nbutton++)
		{
			WfPuttyDialogpMoveSingleControl(
				hwnd, thisc_ctrl->base_id + 1 + nbutton, 0, *pypos);
		}
	}
}

static bool
WfPuttyDialogpResizeListbox(
	LONG			bottom_extra,
	dlgcontrol *		ctrl,
	struct winctrls *	ctrltrees,
	HWND			hwnd,
	struct controlset *	s,
	struct winctrl *	thisc_ctrl,
	size_t			which_tree,
	int *			pypos
)
{
	bool	foundfl = false;
	LONG	height_new;
	HWND	hwnd_ctrl;
	RECT	rect_ctrl, rect_ctrl_child;


	WfPuttyDialogpGetControlRect(
		hwnd, thisc_ctrl->base_id,
		&hwnd_ctrl, &rect_ctrl);

	if (!((ctrl->type == CTRL_LISTBOX)
	&&  (ctrl->listbox.height == -1)))
	{
		return false;
	}

	WfPuttyDialogpGetControlRect(
		hwnd, thisc_ctrl->base_id + 1,
		&hwnd_ctrl, &rect_ctrl);

	if ((bottom_extra % WINFRIPP_DIALOG_YMAGIC4) > 0) {
		bottom_extra -= (bottom_extra % WINFRIPP_DIALOG_YMAGIC4);
	}
	rect_ctrl.bottom += bottom_extra;

	WfPuttyDialogpResizeSingleControl(
		hwnd, thisc_ctrl->base_id + 1, 0,
		rect_ctrl.bottom - rect_ctrl.top);
	foundfl = true;

	if ((s->boxname != NULL)
	&&  (s->boxname[0] != '\0'))
	{
		for (size_t n = 0; n < s->ncontrols; n++) {
			if ((s->ctrls[n]->type != CTRL_COLUMNS)
			&&  (s->ctrls[n]->type != CTRL_TABDELAY))
			{
				if ((thisc_ctrl = winctrl_findbyctrl(
						&ctrltrees[which_tree],
						s->ctrls[n])) != NULL)
				{
					WfPuttyDialogpGetControlRect(
						hwnd, thisc_ctrl->base_id - 1,
						&hwnd_ctrl, &rect_ctrl_child);

					height_new = (rect_ctrl.bottom + WINFRIPP_DIALOG_YMAGIC5)
						   - rect_ctrl_child.top;
					*pypos = rect_ctrl_child.top + height_new;

					WfPuttyDialogpResizeSingleControl(
						hwnd, thisc_ctrl->base_id - 1, 0,
						height_new);
				}
				break;
			}
		}
	}

	return foundfl;
}

/*
 * Public subroutine prototypes private to windows/dialog.c
 */

void
WfPuttyDialogResizeControls(
	struct controlbox *	ctrlbox,
	struct winctrls *	ctrltrees,
	HWND			hwnd,
	HWND			hwnd_treeview,
	char *			path,
	size_t			which_tree,
	int			ypos_initial
)
{
	LONG			bottom_extra;
	dlgcontrol *		ctrl;
	bool			foundfl = false;
	int			index;
	RECT			rect_parent;
	RECT			rect_treeview;
	struct controlset *	s;
	struct winctrl *	thisc_ctrl = NULL;
	int			ypos = 0;


	/*
	 * Derive the amount of additional space available to dynamic listbox
	 * controls (e.g. height==-1) from the bottom coordinate of the client
	 * area minus WINFRIPP_DIALOG_YMAGIC1 minus the bottom coordinate of
	 * the bottommost control.
	 */

	(void)GetClientRect(hwnd, &rect_parent);
	if ((rect_parent.bottom - WINFRIPP_DIALOG_YMAGIC1) > ypos_initial) {
		bottom_extra = (rect_parent.bottom - WINFRIPP_DIALOG_YMAGIC1)
			     - ypos_initial;
	} else {
		bottom_extra = 0;
	}

	/*
	 * Iterate over all panels.
	 */

	for (index = -1;
		 (index = ctrl_find_path(
			ctrlbox, path, index)) >= 0 ;)
	{
		s = ctrlbox->ctrlsets[index];

		/*
		 * If a dynamic listbox control (e.g. height==-1) was previously
		 * encountered and had its size adjusted and if it is enclosed by
		 * a box control, adjust its size as well.
		 *
		 * Being as PuTTY provides no interface to access said box control,
		 * derive its id from the first non-virtual (e.g. CTRL_COLUMNS or
		 * CTRL_TABDELAY) control in the current panel minus 1, as per
		 * winctrl_layout().
		 */

		if (foundfl
		&&  (s->boxname != NULL)
		&&  (s->boxname[0] != '\0'))
		{
			for (size_t n = 0; n < s->ncontrols; n++) {
				if ((s->ctrls[n]->type != CTRL_COLUMNS)
				&&	(s->ctrls[n]->type != CTRL_TABDELAY))
				{
					if ((thisc_ctrl = winctrl_findbyctrl(
							&ctrltrees[which_tree],
							s->ctrls[n])) != NULL)
					{
						WfPuttyDialogpMoveSingleControl(
							hwnd, thisc_ctrl->base_id - 1, 0, ypos);
					}
					break;
				}
			}
		}

		/*
		 * Iterate over the panel's controls until a dynamic listbox
		 * control (e.g. height==-1) is encountered, which then has
		 * its size adjusted. All the controls following a dynamic
		 * listbox will have their size adjusted also.
		 */

		for (size_t n = 0; n < s->ncontrols; n++) {
			ctrl = s->ctrls[n];

			if ((ctrl->column == 0)
			&&  ((thisc_ctrl = winctrl_findbyctrl(
					&ctrltrees[which_tree], ctrl)) != NULL))
			{
				if (!foundfl) {
					foundfl = WfPuttyDialogpResizeListbox(
						bottom_extra, ctrl, ctrltrees, hwnd,
						s, thisc_ctrl, which_tree, &ypos);
				} else if (foundfl) {
					WfPuttyDialogpResizeControl(
						ctrl, hwnd, thisc_ctrl, &ypos);
				}
			}
		}
	}

	/*
	 * Move About/Open/Cancel buttons row to the very bottom of
	 * the client area minus WINFRIPP_DIALOG_YMAGIC3.
	 */

	if ((index = ctrl_find_path(ctrlbox, "", -1)) >= 0) {
		s = ctrlbox->ctrlsets[index];

		for (size_t n = 0; n < s->ncontrols; n++) {
			if ((thisc_ctrl = winctrl_findbyctrl(
					&ctrltrees[1], s->ctrls[n])) != NULL)
			{
				WfPuttyDialogpMoveSingleControl(
					hwnd, thisc_ctrl->base_id,
					0, rect_parent.bottom - WINFRIPP_DIALOG_YMAGIC3);
				}
			}
	}

	/*
	 * Finally, resize the tree view control.
	 */

	if (ypos == 0) {
		RECT	rect_ctrl;
		(void)GetWindowRect(hwnd, &rect_ctrl);
		ypos = (rect_ctrl.bottom - rect_ctrl.top) - WINFRIPP_DIALOG_YMAGIC6;
	}
	if (ypos > 0) {
		(void)GetWindowRect(hwnd_treeview, &rect_treeview);
		(void)MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rect_treeview, 2);
		(void)SetWindowPos(
			hwnd_treeview, NULL,
			0, 0,
			(rect_treeview.right - rect_treeview.left),
			ypos,
			SWP_NOMOVE | SWP_NOZORDER);
	}
}
