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
 * Private types and variables
 */

/*
 * N.B.		WFFSP_ITEM_CONTAINER must remain the highest index item type
 */
typedef enum WffspConfigItem {
	WFFSP_ITEM_HOST_CAS		= 0,
	WFFSP_ITEM_HOST_KEYS		= 1,
	WFFSP_ITEM_SESSIONS		= 2,
	WFFSP_ITEM_JUMP_LIST		= 3,
	WFFSP_ITEM_PRIVKEY_LIST		= 4,
	WFFSP_ITEM_OPTIONS		= 5,
	WFFSP_ITEM_CONTAINER		= 6,

	WFFSP_ITEM_MIN			= WFFSP_ITEM_HOST_CAS,
	WFFSP_ITEM_MAX			= WFFSP_ITEM_CONTAINER,
} WffspConfigItem;


static const char *			WffspConfigItemNames[WFFSP_ITEM_MAX + 1] = {
	[WFFSP_ITEM_HOST_CAS]		= "Host CAs",
	[WFFSP_ITEM_HOST_KEYS]		= "Host keys",
	[WFFSP_ITEM_SESSIONS]		= "Sessions",
	[WFFSP_ITEM_JUMP_LIST]		= "Jump list",
	[WFFSP_ITEM_PRIVKEY_LIST]	= "Pageant private key list",
	[WFFSP_ITEM_OPTIONS]		= "Global options",
	[WFFSP_ITEM_CONTAINER]		= "(Container)",
};

typedef WfrStatus (*WffspConfigItemCleanupFn)(WfsBackend);
static WffspConfigItemCleanupFn		WffspConfigItemCleanupFnList[WFFSP_ITEM_MAX + 1] = {
	[WFFSP_ITEM_HOST_CAS]		= WfsCleanupHostCAs,
	[WFFSP_ITEM_HOST_KEYS]		= WfsCleanupHostKeys,
	[WFFSP_ITEM_SESSIONS]		= WfsCleanupSessions,
	[WFFSP_ITEM_JUMP_LIST]		= WfsCleanupJumpList,
	[WFFSP_ITEM_PRIVKEY_LIST]	= WfsCleanupPrivKeyList,
	[WFFSP_ITEM_OPTIONS]		= WfsCleanupOptions,
	[WFFSP_ITEM_CONTAINER]		= WfsCleanupContainer,
};

typedef WfrStatus (*WffspConfigItemExportFn)(WfsBackend, WfsBackend, bool, bool, WfsErrorFn);
static WffspConfigItemExportFn		WffspConfigItemExportFnList[WFFSP_ITEM_MAX + 1] = {
	[WFFSP_ITEM_HOST_CAS]		= WfsExportHostCAs,
	[WFFSP_ITEM_HOST_KEYS]		= WfsExportHostKeys,
	[WFFSP_ITEM_SESSIONS]		= WfsExportSessions,
	[WFFSP_ITEM_JUMP_LIST]		= WfsExportJumpList,
	[WFFSP_ITEM_PRIVKEY_LIST]	= WfsExportPrivKeyList,
	[WFFSP_ITEM_OPTIONS]		= WfsExportOptions,
	[WFFSP_ITEM_CONTAINER]		= NULL,
};


typedef struct WffspConfigContext {
	dlgcontrol *	droplist_from;
	dlgcontrol *	listbox;
	dlgcontrol *	button_cleanup;
	dlgcontrol *	button_copy_all;
	dlgcontrol *	button_move_all;
	dlgcontrol *	droplist_to;

	dlgcontrol *	droplist_purge_jump_list;
	dlgcontrol *	button_purge_jump_list;
} WffspConfigContext;
#define WFFSP_CONFIG_CONTEXT_EMPTY {		\
	.droplist_from = NULL,			\
	.listbox = NULL,			\
	.button_cleanup = NULL,			\
	.button_copy_all = NULL,		\
	.button_move_all = NULL,		\
	.droplist_to = NULL,			\
						\
	.droplist_purge_jump_list = NULL,	\
	.button_purge_jump_list = NULL,		\
}
#define WFFSP_CONFIG_CONTEXT_INIT(ctx)		\
	(ctx) = (WffspConfigContext)WFFSP_CONFIG_CONTEXT_EMPTY

/*
 * Private macros
 */

/*
 * Get from/source backend from item selected in from/source droplist
 */
#define WFFSP_CONFIG_GET_BACKEND_FROM(ctx, dlg)		\
	(WfsBackend)dlg_listbox_getid(			\
			(ctx)->droplist_from, (dlg),	\
			dlg_listbox_index((ctx)->droplist_from, (dlg)))

/*
 * Get to/destination backend from item selected in to/destination droplist
 */
#define WFFSP_CONFIG_GET_BACKEND_TO(ctx, dlg)		\
	(WfsBackend)dlg_listbox_getid(			\
			(ctx)->droplist_to, (dlg),	\
			dlg_listbox_index((ctx)->droplist_to, (dlg)))


/*
 * Get backend from item selected in purge jump list backend droplist
 */
#define WFFSP_CONFIG_GET_BACKEND_PURGE_JUMP_LIST(ctx, dlg)			\
	(WfsBackend)dlg_listbox_getid(						\
			(ctx)->droplist_purge_jump_list,			\
			(dlg),							\
			dlg_listbox_index((ctx)->droplist_purge_jump_list,	\
			(dlg)))


/*
 * Check if any items are selected in vector of selected items
 * initialised from item type listbox
 */
#define WFFSP_CONFIG_IF_SELECTV_NONE(selectv) ({					\
	size_t	item = WFFSP_ITEM_MIN;							\
											\
	do {										\
		if ((selectv)[item]) {							\
			true;								\
		}									\
	} while ((++item) <= WFFSP_ITEM_MAX);						\
											\
	false;										\
})

/*
 * Initialise vector of selected items in item type listbox
 */
#define WFFSP_CONFIG_INIT_SELECTV(ctx, dlg, selectv) ({					\
	size_t	item = WFFSP_ITEM_MIN;							\
											\
	do {										\
		(selectv)[item] = dlg_listbox_issel((ctx)->listbox, (dlg), item);	\
	} while ((++item) <= WFFSP_ITEM_MAX);						\
})

/*
 * Display message box with appended \n-separated list of item type names
 */
#define WFFSP_CONFIG_MESSAGEBOX_SELECTV(selectv, uType, fmt, ...)						\
	WfrMessageBoxF(												\
			"PuTTie", (uType),									\
			fmt "%s%s%s%s%s%s%s%s%s%s%s%s%s%s",							\
			##__VA_ARGS__,										\
			selectv[WFFSP_ITEM_HOST_CAS] ? WffspConfigItemNames[WFFSP_ITEM_HOST_CAS] : "",		\
			selectv[WFFSP_ITEM_HOST_CAS] ? "\n" : "",						\
			selectv[WFFSP_ITEM_HOST_KEYS] ? WffspConfigItemNames[WFFSP_ITEM_HOST_KEYS] : "",	\
			selectv[WFFSP_ITEM_HOST_KEYS] ? "\n" : "",						\
			selectv[WFFSP_ITEM_SESSIONS] ? WffspConfigItemNames[WFFSP_ITEM_SESSIONS] : "",		\
			selectv[WFFSP_ITEM_SESSIONS] ? "\n" : "",						\
			selectv[WFFSP_ITEM_JUMP_LIST] ? WffspConfigItemNames[WFFSP_ITEM_JUMP_LIST] : "",	\
			selectv[WFFSP_ITEM_JUMP_LIST] ? "\n" : "",						\
			selectv[WFFSP_ITEM_PRIVKEY_LIST] ? WffspConfigItemNames[WFFSP_ITEM_PRIVKEY_LIST] : "",	\
			selectv[WFFSP_ITEM_PRIVKEY_LIST] ? "\n" : "",						\
			selectv[WFFSP_ITEM_OPTIONS] ? WffspConfigItemNames[WFFSP_ITEM_OPTIONS] : "",		\
			selectv[WFFSP_ITEM_OPTIONS] ? "\n" : "",						\
			selectv[WFFSP_ITEM_CONTAINER] ? WffspConfigItemNames[WFFSP_ITEM_CONTAINER] : "",	\
			selectv[WFFSP_ITEM_CONTAINER] ? "\n" : "")

/*
 * Private subroutine prototypes
 */

static void	WffspConfigGeneralHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static void	WffspConfigGeneralCleanupHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static void	WffspConfigGeneralMigrateHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);

static void	WffspPurgeJumpList(WfsBackend backend);

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
	size_t			item;


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
		else if (ctrl == ctx->button_purge_jump_list) {
			WffspPurgeJumpList(
				WFFSP_CONFIG_GET_BACKEND_PURGE_JUMP_LIST(ctx, dlg));
		}
		break;

	case EVENT_REFRESH:
		if ((ctrl == ctx->droplist_from)
		||  (ctrl == ctx->droplist_to))
		{
			dlg_update_start(ctrl, dlg);
			(void)WffsRefreshBackends(ctrl, dlg, (ctrl == ctx->droplist_from));
			dlg_update_done(ctrl, dlg);
		} else if (ctrl == ctx->droplist_purge_jump_list) {
			dlg_update_start(ctrl, dlg);
			(void)WffsRefreshBackends(ctrl, dlg, true);
			dlg_update_done(ctrl, dlg);
		} else if (ctrl == ctx->listbox) {
			dlg_update_start(ctrl, dlg);
			dlg_listbox_clear(ctrl, dlg);

			item = WFFSP_ITEM_MIN;
			do {
				dlg_listbox_addwithid(ctrl, dlg, WffspConfigItemNames[item], item);
			} while ((++item) <= WFFSP_ITEM_MAX);

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
	size_t			item;
	bool			selectv[WFFSP_ITEM_MAX + 1];
	WfrStatus		status;


	(void)data;
	(void)event;
	backend_from = WFFSP_CONFIG_GET_BACKEND_FROM(ctx, dlg);
	WFFSP_CONFIG_INIT_SELECTV(ctx, dlg, selectv);

	if (WFFSP_CONFIG_IF_SELECTV_NONE(selectv)) {
		return;
	} else if (WFR_FAILURE(status = WfsGetBackendName(backend_from, &backend_from_name))) {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "getting backend name");
		return;
	}

	switch (WFFSP_CONFIG_MESSAGEBOX_SELECTV(
			selectv,
			MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1,
			"Clean up the following items in %s backend?\n",
			backend_from_name))
	{
	case IDYES:
		confirmfl = true; break;
	case IDNO: default:
		confirmfl = false; break;
	}

	if (confirmfl) {
		status = WFR_STATUS_CONDITION_SUCCESS;

		item = WFFSP_ITEM_MIN;
		do {
			if (selectv[item]) {
				status = WffspConfigItemCleanupFnList[item](backend_from);
				WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "cleaning up %s", WffspConfigItemNames[item]);
			}
		} while ((++item) <= WFFSP_ITEM_MAX);

		if (WFR_SUCCESS(status)) {
			(void)WFFSP_CONFIG_MESSAGEBOX_SELECTV(
				selectv,
				MB_ICONINFORMATION | MB_OK | MB_DEFBUTTON1,
				"Successfully cleaned up items in %s backend:\n",
				backend_from_name);
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
	WfsErrorFn		error_fn =
				WFR_LAMBDA(void, (const char *name, WfrStatus status) {
					WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "migrating %s", name);
				});
	size_t			item;
	bool			movefl;
	bool			selectv[WFFSP_ITEM_MAX + 1];
	WfrStatus		status;


	(void)data;
	(void)event;
	backend_from = WFFSP_CONFIG_GET_BACKEND_FROM(ctx, dlg);
	backend_to = WFFSP_CONFIG_GET_BACKEND_TO(ctx, dlg);
	movefl = ((ctrl == ctx->button_copy_all) ? false : true);
	WFFSP_CONFIG_INIT_SELECTV(ctx, dlg, selectv);

	if (backend_from == backend_to) {
		return;
	} else if (WFFSP_CONFIG_IF_SELECTV_NONE(selectv)) {
		return;
	} else if (WFR_FAILURE(status = WfsGetBackendName(backend_from, &backend_from_name))
		|| WFR_FAILURE(status = WfsGetBackendName(backend_to, &backend_to_name)))
	{
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "getting backend name");
		return;
	}

	switch (WFFSP_CONFIG_MESSAGEBOX_SELECTV(
			selectv,
			MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1,
			"%s the following items from %s to %s backend?\n",
			movefl ? "Move" : "Copy",
			backend_from_name, backend_to_name))
	{
	case IDYES:
		confirmfl = true; break;
	case IDNO: default:
		confirmfl = false; break;
	}

	if (confirmfl) {
		status = WFR_STATUS_CONDITION_SUCCESS;

		item = WFFSP_ITEM_MIN;
		do {
			if ((item != WFFSP_ITEM_CONTAINER) && selectv[item]) {
				if (WFR_SUCCESS(status = WffspConfigItemExportFnList[item](
						backend_from, backend_to, true, true, error_fn)))
				{
					if (movefl) {
						status = WffspConfigItemCleanupFnList[item](backend_from);
					}
				}
				WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "migrating %s", WffspConfigItemNames[item]);
			}
		} while ((++item) <= WFFSP_ITEM_MAX);

		if (movefl && WFR_SUCCESS(status)) {
			status = WfsCleanupContainer(backend_from);
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "cleaning up container");
		}

		if (WFR_SUCCESS(status)) {
			(void)WFFSP_CONFIG_MESSAGEBOX_SELECTV(
				selectv,
				MB_ICONQUESTION | MB_OK | MB_DEFBUTTON1,
				"Successfully %s the following items from %s to %s backend:\n",
				movefl ? "moved" : "copied",
				backend_from_name, backend_to_name);
		}
	}
}


static void
WffspPurgeJumpList(
	WfsBackend	backend
	)
{
	const char *	backend_name;
	bool		confirmfl;
	size_t		purge_count;
	WfrStatus	status;


	if (WFR_FAILURE(status = WfsGetBackendName(backend, &backend_name))) {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "getting backend name");
		return;
	}

	switch (WfrMessageBoxF(
			"PuTTie", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1,
			"Purge jump list in %s backend?", backend_name))
	{
	case IDYES:
		confirmfl = true; break;
	case IDNO: default:
		confirmfl = false; break;
	}

	if (confirmfl) {
		status = WfsPurgeJumpList(backend, &purge_count);
		if (WFR_SUCCESS(status)) {
			WfrMessageBoxF(
				"PuTTie", MB_ICONINFORMATION | MB_OK | MB_DEFBUTTON1,
				"Purged %u items in jump list in %s backend.",
				purge_count, backend_name);
		} else {
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "purging jump list");
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


	s = ctrl_getset(b, "Frippery/Storage", "frip_storage_purge_jump_list", "Purge jump list");
	ctx->droplist_purge_jump_list = ctrl_droplist(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigGeneralHandler, P(ctx));
	ctx->button_purge_jump_list = ctrl_pushbutton(s, "Purge...", 'p', WFP_HELP_CTX, WffspConfigGeneralHandler, P(ctx));
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
