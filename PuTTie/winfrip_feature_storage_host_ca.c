/*
 * winfrip_feature_storage_host_ca.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "dialog.h"
#include "windows/putty-rc.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_feature_storage_host_ca.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_host_ca.h"

/*
 * Private types
 */

typedef enum WffspConfigDirection {
	WFFSP_CDIR_FROM		= 0,
	WFFSP_CDIR_TO		= 1,
	WFFSP_CDIR_COUNT	= WFFSP_CDIR_TO + 1,
} WffspConfigDirection;

typedef struct WffspConfigContext {
	dlgcontrol *	droplist[WFFSP_CDIR_COUNT];
	dlgcontrol *	editbox[WFFSP_CDIR_COUNT];

	dlgcontrol *	listbox[WFFSP_CDIR_COUNT];

	dlgcontrol *	button_clear[WFFSP_CDIR_COUNT];
	dlgcontrol *	button_delete[WFFSP_CDIR_COUNT];
	dlgcontrol *	button_rename[WFFSP_CDIR_COUNT];

	dlgcontrol *	button_copy, *button_move;

	size_t		itemc[WFFSP_CDIR_COUNT];
	char **		itemv[WFFSP_CDIR_COUNT];
} WffspConfigContext;
#define WFFSP_CONFIG_CONTEXT_EMPTY {	\
	.droplist = {NULL, NULL},	\
	.editbox = {NULL, NULL},	\
	.listbox = {NULL, NULL},	\
					\
	.button_clear = {NULL, NULL},	\
	.button_delete = {NULL, NULL},	\
	.button_rename = {NULL, NULL},	\
					\
	.button_copy = NULL,		\
	.button_move = NULL,		\
					\
	.itemc = {0, 0},		\
	.itemv = {NULL, NULL},		\
}
#define WFFSP_CONFIG_CONTEXT_INIT(ctx)	\
	(ctx) = (WffspConfigContext)WFFSP_CONFIG_CONTEXT_EMPTY

/*
 * Private macros
 */

#define WFFSP_CONFIG_GET_BACKEND(ctx, dir, dlg)			\
	(WfsBackend)dlg_listbox_getid(				\
		(ctx)->droplist[dir], (dlg),			\
		dlg_listbox_index((ctx)->droplist[dir], (dlg)));

#define WFFSP_CONFIG_GET_NHOST_CA(ctx, dir, dlg, nhca) ({	\
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;	\
								\
	nhca = dlg_listbox_getid(				\
		ctx->listbox[dir], dlg,				\
		dlg_listbox_index(ctx->listbox[dir], dlg));	\
	if ((nhca < 0)						\
	||  ((size_t)nhca >= ctx->itemc[dir]))			\
	{							\
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);	\
	}							\
								\
	status;							\
})

/*
 * Private subroutine prototypes
 */

static WfrStatus	WffspClearHostCAs(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus	WffspDeleteHostCA(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus	WffspExportHostCA(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus	WffspRenameHostCA(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus	WffspRefreshHostCAs(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus	WffspSelectHostCA(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);

static void		WffspConfigHostCAsHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static WfrStatus	WffspConfigUpdateHostCAs(WffspConfigContext *ctx, WffspConfigDirection dir, dlgparam *dlg, bool update_listbox);

/*
 * Private subroutines
 */

static WfrStatus
WffspClearHostCAs(
	dlgcontrol *		ctrl,
	dlgparam *		dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend		backend, backend_from, backend_to;
	const char *		backend_name;
	bool			confirmfl;
	WffspConfigDirection	dir;
	LPSTR			lpText;
	WfrStatus		status;


	dir = ((ctrl == ctx->button_clear[WFFSP_CDIR_FROM]) ? WFFSP_CDIR_FROM : WFFSP_CDIR_TO);
	backend = WFFSP_CONFIG_GET_BACKEND(ctx, dir, dlg);
	backend_from = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_FROM, dlg);
	backend_to = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_TO, dlg);
	status = WfsGetBackendName(backend, &backend_name);

	if (WFR_STATUS_SUCCESS(status)
	&&  WFR_STATUS_SUCCESS(status = WfrSnDuprintf(
			&lpText, NULL, "Clear all host CAs in the %s backend?", backend_name)))
	{
		switch (MessageBox(
				NULL, lpText, "PuTTie",
				MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1))
		{
		case IDYES:
			confirmfl = true; break;
		case IDNO: default:
			confirmfl = false; break;
		}

		if (confirmfl) {
			if (WFR_STATUS_SUCCESS(status = WfsClearHostCAs(backend, true)))
			{
				if (backend_from == backend_to) {
					if (WFR_STATUS_SUCCESS(status = WffspConfigUpdateHostCAs(ctx, WFFSP_CDIR_FROM, dlg, true))
					&&  WFR_STATUS_SUCCESS(status = WffspConfigUpdateHostCAs(ctx, WFFSP_CDIR_TO, dlg, true)))
					{
						status = WFR_STATUS_CONDITION_SUCCESS;
					}
				} else {
					status = WffspConfigUpdateHostCAs(ctx, dir, dlg, true);
				}
			}
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "clearing host CAs");

	return status;
}

static WfrStatus
WffspDeleteHostCA(
	dlgcontrol *		ctrl,
	dlgparam *		dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend		backend, backend_from, backend_to;
	WffspConfigDirection	dir;
	int			nhca = -1;
	char *			name = NULL;
	WfrStatus		status, status_;


	dir = ((ctrl == ctx->button_delete[WFFSP_CDIR_FROM]) ? WFFSP_CDIR_FROM : WFFSP_CDIR_TO);
	backend = WFFSP_CONFIG_GET_BACKEND(ctx, dir, dlg);
	backend_from = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_FROM, dlg);
	backend_to = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_TO, dlg);

	if (WFR_STATUS_SUCCESS(status = WFFSP_CONFIG_GET_NHOST_CA(ctx, dir, dlg, nhca))) {
		name = ctx->itemv[dir][nhca];
		status = WfsDeleteHostCA(backend, true, name);

		if (backend_from == backend_to) {
			if (WFR_STATUS_SUCCESS(status_ = WffspConfigUpdateHostCAs(ctx, WFFSP_CDIR_FROM, dlg, true))
			&&  WFR_STATUS_SUCCESS(status_ = WffspConfigUpdateHostCAs(ctx, WFFSP_CDIR_TO, dlg, true)))
			{
				status_ = WFR_STATUS_CONDITION_SUCCESS;
			}
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status_, "deleting host CA %s", name);
		} else {
			status_ = WffspConfigUpdateHostCAs(ctx, dir, dlg, true);
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status_, "deleting host CA %s", name);
		}
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "deleting host CA %s", name);

	return status;
}

static WfrStatus
WffspExportHostCA(
	dlgcontrol *		ctrl,
	dlgparam *		dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend	backend_from, backend_to;
	bool		movefl;
	int		nhca;
	char *		name;
	WfrStatus	status;


	backend_from = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_FROM, dlg);
	backend_to = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_TO, dlg);

	if (backend_from != backend_to) {
		movefl = ((ctrl == ctx->button_copy) ? false : true);

		status = WFR_STATUS_CONDITION_SUCCESS;
		for (int index = 0; (size_t)index < ctx->itemc[WFFSP_CDIR_FROM]; index++) {
			if (dlg_listbox_issel(ctx->listbox[WFFSP_CDIR_FROM], dlg, index)) {
				nhca = dlg_listbox_getid(ctx->listbox[WFFSP_CDIR_FROM], dlg, index);
				name = ctx->itemv[WFFSP_CDIR_FROM][nhca];

				status = WfsExportHostCA(backend_from, backend_to, movefl, name);
				if (WFR_STATUS_FAILURE(status)) {
					WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "exporting host CA %s", name);
					status = WFR_STATUS_CONDITION_SUCCESS;
				}
			}
		}

		if (WFR_STATUS_SUCCESS(status)) {
			if (movefl) {
				status = WffspConfigUpdateHostCAs(ctx, WFFSP_CDIR_FROM, dlg, true);
			}
			if (WFR_STATUS_SUCCESS(status)) {
				status = WffspConfigUpdateHostCAs(ctx, WFFSP_CDIR_TO, dlg, true);
			}
		}

		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "exporting host CAs");
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

static WfrStatus
WffspRenameHostCA(
	dlgcontrol *		ctrl,
	dlgparam *		dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend		backend, backend_from, backend_to;
	WffspConfigDirection	dir;
	int			nhca;
	char *			name = NULL, *name_new;
	WfrStatus		status;


	dir = ((ctrl == ctx->button_rename[WFFSP_CDIR_FROM]) ? WFFSP_CDIR_FROM : WFFSP_CDIR_TO);
	backend = WFFSP_CONFIG_GET_BACKEND(ctx, dir, dlg);
	backend_from = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_FROM, dlg);
	backend_to = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_TO, dlg);
	name_new = dlg_editbox_get(ctx->editbox[dir], dlg);

	if (WFR_STATUS_SUCCESS(status = WFFSP_CONFIG_GET_NHOST_CA(ctx, dir, dlg, nhca))) {
		name = ctx->itemv[dir][nhca];

		status = WfsRenameHostCA(backend, true, name, name_new);
		if (WFR_STATUS_FAILURE(status)) {
			WFR_FREE(name_new);
		} else {
			if (backend_from == backend_to) {
				if (WFR_STATUS_SUCCESS(status = WffspConfigUpdateHostCAs(ctx, WFFSP_CDIR_FROM, dlg, true))
				&&  WFR_STATUS_SUCCESS(status = WffspConfigUpdateHostCAs(ctx, WFFSP_CDIR_TO, dlg, true)))
				{
					status = WFR_STATUS_CONDITION_SUCCESS;
				}
			} else {
				status = WffspConfigUpdateHostCAs(ctx, dir, dlg, true);
			}
		}
	} else {
		WFR_FREE(name_new);
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "renaming host CA %s", name);

	return WFR_STATUS_CONDITION_SUCCESS;
}

static WfrStatus
WffspRefreshHostCAs(
	dlgcontrol *		ctrl,
	dlgparam *		dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend		backend;
	const char *		backend_name;
	WffspConfigDirection	dir;
	WfrStatus		status;


	if ((ctrl == ctx->droplist[WFFSP_CDIR_FROM])
	||  (ctrl == ctx->droplist[WFFSP_CDIR_TO]))
	{
		dir = ((ctrl == ctx->droplist[WFFSP_CDIR_FROM]) ? WFFSP_CDIR_FROM : WFFSP_CDIR_TO);

		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);

		backend = WFS_BACKEND_MIN;
		do {
			if (WFR_STATUS_SUCCESS(status = WfsGetBackendName(backend, &backend_name))) {
				dlg_listbox_addwithid(ctrl, dlg, backend_name, backend);
			}
		} while (WfsGetBackendNext(&backend));

		if (ctrl == ctx->droplist[WFFSP_CDIR_FROM]) {
			dlg_listbox_select(ctrl, dlg, WFS_BACKEND_REGISTRY);
		} else if (ctrl == ctx->droplist[WFFSP_CDIR_TO]) {
			dlg_listbox_select(ctrl, dlg, WFS_BACKEND_FILE);
		}

		status = WffspConfigUpdateHostCAs(ctx, dir, dlg, true);
		dlg_update_done(ctrl, dlg);
	} else if (ctrl == ctx->listbox[WFFSP_CDIR_FROM]) {
		status = WffspConfigUpdateHostCAs(ctx, WFFSP_CDIR_FROM, dlg, true);
	} else if (ctrl == ctx->listbox[WFFSP_CDIR_TO]) {
		status = WffspConfigUpdateHostCAs(ctx, WFFSP_CDIR_TO, dlg, true);
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "refreshing host CA list");

	return status;
}

static WfrStatus
WffspSelectHostCA(
	dlgcontrol *		ctrl,
	dlgparam *		dlg,
	WffspConfigContext *	ctx
	)
{
	WffspConfigDirection	dir;
	int			nhca;
	WfrStatus		status;


	if (ctrl == ctx->droplist[WFFSP_CDIR_FROM]) {
		status = WffspConfigUpdateHostCAs(ctx, WFFSP_CDIR_FROM, dlg, true);
	} else if (ctrl == ctx->droplist[WFFSP_CDIR_TO]) {
		status = WffspConfigUpdateHostCAs(ctx, WFFSP_CDIR_TO, dlg, true);
	} else if ((ctrl == ctx->listbox[WFFSP_CDIR_FROM])
			|| (ctrl == ctx->listbox[WFFSP_CDIR_TO]))
	{
		dir = ((ctrl == ctx->listbox[WFFSP_CDIR_FROM]) ? WFFSP_CDIR_FROM : WFFSP_CDIR_TO);

		nhca = dlg_listbox_getid(ctrl, dlg, dlg_listbox_index(ctrl, dlg));
		if (nhca >= 0) {
			if (WFR_STATUS_SUCCESS(status = WFFSP_CONFIG_GET_NHOST_CA(ctx, dir, dlg, nhca))) {
				dlg_editbox_set(ctx->editbox[dir], dlg, ctx->itemv[dir][nhca]);
			}
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "selecting host CA");

	return status;
}


static void
WffspConfigHostCAsHandler(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	void *		data,
	int		event
	)
{
	WffspConfigContext *	ctx = (WffspConfigContext *)ctrl->context.p;


	(void)data;
	switch (event) {
	case EVENT_ACTION:
		if ((ctrl == ctx->button_copy)
		||  (ctrl == ctx->button_move))
		{
			(void)WffspExportHostCA(ctrl, dlg, ctx);
		} else if ((ctrl == ctx->button_clear[WFFSP_CDIR_FROM])
				|| (ctrl == ctx->button_clear[WFFSP_CDIR_TO]))
		{
			(void)WffspClearHostCAs(ctrl, dlg, ctx);
		} else if ((ctrl == ctx->button_delete[WFFSP_CDIR_FROM])
				|| (ctrl == ctx->button_delete[WFFSP_CDIR_TO]))
		{
			(void)WffspDeleteHostCA(ctrl, dlg, ctx);
		} else if ((ctrl == ctx->button_rename[WFFSP_CDIR_FROM])
				|| (ctrl == ctx->button_rename[WFFSP_CDIR_TO]))
		{
			(void)WffspRenameHostCA(ctrl, dlg, ctx);
		}
		break;

	case EVENT_REFRESH:
		(void)WffspRefreshHostCAs(ctrl, dlg, ctx);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		(void)WffspSelectHostCA(ctrl, dlg, ctx);
		break;
	}
}

static WfrStatus
WffspConfigUpdateHostCAs(
	WffspConfigContext *	ctx,
	WffspConfigDirection	dir,
	dlgparam *		dlg,
	bool			update_listbox
	)
{
	WfsBackend	backend;
	dlgcontrol *	ctrl;
	bool		donefl;
	void *		enum_state;
	size_t		itemc_new = 0;
	char **		itemv_new = NULL;
	char *		name;
	WfrStatus	status;


	ctrl = ctx->listbox[dir];
	backend = WFFSP_CONFIG_GET_BACKEND(ctx, dir, dlg);
	donefl = false;

	if (WFR_STATUS_SUCCESS(status = WfsEnumerateHostCAs(
				backend, false, true, NULL, NULL, &enum_state)))
	{
		do {
			status = WfsEnumerateHostCAs(
				backend, false, false,
				&donefl, &name, enum_state);

			if (WFR_STATUS_SUCCESS(status) && !donefl) {
				if (WFR_STATUS_SUCCESS(status = WFR_RESIZE(
						itemv_new, itemc_new, itemc_new + 1, char *)))
				{
					itemv_new[itemc_new - 1] = name;
				}
			}
		} while (WFR_STATUS_SUCCESS(status) && !donefl);

		if (WFR_STATUS_SUCCESS(status)) {
			for (size_t nhca = 0; nhca < ctx->itemc[dir]; nhca++) {
				WFR_FREE_IF_NOTNULL(ctx->itemv[dir][nhca]);
			}
			WFR_FREE_IF_NOTNULL(ctx->itemv[dir]);

			ctx->itemc[dir] = itemc_new;
			ctx->itemv[dir] = itemv_new;

			if (update_listbox) {
				dlg_update_start(ctrl, dlg);
				dlg_listbox_clear(ctrl, dlg);

				for (size_t nhca = 0; nhca < itemc_new; nhca++) {
					dlg_listbox_addwithid(ctrl, dlg, itemv_new[nhca], nhca);
				}

				dlg_update_done(ctrl, dlg);
			}
		} else {
			if (itemv_new) {
				for (size_t nhca = 0; nhca < itemc_new; nhca++) {
					WFR_FREE_IF_NOTNULL(itemv_new[nhca]);
				}
				WFR_FREE(itemv_new);
			}
		}

		WFR_FREE(enum_state);
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "updating host CA list");

	return status;
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
WffsHostCAConfigPanel(
	struct controlbox *	b
	)
{
	WffspConfigContext *	ctx;
	struct controlset *	s;


	/*
	 * The Frippery: hcas panel.
	 */

	ctrl_settitle(b, "Frippery/Host CAs", "Configure pointless frippery: host CAs");


	ctx = WFR_NEW(WffspConfigContext);
	WFFSP_CONFIG_CONTEXT_INIT(*ctx);

	s = ctrl_getset(b, "Frippery/Host CAs", "frip_hcas_from", "From:");
	ctrl_columns(s, 2, 70, 30);
	ctx->editbox[WFFSP_CDIR_FROM] = ctrl_editbox(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigHostCAsHandler, P(ctx), P(NULL));
	ctx->editbox[WFFSP_CDIR_FROM]->column = 0;
	ctx->droplist[WFFSP_CDIR_FROM] = ctrl_droplist(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigHostCAsHandler, P(ctx));
	ctx->droplist[WFFSP_CDIR_FROM]->column = 1;
	/* Reset columns so that the buttons are alongside the list, rather
	 * than alongside that edit box. */
	ctrl_columns(s, 1, 100);
	ctrl_columns(s, 2, 75, 25);
	ctx->listbox[WFFSP_CDIR_FROM] = ctrl_listbox(s, NULL, NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostCAsHandler, P(ctx));
	ctx->listbox[WFFSP_CDIR_FROM]->column = 0;
	ctx->listbox[WFFSP_CDIR_FROM]->listbox.height = 6;
	ctx->listbox[WFFSP_CDIR_FROM]->listbox.multisel = 1;
	ctx->button_clear[WFFSP_CDIR_FROM] = ctrl_pushbutton(s, "Clear...", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostCAsHandler, P(ctx));
	ctx->button_clear[WFFSP_CDIR_FROM]->column = 1;
	ctx->button_delete[WFFSP_CDIR_FROM] = ctrl_pushbutton(s, "Delete", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostCAsHandler, P(ctx));
	ctx->button_delete[WFFSP_CDIR_FROM]->column = 1;
	ctx->button_rename[WFFSP_CDIR_FROM] = ctrl_pushbutton(s, "Rename", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostCAsHandler, P(ctx));
	ctx->button_rename[WFFSP_CDIR_FROM]->column = 1;
	ctrl_columns(s, 1, 100);

	s = ctrl_getset(b, "Frippery/Host CAs", "frip_hcas_export", "Export from/to:");
	ctrl_columns(s, 2, 50, 50);
	ctx->button_copy = ctrl_pushbutton(s, "Copy", 'p', WFP_HELP_CTX, WffspConfigHostCAsHandler, P(ctx));
	ctx->button_copy->column = 0;
	ctx->button_move = ctrl_pushbutton(s, "Move", 'm', WFP_HELP_CTX, WffspConfigHostCAsHandler, P(ctx));
	ctx->button_move->column = 1;
	ctrl_columns(s, 1, 100);

	s = ctrl_getset(b, "Frippery/Host CAs", "frip_hcas_to", "To:");
	ctrl_columns(s, 2, 70, 30);
	ctx->editbox[WFFSP_CDIR_TO] = ctrl_editbox(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigHostCAsHandler, P(ctx), P(NULL));
	ctx->editbox[WFFSP_CDIR_TO]->column = 0;
	ctx->droplist[WFFSP_CDIR_TO] = ctrl_droplist(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigHostCAsHandler, P(ctx));
	ctx->droplist[WFFSP_CDIR_TO]->column = 1;
	/* Reset columns so that the buttons are alongside the list, rather
	 * than alongside that edit box. */
	ctrl_columns(s, 1, 100);
	ctrl_columns(s, 2, 75, 25);
	ctx->listbox[WFFSP_CDIR_TO] = ctrl_listbox(s, NULL, NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostCAsHandler, P(ctx));
	ctx->listbox[WFFSP_CDIR_TO]->column = 0;
	ctx->listbox[WFFSP_CDIR_TO]->listbox.height = 6;
	ctx->listbox[WFFSP_CDIR_TO]->listbox.multisel = 1;
	ctx->button_clear[WFFSP_CDIR_TO] = ctrl_pushbutton(s, "Clear...", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostCAsHandler, P(ctx));
	ctx->button_clear[WFFSP_CDIR_TO]->column = 1;
	ctx->button_delete[WFFSP_CDIR_TO] = ctrl_pushbutton(s, "Delete", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostCAsHandler, P(ctx));
	ctx->button_delete[WFFSP_CDIR_TO]->column = 1;
	ctx->button_rename[WFFSP_CDIR_TO] = ctrl_pushbutton(s, "Rename", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostCAsHandler, P(ctx));
	ctx->button_rename[WFFSP_CDIR_TO]->column = 1;
	ctrl_columns(s, 1, 100);
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
