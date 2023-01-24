/*
 * winfrip_feature_storage_host_keys.c - pointless frippery & tremendous amounts of bloat
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
#include "PuTTie/winfrip_feature_storage_host_keys.h"
#include "PuTTie/winfrip_storage.h"

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

#define WFFSP_CONFIG_GET_NITEM(ctx, dir, dlg, nitem) ({		\
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;	\
								\
	nitem = dlg_listbox_getid(				\
		ctx->listbox[dir], dlg,				\
		dlg_listbox_index(ctx->listbox[dir], dlg));	\
	if ((nitem < 0)						\
	||  ((size_t)nitem >= ctx->itemc[dir]))			\
	{							\
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);	\
	}							\
								\
	status;							\
})

/*
 * Private subroutine prototypes
 */

static WfrStatus	WffspClearHostKeys(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus	WffspDeleteHostKey(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus	WffspExportHostKey(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus	WffspRenameHostKey(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus	WffspRefreshHostKeys(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus	WffspSelectHostKey(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);

static void		WffspConfigHostKeysHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static WfrStatus	WffspConfigUpdateHostKeys(WffspConfigContext *ctx, WffspConfigDirection dir, dlgparam *dlg, bool update_listbox);

/*
 * Private subroutines
 */

static WfrStatus
WffspClearHostKeys(
	dlgcontrol *		ctrl,
	dlgparam *		dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend		backend, backend_from, backend_to;
	const char *		backend_name;
	bool			confirmfl;
	WffspConfigDirection	dir;
	LPSTR			lpText = NULL;
	WfrStatus		status;


	dir = ((ctrl == ctx->button_clear[WFFSP_CDIR_FROM]) ? WFFSP_CDIR_FROM : WFFSP_CDIR_TO);
	backend = WFFSP_CONFIG_GET_BACKEND(ctx, dir, dlg);
	backend_from = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_FROM, dlg);
	backend_to = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_TO, dlg);
	status = WfsGetBackendName(backend, &backend_name);

	if (WFR_STATUS_SUCCESS(status)
	&&  WFR_STATUS_SUCCESS(status = WfrSnDuprintf(
				&lpText, NULL, "Clear all host keys in the %s backend?", backend_name)))
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
			if (WFR_STATUS_SUCCESS(status = WfsClearHostKeys(backend, true))) {
				if (backend_from == backend_to) {
					if (WFR_STATUS_SUCCESS(status = WffspConfigUpdateHostKeys(ctx, WFFSP_CDIR_FROM, dlg, true))
					&&  WFR_STATUS_SUCCESS(status = WffspConfigUpdateHostKeys(ctx, WFFSP_CDIR_TO, dlg, true)))
					{
						status = WFR_STATUS_CONDITION_SUCCESS;
					}
				} else {
					status = WffspConfigUpdateHostKeys(ctx, dir, dlg, true);
				}
			}
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "clearing host keys");

	return status;
}

static WfrStatus
WffspDeleteHostKey(
	dlgcontrol *		ctrl,
	dlgparam *		dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend		backend, backend_from, backend_to;
	WffspConfigDirection	dir;
	char *			key_name = NULL;
	int			nhost_key;
	WfrStatus		status;


	dir = ((ctrl == ctx->button_delete[WFFSP_CDIR_FROM]) ? WFFSP_CDIR_FROM : WFFSP_CDIR_TO);
	backend = WFFSP_CONFIG_GET_BACKEND(ctx, dir, dlg);
	backend_from = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_FROM, dlg);
	backend_to = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_TO, dlg);

	if (WFR_STATUS_SUCCESS(status = WFFSP_CONFIG_GET_NITEM(ctx, dir, dlg, nhost_key))) {
		key_name = ctx->itemv[dir][nhost_key];
		status = WfsDeleteHostKey(backend, true, key_name);

		if (backend_from == backend_to) {
			if (WFR_STATUS_SUCCESS(status = WffspConfigUpdateHostKeys(ctx, WFFSP_CDIR_FROM, dlg, true))
			&&  WFR_STATUS_SUCCESS(status = WffspConfigUpdateHostKeys(ctx, WFFSP_CDIR_TO, dlg, true)))
			{
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		} else {
			status = WffspConfigUpdateHostKeys(ctx, dir, dlg, true);
		}
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "deleting host key %s", key_name);

	return status;
}

static WfrStatus
WffspExportHostKey(
	dlgcontrol *		ctrl,
	dlgparam *		dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend	backend_from, backend_to;
	char *		key_name;
	bool		movefl;
	int		nhost_key;
	WfrStatus	status;


	backend_from = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_FROM, dlg);
	backend_to = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_TO, dlg);

	if (backend_from != backend_to) {
		movefl = ((ctrl == ctx->button_copy) ? false : true);

		status = WFR_STATUS_CONDITION_SUCCESS;
		for (int index = 0; (size_t)index < ctx->itemc[WFFSP_CDIR_FROM]; index++) {
			if (dlg_listbox_issel(ctx->listbox[WFFSP_CDIR_FROM], dlg, index)) {
				nhost_key = dlg_listbox_getid(ctx->listbox[WFFSP_CDIR_FROM], dlg, index);
				key_name = ctx->itemv[WFFSP_CDIR_FROM][nhost_key];

				status = WfsExportHostKey(backend_from, backend_to, movefl, key_name);
				if (WFR_STATUS_FAILURE(status)) {
					WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "exporting host key %s", key_name);
					status = WFR_STATUS_CONDITION_SUCCESS;
				}
			}
		}

		if (WFR_STATUS_SUCCESS(status)) {
			if (movefl) {
				status = WffspConfigUpdateHostKeys(ctx, WFFSP_CDIR_FROM, dlg, true);
			}
			if (WFR_STATUS_SUCCESS(status)) {
				status = WffspConfigUpdateHostKeys(ctx, WFFSP_CDIR_TO, dlg, true);
			}
		}
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "exporting host keys");

	return status;
}

static WfrStatus
WffspRenameHostKey(
	dlgcontrol *		ctrl,
	dlgparam *		dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend		backend, backend_from, backend_to;
	WffspConfigDirection	dir;
	char *			key_name = NULL, *key_name_new = NULL;
	int			nhost_key;
	WfrStatus		status;


	dir = ((ctrl == ctx->button_rename[WFFSP_CDIR_FROM]) ? WFFSP_CDIR_FROM : WFFSP_CDIR_TO);
	backend = WFFSP_CONFIG_GET_BACKEND(ctx, dir, dlg);
	backend_from = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_FROM, dlg);
	backend_to = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_TO, dlg);
	key_name_new = dlg_editbox_get(ctx->editbox[dir], dlg);

	if (WFR_STATUS_SUCCESS(status = WFFSP_CONFIG_GET_NITEM(ctx, dir, dlg, nhost_key))) {
		key_name = ctx->itemv[dir][nhost_key];

		status = WfsRenameHostKey(backend, true, key_name, key_name_new);
		if (WFR_STATUS_FAILURE(status)) {
			sfree(key_name_new);
		} else {
			if (backend_from == backend_to) {
				if (WFR_STATUS_SUCCESS(status = WffspConfigUpdateHostKeys(ctx, WFFSP_CDIR_FROM, dlg, true))
				&&  WFR_STATUS_SUCCESS(status = WffspConfigUpdateHostKeys(ctx, WFFSP_CDIR_TO, dlg, true)))
				{
					status = WFR_STATUS_CONDITION_SUCCESS;
				}
			} else {
				status = WffspConfigUpdateHostKeys(ctx, dir, dlg, true);
			}
		}
	} else {
		sfree(key_name_new);
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "renaming host key %s", key_name);

	return status;
}

static WfrStatus
WffspRefreshHostKeys(
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

		status = WffspConfigUpdateHostKeys(ctx, dir, dlg, true);
		dlg_update_done(ctrl, dlg);
	} else if (ctrl == ctx->listbox[WFFSP_CDIR_FROM]) {
		status = WffspConfigUpdateHostKeys(ctx, WFFSP_CDIR_FROM, dlg, true);
	} else if (ctrl == ctx->listbox[WFFSP_CDIR_TO]) {
		status = WffspConfigUpdateHostKeys(ctx, WFFSP_CDIR_TO, dlg, true);
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "refreshing host key list");

	return status;
}

static WfrStatus
WffspSelectHostKey(
	dlgcontrol *		ctrl,
	dlgparam *		dlg,
	WffspConfigContext *	ctx
	)
{
	WffspConfigDirection	dir;
	int			nhost_key;
	WfrStatus		status;


	if (ctrl == ctx->droplist[WFFSP_CDIR_FROM]) {
		status = WffspConfigUpdateHostKeys(ctx, WFFSP_CDIR_FROM, dlg, true);
	} else if (ctrl == ctx->droplist[WFFSP_CDIR_TO]) {
		status = WffspConfigUpdateHostKeys(ctx, WFFSP_CDIR_TO, dlg, true);
	} else if ((ctrl == ctx->listbox[WFFSP_CDIR_FROM])
			|| (ctrl == ctx->listbox[WFFSP_CDIR_TO]))
	{
		dir = ((ctrl == ctx->listbox[WFFSP_CDIR_FROM]) ? WFFSP_CDIR_FROM : WFFSP_CDIR_TO);

		nhost_key = dlg_listbox_getid(ctrl, dlg, dlg_listbox_index(ctrl, dlg));
		if (nhost_key >= 0) {
			if (WFR_STATUS_SUCCESS(status = WFFSP_CONFIG_GET_NITEM(ctx, dir, dlg, nhost_key))) {
				dlg_editbox_set(ctx->editbox[dir], dlg, ctx->itemv[dir][nhost_key]);
			}
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "selecting host key");

	return status;
}


static void
WffspConfigHostKeysHandler(
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
			(void)WffspExportHostKey(ctrl, dlg, ctx);
		} else if ((ctrl == ctx->button_clear[WFFSP_CDIR_FROM])
				|| (ctrl == ctx->button_clear[WFFSP_CDIR_TO]))
		{
			(void)WffspClearHostKeys(ctrl, dlg, ctx);
		} else if ((ctrl == ctx->button_delete[WFFSP_CDIR_FROM])
				|| (ctrl == ctx->button_delete[WFFSP_CDIR_TO]))
		{
			(void)WffspDeleteHostKey(ctrl, dlg, ctx);
		} else if ((ctrl == ctx->button_rename[WFFSP_CDIR_FROM])
				|| (ctrl == ctx->button_rename[WFFSP_CDIR_TO]))
		{
			(void)WffspRenameHostKey(ctrl, dlg, ctx);
		}
		break;

	case EVENT_REFRESH:
		(void)WffspRefreshHostKeys(ctrl, dlg, ctx);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		(void)WffspSelectHostKey(ctrl, dlg, ctx);
		break;
	}
}

static WfrStatus
WffspConfigUpdateHostKeys(
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
	const char *	key_name;
	char *		key_name_;
	size_t		key_namec_new = 0;
	char **		key_namev_new = NULL;
	WfrStatus	status;


	ctrl = ctx->listbox[dir];
	backend = WFFSP_CONFIG_GET_BACKEND(ctx, dir, dlg);
	donefl = false;

	if (WFR_STATUS_SUCCESS(status = WfsEnumerateHostKeys(
				backend, false, true, NULL, NULL, &enum_state)))
	{
		do {
			status = WfsEnumerateHostKeys(
					backend, false, false,
					&donefl, &key_name, enum_state);

			if (WFR_STATUS_SUCCESS(status) && !donefl) {
				if (!(key_name_ = snewn(strlen(key_name) + 1, char))) {
					status = WFR_STATUS_FROM_ERRNO();
				} else if (WFR_STATUS_FAILURE(status = WFR_SRESIZE_IF_NEQ_SIZE(
						key_namev_new, key_namec_new,
						key_namec_new + 1, char *)))
				{
					sfree(key_name_);
				} else {
					strcpy(key_name_, key_name);
					key_namev_new[key_namec_new - 1] = key_name_;
				}
			}
		} while (WFR_STATUS_SUCCESS(status) && !donefl);

		if (WFR_STATUS_SUCCESS(status)) {
			for (size_t nhost_key = 0; nhost_key < ctx->itemc[dir]; nhost_key++) {
				WFR_SFREE_IF_NOTNULL(ctx->itemv[dir][nhost_key]);
			}
			WFR_SFREE_IF_NOTNULL(ctx->itemv[dir]);

			ctx->itemc[dir] = key_namec_new;
			ctx->itemv[dir] = key_namev_new;

			if (update_listbox) {
				dlg_update_start(ctrl, dlg);
				dlg_listbox_clear(ctrl, dlg);

				for (size_t nhost_key = 0; nhost_key < key_namec_new; nhost_key++) {
					dlg_listbox_addwithid(ctrl, dlg, key_namev_new[nhost_key], nhost_key);
				}

				dlg_update_done(ctrl, dlg);
			}
		} else {
			if (key_namev_new) {
				for (size_t nhost_key = 0; nhost_key < key_namec_new; nhost_key++) {
					WFR_SFREE_IF_NOTNULL(key_namev_new[nhost_key]);
				}
				sfree(key_namev_new);
			}
		}

		sfree(enum_state);
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "updating host key list");

	return status;
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
WffsHostKeysConfigPanel(
	struct controlbox *	b
	)
{
	WffspConfigContext *	ctx;
	struct controlset *	s;


	/*
	 * The Frippery: host keys panel.
	 */

	ctrl_settitle(b, "Frippery/Host keys", "Configure pointless frippery: host keys");

	ctx = snew(WffspConfigContext);
	WFFSP_CONFIG_CONTEXT_INIT(*ctx);

	s = ctrl_getset(b, "Frippery/Host keys", "frip_host_keys_from", "From:");
	ctrl_columns(s, 2, 70, 30);
	ctx->editbox[WFFSP_CDIR_FROM] = ctrl_editbox(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigHostKeysHandler, P(ctx), P(NULL));
	ctx->editbox[WFFSP_CDIR_FROM]->column = 0;
	ctx->droplist[WFFSP_CDIR_FROM] = ctrl_droplist(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigHostKeysHandler, P(ctx));
	ctx->droplist[WFFSP_CDIR_FROM]->column = 1;
	/* Reset columns so that the buttons are alongside the list, rather
	 * than alongside that edit box. */
	ctrl_columns(s, 1, 100);
	ctrl_columns(s, 2, 75, 25);
	ctx->listbox[WFFSP_CDIR_FROM] = ctrl_listbox(s, NULL, NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostKeysHandler, P(ctx));
	ctx->listbox[WFFSP_CDIR_FROM]->column = 0;
	ctx->listbox[WFFSP_CDIR_FROM]->listbox.height = 6;
	ctx->listbox[WFFSP_CDIR_FROM]->listbox.multisel = 1;
	ctx->button_clear[WFFSP_CDIR_FROM] = ctrl_pushbutton(s, "Clear...", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostKeysHandler, P(ctx));
	ctx->button_clear[WFFSP_CDIR_FROM]->column = 1;
	ctx->button_delete[WFFSP_CDIR_FROM] = ctrl_pushbutton(s, "Delete", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostKeysHandler, P(ctx));
	ctx->button_delete[WFFSP_CDIR_FROM]->column = 1;
	ctx->button_rename[WFFSP_CDIR_FROM] = ctrl_pushbutton(s, "Rename", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostKeysHandler, P(ctx));
	ctx->button_rename[WFFSP_CDIR_FROM]->column = 1;
	ctrl_columns(s, 1, 100);

	s = ctrl_getset(b, "Frippery/Host keys", "frip_host_keys_export", "Export from/to:");
	ctrl_columns(s, 2, 50, 50);
	ctx->button_copy = ctrl_pushbutton(s, "Copy", 'p', WFP_HELP_CTX, WffspConfigHostKeysHandler, P(ctx));
	ctx->button_copy->column = 0;
	ctx->button_move = ctrl_pushbutton(s, "Move", 'm', WFP_HELP_CTX, WffspConfigHostKeysHandler, P(ctx));
	ctx->button_move->column = 1;
	ctrl_columns(s, 1, 100);

	s = ctrl_getset(b, "Frippery/Host keys", "frip_host_keys_to", "To:");
	ctrl_columns(s, 2, 70, 30);
	ctx->editbox[WFFSP_CDIR_TO] = ctrl_editbox(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigHostKeysHandler, P(ctx), P(NULL));
	ctx->editbox[WFFSP_CDIR_TO]->column = 0;
	ctx->droplist[WFFSP_CDIR_TO] = ctrl_droplist(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigHostKeysHandler, P(ctx));
	ctx->droplist[WFFSP_CDIR_TO]->column = 1;
	/* Reset columns so that the buttons are alongside the list, rather
	 * than alongside that edit box. */
	ctrl_columns(s, 1, 100);
	ctrl_columns(s, 2, 75, 25);
	ctx->listbox[WFFSP_CDIR_TO] = ctrl_listbox(s, NULL, NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostKeysHandler, P(ctx));
	ctx->listbox[WFFSP_CDIR_TO]->column = 0;
	ctx->listbox[WFFSP_CDIR_TO]->listbox.height = 6;
	ctx->listbox[WFFSP_CDIR_TO]->listbox.multisel = 1;
	ctx->button_clear[WFFSP_CDIR_TO] = ctrl_pushbutton(s, "Clear...", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostKeysHandler, P(ctx));
	ctx->button_clear[WFFSP_CDIR_TO]->column = 1;
	ctx->button_delete[WFFSP_CDIR_TO] = ctrl_pushbutton(s, "Delete", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostKeysHandler, P(ctx));
	ctx->button_delete[WFFSP_CDIR_TO]->column = 1;
	ctx->button_rename[WFFSP_CDIR_TO] = ctrl_pushbutton(s, "Rename", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigHostKeysHandler, P(ctx));
	ctx->button_rename[WFFSP_CDIR_TO]->column = 1;
	ctrl_columns(s, 1, 100);
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
