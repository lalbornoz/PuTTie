/*
 * winfrip_feature_storage_sessions.c - pointless frippery & tremendous amounts of bloat
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
#include "PuTTie/winfrip_feature_storage_sessions.h"
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

	size_t			itemc[WFFSP_CDIR_COUNT];
	char **			itemv[WFFSP_CDIR_COUNT];
} WffspConfigContext;
#define WFFSP_CONFIG_CONTEXT_EMPTY {										\
		.droplist = {NULL, NULL},											\
		.editbox = {NULL, NULL},											\
		.listbox = {NULL, NULL},											\
																			\
		.button_clear = {NULL, NULL},										\
		.button_delete = {NULL, NULL},										\
		.button_rename = {NULL, NULL},										\
																			\
		.button_copy = NULL,												\
		.button_move = NULL,												\
																			\
		.itemc = {0, 0},													\
		.itemv = {NULL, NULL},												\
	}
#define WFFSP_CONFIG_CONTEXT_INIT(ctx)										\
		(ctx) = (WffspConfigContext)WFFSP_CONFIG_CONTEXT_EMPTY

/*
 * Private macros
 */

#define WFFSP_CONFIG_GET_BACKEND(ctx, dir, dlg)								\
		(WfsBackend)dlg_listbox_getid(										\
				(ctx)->droplist[dir], (dlg),								\
				dlg_listbox_index((ctx)->droplist[dir], (dlg)));

#define WFFSP_CONFIG_GET_NSESSION(ctx, dir, dlg, nsession) ({				\
			WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;				\
																			\
			nsession = dlg_listbox_getid(ctx->listbox[dir], dlg,			\
							dlg_listbox_index(ctx->listbox[dir], dlg));		\
			if ((nsession < 0)												\
			||  ((size_t)nsession >= ctx->itemc[dir]))						\
			{																\
				status = WFR_STATUS_FROM_ERRNO1(ENOENT);					\
			}																\
																			\
			status;															\
		})

/*
 * Private subroutine prototypes
 */

static WfrStatus WffspClearSessions(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus WffspDeleteSession(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus WffspExportSession(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus WffspRenameSession(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus WffspRefreshSessions(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus WffspSelectSession(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);

static void WffspConfigSessionsHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static WfrStatus WffspConfigUpdateSessions(WffspConfigContext *ctx, WffspConfigDirection dir, dlgparam *dlg, bool update_listbox);

/*
 * Private subroutines
 */

static WfrStatus
WffspClearSessions(
	dlgcontrol *			ctrl,
	dlgparam *				dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend				backend, backend_from, backend_to;
	const char *			backend_name;
	bool					confirmfl;
	WffspConfigDirection	dir;
	LPSTR					lpText;
	WfrStatus				status;


	dir = ((ctrl == ctx->button_clear[WFFSP_CDIR_FROM]) ? WFFSP_CDIR_FROM : WFFSP_CDIR_TO);
	backend = WFFSP_CONFIG_GET_BACKEND(ctx, dir, dlg);
	backend_from = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_FROM, dlg);
	backend_to = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_TO, dlg);
	status = WfsGetBackendName(backend, &backend_name);

	if (WFR_STATUS_SUCCESS(status)
	&&  WFR_STATUS_SUCCESS(status = WfrSnDuprintf(
				&lpText, NULL, "Clear all sessions in the %s backend?", backend_name)))
	{
		switch (MessageBox(
					NULL, lpText, "PuTTY",
					MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1)) {
		case IDYES:
			confirmfl = true; break;
		case IDNO: default:
			confirmfl = false; break;
		}

		if (confirmfl) {
			if (WFR_STATUS_SUCCESS(status = WfsClearSessions(backend, true)))
			{
				if (backend_from == backend_to) {
					if (WFR_STATUS_SUCCESS(status = WffspConfigUpdateSessions(ctx, WFFSP_CDIR_FROM, dlg, true))
					&&  WFR_STATUS_SUCCESS(status = WffspConfigUpdateSessions(ctx, WFFSP_CDIR_TO, dlg, true)))
					{
						status = WFR_STATUS_CONDITION_SUCCESS;
					}
				} else {
					status = WffspConfigUpdateSessions(ctx, dir, dlg, true);
				}
			}
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX("clearing sessions", status);

	return status;
}

static WfrStatus
WffspDeleteSession(
	dlgcontrol *			ctrl,
	dlgparam *				dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend				backend, backend_from, backend_to;
	WffspConfigDirection	dir;
	int						nsession;
	WfrStatus				status;


	dir = ((ctrl == ctx->button_delete[WFFSP_CDIR_FROM]) ? WFFSP_CDIR_FROM : WFFSP_CDIR_TO);
	backend = WFFSP_CONFIG_GET_BACKEND(ctx, dir, dlg);
	backend_from = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_FROM, dlg);
	backend_to = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_TO, dlg);

	if (WFR_STATUS_SUCCESS(status = WFFSP_CONFIG_GET_NSESSION(ctx, dir, dlg, nsession))
	&&  WFR_STATUS_SUCCESS(status = WfsDeleteSession(backend, true, ctx->itemv[dir][nsession])))
	{
		if (backend_from == backend_to) {
			if (WFR_STATUS_SUCCESS(status = WffspConfigUpdateSessions(ctx, WFFSP_CDIR_FROM, dlg, true))
			&&  WFR_STATUS_SUCCESS(status = WffspConfigUpdateSessions(ctx, WFFSP_CDIR_TO, dlg, true)))
			{
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		} else {
			status = WffspConfigUpdateSessions(ctx, dir, dlg, true);
		}
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX("deleting session", status);

	return status;
}

static WfrStatus
WffspExportSession(
	dlgcontrol *			ctrl,
	dlgparam *				dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend	backend_from, backend_to;
	bool		movefl;
	int			nsession;
	char *		sessionname;
	WfrStatus	status;


	backend_from = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_FROM, dlg);
	backend_to = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_TO, dlg);

	if (backend_from != backend_to) {
		movefl = ((ctrl == ctx->button_copy) ? false : true);

		status = WFR_STATUS_CONDITION_SUCCESS;
		for (int index = 0; (size_t)index < ctx->itemc[WFFSP_CDIR_FROM]; index++) {
			if (dlg_listbox_issel(ctx->listbox[WFFSP_CDIR_FROM], dlg, index)) {
				nsession = dlg_listbox_getid(ctx->listbox[WFFSP_CDIR_FROM], dlg, index);
				sessionname = ctx->itemv[WFFSP_CDIR_FROM][nsession];

				status = WfsExportSession(backend_from, backend_to, movefl, sessionname);
				if (WFR_STATUS_FAILURE(status)) {
					break;
				}
			}
		}

		if (WFR_STATUS_SUCCESS(status)) {
			if (movefl) {
				status = WffspConfigUpdateSessions(ctx, WFFSP_CDIR_FROM, dlg, true);
			}
			if (WFR_STATUS_SUCCESS(status)) {
				status = WffspConfigUpdateSessions(ctx, WFFSP_CDIR_TO, dlg, true);
			}
		}
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX("exporting session", status);

	return status;
}

static WfrStatus
WffspRenameSession(
	dlgcontrol *			ctrl,
	dlgparam *				dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend				backend, backend_from, backend_to;
	WffspConfigDirection	dir;
	int						nsession;
	char *					sessionname_new;
	WfrStatus				status;


	dir = ((ctrl == ctx->button_rename[WFFSP_CDIR_FROM]) ? WFFSP_CDIR_FROM : WFFSP_CDIR_TO);
	backend = WFFSP_CONFIG_GET_BACKEND(ctx, dir, dlg);
	backend_from = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_FROM, dlg);
	backend_to = WFFSP_CONFIG_GET_BACKEND(ctx, WFFSP_CDIR_TO, dlg);
	sessionname_new = dlg_editbox_get(ctx->editbox[dir], dlg);

	if (WFR_STATUS_SUCCESS(status = WFFSP_CONFIG_GET_NSESSION(ctx, dir, dlg, nsession))
	&&  WFR_STATUS_SUCCESS(status = WfsRenameSession(backend, true, ctx->itemv[dir][nsession], sessionname_new)))
	{
		if (backend_from == backend_to) {
			if (WFR_STATUS_SUCCESS(status = WffspConfigUpdateSessions(ctx, WFFSP_CDIR_FROM, dlg, true))
			&&  WFR_STATUS_SUCCESS(status = WffspConfigUpdateSessions(ctx, WFFSP_CDIR_TO, dlg, true)))
			{
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		} else {
			status = WffspConfigUpdateSessions(ctx, dir, dlg, true);
		}
	}

	sfree(sessionname_new);

	WFR_IF_STATUS_FAILURE_MESSAGEBOX("renaming session", status);

	return WFR_STATUS_CONDITION_SUCCESS;
}

static WfrStatus
WffspRefreshSessions(
	dlgcontrol *			ctrl,
	dlgparam *				dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend				backend;
	const char *			backend_name;
	WffspConfigDirection	dir;
	WfrStatus				status;


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
			dlg_listbox_select(ctrl, dlg, WFS_BACKEND_FILE);
		} else if (ctrl == ctx->droplist[WFFSP_CDIR_TO]) {
			dlg_listbox_select(ctrl, dlg, WFS_BACKEND_REGISTRY);
		}

		status = WffspConfigUpdateSessions(ctx, dir, dlg, true);
		dlg_update_done(ctrl, dlg);
	} else if (ctrl == ctx->listbox[WFFSP_CDIR_FROM]) {
		status = WffspConfigUpdateSessions(ctx, WFFSP_CDIR_FROM, dlg, true);
	} else if (ctrl == ctx->listbox[WFFSP_CDIR_TO]) {
		status = WffspConfigUpdateSessions(ctx, WFFSP_CDIR_TO, dlg, true);
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX("refreshing session list", status);

	return status;
}

static WfrStatus
WffspSelectSession(
	dlgcontrol *			ctrl,
	dlgparam *				dlg,
	WffspConfigContext *	ctx
	)
{
	WffspConfigDirection	dir;
	int						nsession;
	WfrStatus				status;


	if (ctrl == ctx->droplist[WFFSP_CDIR_FROM]) {
		status = WffspConfigUpdateSessions(ctx, WFFSP_CDIR_FROM, dlg, true);
	} else if (ctrl == ctx->droplist[WFFSP_CDIR_TO]) {
		status = WffspConfigUpdateSessions(ctx, WFFSP_CDIR_TO, dlg, true);
	} else if ((ctrl == ctx->listbox[WFFSP_CDIR_FROM])
			|| (ctrl == ctx->listbox[WFFSP_CDIR_TO]))
	{
		dir = ((ctrl == ctx->listbox[WFFSP_CDIR_FROM]) ? WFFSP_CDIR_FROM : WFFSP_CDIR_TO);

		nsession = dlg_listbox_getid(ctrl, dlg, dlg_listbox_index(ctrl, dlg));
		if (nsession >= 0) {
			if (WFR_STATUS_SUCCESS(status = WFFSP_CONFIG_GET_NSESSION(ctx, dir, dlg, nsession))) {
				dlg_editbox_set(ctx->editbox[dir], dlg, ctx->itemv[dir][nsession]);
			}
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX("selecting session", status);

	return status;
}


static void
WffspConfigSessionsHandler(
	dlgcontrol *	ctrl,
	dlgparam *		dlg,
	void *			data,
	int				event
	)
{
	WffspConfigContext *	ctx = (WffspConfigContext *)ctrl->context.p;


	(void)data;
	switch (event) {
	case EVENT_ACTION:
		if ((ctrl == ctx->button_copy)
		||  (ctrl == ctx->button_move))
		{
			(void)WffspExportSession(ctrl, dlg, ctx);
		} else if ((ctrl == ctx->button_clear[WFFSP_CDIR_FROM])
				|| (ctrl == ctx->button_clear[WFFSP_CDIR_TO]))
		{
			(void)WffspClearSessions(ctrl, dlg, ctx);
		} else if ((ctrl == ctx->button_delete[WFFSP_CDIR_FROM])
				|| (ctrl == ctx->button_delete[WFFSP_CDIR_TO]))
		{
			(void)WffspDeleteSession(ctrl, dlg, ctx);
		} else if ((ctrl == ctx->button_rename[WFFSP_CDIR_FROM])
				|| (ctrl == ctx->button_rename[WFFSP_CDIR_TO]))
		{
			(void)WffspRenameSession(ctrl, dlg, ctx);
		}
		break;

	case EVENT_REFRESH:
		(void)WffspRefreshSessions(ctrl, dlg, ctx);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		(void)WffspSelectSession(ctrl, dlg, ctx);
		break;
	}
}

static WfrStatus
WffspConfigUpdateSessions(
	WffspConfigContext *	ctx,
	WffspConfigDirection	dir,
	dlgparam *				dlg,
	bool					update_listbox
	)
{
	WfsBackend		backend;
	dlgcontrol *	ctrl;
	bool			donefl;
	void *			enum_state;
	size_t			itemc_new = 0;
	char *			sessionname;
	char **			itemv_new = NULL;
	WfrStatus		status;


	ctrl = ctx->listbox[dir];
	backend = WFFSP_CONFIG_GET_BACKEND(ctx, dir, dlg);
	donefl = false;

	if (WFR_STATUS_SUCCESS(status = WfsEnumerateSessions(
				backend, false, true, NULL, NULL, &enum_state)))
	{
		do {
			status = WfsEnumerateSessions(
					backend, false, false,
					&donefl, &sessionname, enum_state);

			if (WFR_STATUS_SUCCESS(status) && !donefl) {
				if (WFR_STATUS_SUCCESS(status = WFR_SRESIZE_IF_NEQ_SIZE(
									itemv_new, itemc_new, itemc_new + 1, char *)))
				{
					itemv_new[itemc_new - 1] = sessionname;
				}
			}
		} while (WFR_STATUS_SUCCESS(status) && !donefl);

		if (WFR_STATUS_SUCCESS(status)) {
			for (size_t nsession = 0; nsession < ctx->itemc[dir]; nsession++) {
				WFR_SFREE_IF_NOTNULL(ctx->itemv[dir][nsession]);
			}
			WFR_SFREE_IF_NOTNULL(ctx->itemv[dir]);

			ctx->itemc[dir] = itemc_new;
			ctx->itemv[dir] = itemv_new;

			if (update_listbox) {
				dlg_update_start(ctrl, dlg);
				dlg_listbox_clear(ctrl, dlg);

				for (size_t nsession = 0; nsession < itemc_new; nsession++) {
					dlg_listbox_addwithid(ctrl, dlg, itemv_new[nsession], nsession);
				}

				dlg_update_done(ctrl, dlg);
			}
		} else {
			if (itemv_new) {
				for (size_t nsession = 0; nsession < itemc_new; nsession++) {
					WFR_SFREE_IF_NOTNULL(itemv_new[nsession]);
				}
				sfree(itemv_new);
			}
		}

		sfree(enum_state);
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX("updating session list", status);

	return status;
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
WffsSessionsConfigPanel(
	struct controlbox *		b
	)
{
	WffspConfigContext *	ctx;
	struct controlset *		s;


	/*
	 * The Frippery: sessions panel.
	 */

	ctrl_settitle(b, "Frippery/Sessions", "Configure pointless frippery: sessions");


	ctx = snew(WffspConfigContext);
	WFFSP_CONFIG_CONTEXT_INIT(*ctx);

	s = ctrl_getset(b, "Frippery/Sessions", "frip_sessions_from", "From:");
	ctrl_columns(s, 2, 70, 30);
	ctx->editbox[WFFSP_CDIR_FROM] = ctrl_editbox(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigSessionsHandler, P(ctx), P(NULL));
	ctx->editbox[WFFSP_CDIR_FROM]->column = 0;
	ctx->droplist[WFFSP_CDIR_FROM] = ctrl_droplist(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigSessionsHandler, P(ctx));
	ctx->droplist[WFFSP_CDIR_FROM]->column = 1;
	/* Reset columns so that the buttons are alongside the list, rather
	 * than alongside that edit box. */
	ctrl_columns(s, 1, 100);
	ctrl_columns(s, 2, 75, 25);
	ctx->listbox[WFFSP_CDIR_FROM] = ctrl_listbox(s, NULL, NO_SHORTCUT, WFP_HELP_CTX, WffspConfigSessionsHandler, P(ctx));
	ctx->listbox[WFFSP_CDIR_FROM]->column = 0;
	ctx->listbox[WFFSP_CDIR_FROM]->listbox.height = 6;
	ctx->listbox[WFFSP_CDIR_FROM]->listbox.multisel = 1;
	ctx->button_clear[WFFSP_CDIR_FROM] = ctrl_pushbutton(s, "Clear...", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigSessionsHandler, P(ctx));
	ctx->button_clear[WFFSP_CDIR_FROM]->column = 1;
	ctx->button_delete[WFFSP_CDIR_FROM] = ctrl_pushbutton(s, "Delete", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigSessionsHandler, P(ctx));
	ctx->button_delete[WFFSP_CDIR_FROM]->column = 1;
	ctx->button_rename[WFFSP_CDIR_FROM] = ctrl_pushbutton(s, "Rename", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigSessionsHandler, P(ctx));
	ctx->button_rename[WFFSP_CDIR_FROM]->column = 1;
	ctrl_columns(s, 1, 100);

	s = ctrl_getset(b, "Frippery/Sessions", "frip_sessions_export", "Export from/to:");
	ctrl_columns(s, 2, 50, 50);
	ctx->button_copy = ctrl_pushbutton(s, "Copy", 'p', WFP_HELP_CTX, WffspConfigSessionsHandler, P(ctx));
	ctx->button_copy->column = 0;
	ctx->button_move = ctrl_pushbutton(s, "Move", 'm', WFP_HELP_CTX, WffspConfigSessionsHandler, P(ctx));
	ctx->button_move->column = 1;
	ctrl_columns(s, 1, 100);

	s = ctrl_getset(b, "Frippery/Sessions", "frip_sessions_to", "To:");
	ctrl_columns(s, 2, 70, 30);
	ctx->editbox[WFFSP_CDIR_TO] = ctrl_editbox(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigSessionsHandler, P(ctx), P(NULL));
	ctx->editbox[WFFSP_CDIR_TO]->column = 0;
	ctx->droplist[WFFSP_CDIR_TO] = ctrl_droplist(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigSessionsHandler, P(ctx));
	ctx->droplist[WFFSP_CDIR_TO]->column = 1;
	/* Reset columns so that the buttons are alongside the list, rather
	 * than alongside that edit box. */
	ctrl_columns(s, 1, 100);
	ctrl_columns(s, 2, 75, 25);
	ctx->listbox[WFFSP_CDIR_TO] = ctrl_listbox(s, NULL, NO_SHORTCUT, WFP_HELP_CTX, WffspConfigSessionsHandler, P(ctx));
	ctx->listbox[WFFSP_CDIR_TO]->column = 0;
	ctx->listbox[WFFSP_CDIR_TO]->listbox.height = 6;
	ctx->listbox[WFFSP_CDIR_TO]->listbox.multisel = 1;
	ctx->button_clear[WFFSP_CDIR_TO] = ctrl_pushbutton(s, "Clear...", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigSessionsHandler, P(ctx));
	ctx->button_clear[WFFSP_CDIR_TO]->column = 1;
	ctx->button_delete[WFFSP_CDIR_TO] = ctrl_pushbutton(s, "Delete", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigSessionsHandler, P(ctx));
	ctx->button_delete[WFFSP_CDIR_TO]->column = 1;
	ctx->button_rename[WFFSP_CDIR_TO] = ctrl_pushbutton(s, "Rename", NO_SHORTCUT, WFP_HELP_CTX, WffspConfigSessionsHandler, P(ctx));
	ctx->button_rename[WFFSP_CDIR_TO]->column = 1;
	ctrl_columns(s, 1, 100);
}

void
WffsSessionsConfigPanelDroplistBackendHandler(
	dlgcontrol *	ctrl,
	dlgparam *		dlg,
	void *			data,
	int				event
	)
{
	WfsBackend		backend;
	const char *	backend_name;
	dlgcontrol *	ctrl_sessionsaver;
	int				id;
	WfrStatus		status;


	(void)data;

	ctrl_sessionsaver = *(dlgcontrol **)ctrl->context.p;
	switch (event) {
	case EVENT_REFRESH:
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);

		backend = WFS_BACKEND_MIN;
		do {
			if (WFR_STATUS_SUCCESS(status = WfsGetBackendName(backend, &backend_name))) {
				dlg_listbox_addwithid(ctrl, dlg, backend_name, backend);
			}
		} while (WfsGetBackendNext(&backend));

		dlg_listbox_select(ctrl, dlg, WfsGetBackend());
		dlg_update_done(ctrl, dlg);

		WFR_IF_STATUS_FAILURE_MESSAGEBOX("updating backends list", status);

		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		id = dlg_listbox_getid(ctrl, dlg, dlg_listbox_index(ctrl, dlg));
		status = WfsSetBackend(id, id, false);
		ctrl_sessionsaver->handler(ctrl_sessionsaver, dlg, data, EVENT_REFRESH);

		WFR_IF_STATUS_FAILURE_MESSAGEBOX("setting backend", status);

		break;
	}
}

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
