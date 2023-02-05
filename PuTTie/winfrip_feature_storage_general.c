/*
 * winfrip_feature_storage_general.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "dialog.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_feature_storage_general.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_feature_storage_priv.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_host_keys.h"
#include "PuTTie/winfrip_storage_jump_list.h"
#include "PuTTie/winfrip_storage_options.h"
#include "PuTTie/winfrip_storage_privkey_list.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"

/*
 * Private types
 */

typedef enum WffspConfigItem {
	WFFSP_ITEM_CONTAINER		= 0,
	WFFSP_ITEM_HOST_CAS		= 1,
	WFFSP_ITEM_HOST_KEYS		= 2,
	WFFSP_ITEM_SESSIONS		= 3,
	WFFSP_ITEM_JUMP_LIST		= 4,
	WFFSP_ITEM_PRIVKEY_LIST		= 5,
	WFFSP_ITEM_OPTIONS		= 6,

	WFFSP_ITEM_MAX			= WFFSP_ITEM_OPTIONS,
} WffspConfigItem;
#define WFFSP_ITEM_CONTAINER_NAME	"(Container)"
#define WFFSP_ITEM_HOST_CAS_NAME	"Host CAs"
#define WFFSP_ITEM_HOST_KEYS_NAME	"Host keys"
#define WFFSP_ITEM_SESSIONS_NAME	"Sessions"
#define WFFSP_ITEM_JUMP_LIST_NAME	"Jump list"
#define WFFSP_ITEM_PRIVKEY_LIST_NAME	"Pageant private key list"
#define WFFSP_ITEM_OPTIONS_NAME		"Global options"

typedef struct WffspConfigContext {
	dlgcontrol *	droplist_from;
	dlgcontrol *	listbox;
	dlgcontrol *	button_cleanup;
	dlgcontrol *	button_copy_all;
	dlgcontrol *	button_move_all;
	dlgcontrol *	droplist_to;
} WffspConfigContext;
#define WFFSP_CONFIG_CONTEXT_EMPTY {	\
	.droplist_from = NULL,		\
	.listbox = NULL,		\
	.button_cleanup = NULL,		\
	.button_copy_all = NULL,	\
	.button_move_all = NULL,	\
	.droplist_to = NULL,		\
}
#define WFFSP_CONFIG_CONTEXT_INIT(ctx)	\
	(ctx) = (WffspConfigContext)WFFSP_CONFIG_CONTEXT_EMPTY

/*
 * Private macros
 */

#define WFFSP_CONFIG_GET_BACKEND_FROM(ctx, dlg)		\
	(WfsBackend)dlg_listbox_getid(			\
			(ctx)->droplist_from, (dlg),	\
			dlg_listbox_index((ctx)->droplist_from, (dlg)));

#define WFFSP_CONFIG_GET_BACKEND_TO(ctx, dlg)		\
	(WfsBackend)dlg_listbox_getid(			\
			(ctx)->droplist_to, (dlg),	\
			dlg_listbox_index((ctx)->droplist_to, (dlg)));

/*
 * Private subroutine prototypes
 */

static void	WffspConfigGeneralHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static void	WffspConfigGeneralCleanupHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static void	WffspConfigGeneralMigrateHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);

/*
 * Private subroutines
 */

static void
WffspConfigGeneralHandler(
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
		if (ctrl == ctx->button_cleanup)
		{
			WffspConfigGeneralCleanupHandler(ctrl, dlg, data, event);
		}
		else if ((ctrl == ctx->button_copy_all)
		      || (ctrl == ctx->button_move_all))
		{
			WffspConfigGeneralMigrateHandler(ctrl, dlg, data, event);
		}
		break;

	case EVENT_REFRESH:
		if ((ctrl == ctx->droplist_from)
		||  (ctrl == ctx->droplist_to))
		{
			dlg_update_start(ctrl, dlg);
			(void)WffsRefreshBackends(ctrl, dlg, (ctrl == ctx->droplist_from));
			dlg_update_done(ctrl, dlg);
		} else if (ctrl == ctx->listbox) {
			dlg_update_start(ctrl, dlg);
			dlg_listbox_clear(ctrl, dlg);

			dlg_listbox_addwithid(ctrl, dlg, WFFSP_ITEM_CONTAINER_NAME, WFFSP_ITEM_CONTAINER);
			dlg_listbox_addwithid(ctrl, dlg, WFFSP_ITEM_HOST_CAS_NAME, WFFSP_ITEM_HOST_CAS);
			dlg_listbox_addwithid(ctrl, dlg, WFFSP_ITEM_HOST_KEYS_NAME, WFFSP_ITEM_HOST_KEYS);
			dlg_listbox_addwithid(ctrl, dlg, WFFSP_ITEM_SESSIONS_NAME, WFFSP_ITEM_SESSIONS);
			dlg_listbox_addwithid(ctrl, dlg, WFFSP_ITEM_JUMP_LIST_NAME, WFFSP_ITEM_JUMP_LIST);
			dlg_listbox_addwithid(ctrl, dlg, WFFSP_ITEM_PRIVKEY_LIST_NAME, WFFSP_ITEM_PRIVKEY_LIST);
			dlg_listbox_addwithid(ctrl, dlg, WFFSP_ITEM_OPTIONS_NAME, WFFSP_ITEM_OPTIONS);

			dlg_update_done(ctrl, dlg);
		}
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		break;
	}
}

static void
WffspConfigGeneralCleanupHandler(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	void *		data,
	int		event
	)
{
	WfsBackend		backend_from;
	const char *		backend_from_name;
	bool			confirmfl;
	WffspConfigContext *	ctx = (WffspConfigContext *)ctrl->context.p;
	bool			selectv[WFFSP_ITEM_MAX + 1];
	WfrStatus		status;


	(void)data;
	(void)event;
	backend_from = WFFSP_CONFIG_GET_BACKEND_FROM(ctx, dlg);
	selectv[WFFSP_ITEM_CONTAINER] = dlg_listbox_issel(ctx->listbox, dlg, WFFSP_ITEM_CONTAINER);
	selectv[WFFSP_ITEM_HOST_CAS] = dlg_listbox_issel(ctx->listbox, dlg, WFFSP_ITEM_HOST_CAS);
	selectv[WFFSP_ITEM_HOST_KEYS] = dlg_listbox_issel(ctx->listbox, dlg, WFFSP_ITEM_HOST_KEYS);
	selectv[WFFSP_ITEM_SESSIONS] = dlg_listbox_issel(ctx->listbox, dlg, WFFSP_ITEM_SESSIONS);
	selectv[WFFSP_ITEM_JUMP_LIST] = dlg_listbox_issel(ctx->listbox, dlg, WFFSP_ITEM_JUMP_LIST);
	selectv[WFFSP_ITEM_PRIVKEY_LIST] = dlg_listbox_issel(ctx->listbox, dlg, WFFSP_ITEM_PRIVKEY_LIST);
	selectv[WFFSP_ITEM_OPTIONS] = dlg_listbox_issel(ctx->listbox, dlg, WFFSP_ITEM_OPTIONS);

	if (!selectv[WFFSP_ITEM_CONTAINER]
	&&  !selectv[WFFSP_ITEM_HOST_CAS]
	&&  !selectv[WFFSP_ITEM_HOST_KEYS]
	&&  !selectv[WFFSP_ITEM_SESSIONS]
	&&  !selectv[WFFSP_ITEM_JUMP_LIST]
	&&  !selectv[WFFSP_ITEM_PRIVKEY_LIST]
	&&  !selectv[WFFSP_ITEM_OPTIONS])
	{
		return;
	} else if (WFR_STATUS_FAILURE(status = WfsGetBackendName(backend_from, &backend_from_name))) {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "getting backend name");
		return;
	}

	switch (WfrMessageBoxF(
			"PuTTie", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1,
			"Clean up the following items in %s backend?\n"
			"%s%s%s%s%s%s%s",
			backend_from_name,
			selectv[WFFSP_ITEM_CONTAINER] ? "(container)\n" : "",
			selectv[WFFSP_ITEM_HOST_CAS] ? "host CAs\n" : "",
			selectv[WFFSP_ITEM_HOST_KEYS] ? "host keys\n" : "",
			selectv[WFFSP_ITEM_SESSIONS] ? "sessions\n" : "",
			selectv[WFFSP_ITEM_JUMP_LIST] ? "jump list\n" : "",
			selectv[WFFSP_ITEM_PRIVKEY_LIST] ? "Pageant private key list\n" : "",
			selectv[WFFSP_ITEM_OPTIONS] ? "global options\n" : ""))
	{
	case IDYES:
		confirmfl = true; break;
	case IDNO: default:
		confirmfl = false; break;
	}

	if (confirmfl) {
		status = WFR_STATUS_CONDITION_SUCCESS;

		if (selectv[WFFSP_ITEM_HOST_CAS]) {
			status = WfsCleanupHostCAs(backend_from);
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "cleaning up host CAs");
		}

		if (selectv[WFFSP_ITEM_HOST_KEYS]) {
			status = WfsCleanupHostKeys(backend_from);
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "cleaning up host keys");
		}

		if (selectv[WFFSP_ITEM_SESSIONS]) {
			status = WfsCleanupSessions(backend_from);
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "cleaning up sessions");
		}

		if (selectv[WFFSP_ITEM_JUMP_LIST]) {
			status = WfsCleanupJumpList(backend_from);
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "cleaning up jump list");
		}

		if (selectv[WFFSP_ITEM_PRIVKEY_LIST]) {
			status = WfsCleanupPrivKeyList(backend_from);
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "cleaning up Pageant private key list");
		}

		if (selectv[WFFSP_ITEM_OPTIONS]) {
			status = WfsCleanupOptions(backend_from);
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "cleaning up global options");
		}

		if (selectv[WFFSP_ITEM_CONTAINER]) {
			status = WfsCleanupContainer(backend_from);
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "cleaning up container");
		}

		if (WFR_STATUS_SUCCESS(status)) {
			WfrMessageBoxF(
				"PuTTie", MB_ICONINFORMATION | MB_OK | MB_DEFBUTTON1,
				"Successfully cleaned up items in %s backend:\n"
				"%s%s%s%s%s%s",
				backend_from_name,
				selectv[WFFSP_ITEM_CONTAINER] ? "(container)\n" : "",
				selectv[WFFSP_ITEM_HOST_CAS] ? "host CAs\n" : "",
				selectv[WFFSP_ITEM_HOST_KEYS] ? "host keys\n" : "",
				selectv[WFFSP_ITEM_SESSIONS] ? "sessions\n" : "",
				selectv[WFFSP_ITEM_JUMP_LIST] ? "jump list\n" : "",
				selectv[WFFSP_ITEM_PRIVKEY_LIST] ? "Pageant private key list\n" : "",
				selectv[WFFSP_ITEM_OPTIONS] ? "global options" : "");
		}
	}
}

static void
WffspConfigGeneralMigrateHandler(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	void *		data,
	int		event
	)
{
	WfsBackend		backend_from, backend_to;
	const char *		backend_from_name, *backend_to_name;
	bool			confirmfl;
	WffspConfigContext *	ctx = (WffspConfigContext *)ctrl->context.p;
	void			(*error_fn_host_ca)(const char *, WfrStatus) =
				WFR_LAMBDA(void, (const char *name, WfrStatus status) {
					WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "migrating host CA %s", name);
				});
	void			(*error_fn_host_key)(const char *, WfrStatus) =
				WFR_LAMBDA(void, (const char *key_name, WfrStatus status) {
					WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "migrating host key %s", key_name);
				});
	void			(*error_fn_option)(const char *, WfrStatus) =
				WFR_LAMBDA(void, (const char *key, WfrStatus status) {
					WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "migrating option %s", key);
				});
	void			(*error_fn_session)(const char *, WfrStatus) =
				WFR_LAMBDA(void, (const char *sessionname, WfrStatus status) {
					WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "migrating session %s", sessionname);
				});
	bool			movefl;
	bool			selectv[WFFSP_ITEM_MAX + 1];
	WfrStatus		status;


	(void)data;
	(void)event;
	backend_from = WFFSP_CONFIG_GET_BACKEND_FROM(ctx, dlg);
	backend_to = WFFSP_CONFIG_GET_BACKEND_TO(ctx, dlg);
	movefl = ((ctrl == ctx->button_copy_all) ? false : true);
	selectv[WFFSP_ITEM_CONTAINER] = dlg_listbox_issel(ctx->listbox, dlg, WFFSP_ITEM_CONTAINER);
	selectv[WFFSP_ITEM_HOST_CAS] = dlg_listbox_issel(ctx->listbox, dlg, WFFSP_ITEM_HOST_CAS);
	selectv[WFFSP_ITEM_HOST_KEYS] = dlg_listbox_issel(ctx->listbox, dlg, WFFSP_ITEM_HOST_KEYS);
	selectv[WFFSP_ITEM_SESSIONS] = dlg_listbox_issel(ctx->listbox, dlg, WFFSP_ITEM_SESSIONS);
	selectv[WFFSP_ITEM_JUMP_LIST] = dlg_listbox_issel(ctx->listbox, dlg, WFFSP_ITEM_JUMP_LIST);
	selectv[WFFSP_ITEM_PRIVKEY_LIST] = dlg_listbox_issel(ctx->listbox, dlg, WFFSP_ITEM_PRIVKEY_LIST);
	selectv[WFFSP_ITEM_OPTIONS] = dlg_listbox_issel(ctx->listbox, dlg, WFFSP_ITEM_OPTIONS);

	if (backend_from == backend_to) {
		return;
	} else if (!selectv[WFFSP_ITEM_HOST_KEYS]
		&& !selectv[WFFSP_ITEM_HOST_CAS]
		&& !selectv[WFFSP_ITEM_SESSIONS]
		&& !selectv[WFFSP_ITEM_JUMP_LIST]
		&& !selectv[WFFSP_ITEM_PRIVKEY_LIST]
		&& !selectv[WFFSP_ITEM_OPTIONS])
	{
		return;
	} else if (WFR_STATUS_FAILURE(status = WfsGetBackendName(backend_from, &backend_from_name))
		|| WFR_STATUS_FAILURE(status = WfsGetBackendName(backend_to, &backend_to_name)))
	{
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "getting backend name");
		return;
	}

	switch (WfrMessageBoxF(
			"PuTTie", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1,
			"%s the following items from %s to %s backend?\n"
			"%s%s%s%s%s",
			movefl ? "Move" : "Copy",
			backend_from_name, backend_to_name,
			selectv[WFFSP_ITEM_HOST_KEYS] ? "host keys\n" : "",
			selectv[WFFSP_ITEM_HOST_CAS] ? "host CAs\n" : "",
			selectv[WFFSP_ITEM_SESSIONS] ? "sessions\n" : "",
			selectv[WFFSP_ITEM_JUMP_LIST] ? "jump list\n" : "",
			selectv[WFFSP_ITEM_PRIVKEY_LIST] ? "Pageant private key list\n" : "",
			selectv[WFFSP_ITEM_OPTIONS] ? "global options" : ""))
	{
	case IDYES:
		confirmfl = true; break;
	case IDNO: default:
		confirmfl = false; break;
	}

	if (confirmfl) {
		status = WFR_STATUS_CONDITION_SUCCESS;

		if (selectv[WFFSP_ITEM_HOST_CAS]) {
			if (WFR_STATUS_SUCCESS(status = WfsExportHostCAs(backend_from, backend_to, true, true, error_fn_host_ca))) {
				if (movefl) {
					status = WfsCleanupHostCAs(backend_from);
				}
			}
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "migrating host CAs");
		}

		if (selectv[WFFSP_ITEM_HOST_KEYS]) {
			if (WFR_STATUS_SUCCESS(status = WfsExportHostKeys(backend_from, backend_to, true, true, error_fn_host_key))) {
				if (movefl) {
					status = WfsCleanupHostKeys(backend_from);
				}
			}
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "migrating host keys");
		}

		if (selectv[WFFSP_ITEM_SESSIONS]) {
			if (WFR_STATUS_SUCCESS(status = WfsExportSessions(backend_from, backend_to, true, true, error_fn_session))) {
				if (movefl) {
					status = WfsCleanupSessions(backend_from);
				}
			}
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "migrating sessions");
		}

		if (selectv[WFFSP_ITEM_JUMP_LIST]) {
			if (WFR_STATUS_SUCCESS(status = WfsExportJumpList(backend_from, backend_to, movefl))) {
				if (movefl) {
					status = WfsCleanupJumpList(backend_from);
				}
			}
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "migrating jump list");
		}

		if (selectv[WFFSP_ITEM_PRIVKEY_LIST]) {
			if (WFR_STATUS_SUCCESS(status = WfsExportPrivKeyList(backend_from, backend_to, movefl))) {
				if (movefl) {
					status = WfsCleanupPrivKeyList(backend_from);
				}
			}
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "migrating Pageant private key list");
		}

		if (selectv[WFFSP_ITEM_OPTIONS]) {
			if (WFR_STATUS_SUCCESS(status = WfsExportOptions(backend_from, backend_to, true, true, error_fn_option))) {
				if (movefl) {
					status = WfsCleanupOptions(backend_from);
				}
			}
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "migrating global options");
		}

		if (movefl && WFR_STATUS_SUCCESS(status)) {
			status = WfsCleanupContainer(backend_from);
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "cleaning up container");
		}

		if (WFR_STATUS_SUCCESS(status)) {
			WfrMessageBoxF(
				"PuTTie", MB_ICONQUESTION | MB_OK | MB_DEFBUTTON1,
				"Successfully %s the following items from %s to %s backend:\n"
				"%s%s%s%s%s",
				movefl ? "moved" : "copied",
				backend_from_name, backend_to_name,
				selectv[WFFSP_ITEM_HOST_KEYS] ? "host keys\n" : "",
				selectv[WFFSP_ITEM_HOST_CAS] ? "host CAs\n" : "",
				selectv[WFFSP_ITEM_SESSIONS] ? "sessions\n" : "",
				selectv[WFFSP_ITEM_JUMP_LIST] ? "jump list\n" : "",
				selectv[WFFSP_ITEM_PRIVKEY_LIST] ? "Pageant private key list\n" : "",
				selectv[WFFSP_ITEM_OPTIONS] ? "global options\n" : "");
		}
	}
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
WffsGeneralConfigPanel(
	struct controlbox *	b
	)
{
	WffspConfigContext *	ctx;
	struct controlset *	s;


	/*
	 * The Frippery: sessions panel.
	 */

	ctrl_settitle(b, "Frippery/Storage", "Configure pointless frippery: storage");


	ctx = WFR_NEW(WffspConfigContext);
	WFFSP_CONFIG_CONTEXT_INIT(*ctx);

	s = ctrl_getset(b, "Frippery/Storage", "frip_storage_migrate_cleanup", "Clean up / migrate");
	ctx->droplist_from = ctrl_droplist(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigGeneralHandler, P(ctx));
	ctx->listbox = ctrl_listbox(s, NULL, NO_SHORTCUT, WFP_HELP_CTX, WffspConfigGeneralHandler, P(ctx));
	ctx->listbox->listbox.height = 7;
	ctx->listbox->listbox.multisel = 1;
	ctx->button_cleanup = ctrl_pushbutton(s, "Clean up...", 'e', WFP_HELP_CTX, WffspConfigGeneralHandler, P(ctx));
	ctrl_columns(s, 2, 50, 50);
	ctx->button_copy_all = ctrl_pushbutton(s, "Copy all...", 'y', WFP_HELP_CTX, WffspConfigGeneralHandler, P(ctx));
	ctx->button_copy_all->column = 0;
	ctx->button_move_all = ctrl_pushbutton(s, "Move all...", 'v', WFP_HELP_CTX, WffspConfigGeneralHandler, P(ctx));
	ctx->button_move_all->column = 1;
	ctrl_columns(s, 1, 100);
	ctx->droplist_to = ctrl_droplist(s, "To:", NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigGeneralHandler, P(ctx));
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
