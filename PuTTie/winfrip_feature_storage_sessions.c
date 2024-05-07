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
#include "PuTTie/winfrip_rtl_windows.h"
#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_feature_storage_sessions.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_feature_storage_priv.h"
#include "PuTTie/winfrip_storage_sessions.h"

/*
 * Private subroutine prototypes
 */

static void	WffspConfigSessionsHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);

/*
 * Private subroutines
 */

static void
WffspConfigSessionsHandler(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	void *		data,
	int		event
	)
{
	WffsContext *	ctx = (WffsContext *)ctrl->context.p;


	(void)data;
	switch (event) {
	case EVENT_ACTION:
		if ((ctrl == ctx->button_clear[WFFS_DIR_FROM])
		||  (ctrl == ctx->button_clear[WFFS_DIR_TO]))
		{
			(void)WffsClearItems(WfsClearSessions, ctrl, ctx, dlg, "sessions", WfsEnumerateSessions);
		} else if ((ctrl == ctx->button_delete[WFFS_DIR_FROM])
			|| (ctrl == ctx->button_delete[WFFS_DIR_TO]))
		{
			(void)WffsDeleteItem(WfsDeleteSession, ctrl, ctx, dlg, WfsEnumerateSessions);
		} else if ((ctrl == ctx->button_copy)
			|| (ctrl == ctx->button_move))
		{
			(void)WffsExportItem(ctrl, ctx, dlg, WfsExportSession, WfsEnumerateSessions);
		} else if ((ctrl == ctx->button_rename[WFFS_DIR_FROM])
			|| (ctrl == ctx->button_rename[WFFS_DIR_TO]))
		{
			(void)WffsRenameItem(ctrl, ctx, dlg, WfsRenameSession, WfsEnumerateSessions);
		}
		break;

	case EVENT_REFRESH:
		(void)WffsRefreshItems(ctrl, ctx, dlg, WfsEnumerateSessions);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		(void)WffsSelectItem(ctrl, ctx, dlg, WfsEnumerateSessions);
		break;
	}
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
WffsSessionsConfigPanel(
	struct controlbox *	b
	)
{
	(void)WffsInitPanel(
		b, WffspConfigSessionsHandler,
		"frip_sessions_export",
		"frip_sessions_from",
		"frip_sessions_to",
		"Frippery/Sessions",
		"Configure pointless frippery: sessions");
}

void
WffsSessionsConfigPanelDroplistBackendHandler(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	void *		data,
	int		event
	)
{
	WfsBackend	backend;
	const char *	backend_name;
	dlgcontrol *	ctrl_sessionsaver;
	int		id;
	WfrStatus	status;


	(void)data;

	ctrl_sessionsaver = *(dlgcontrol **)ctrl->context.p;
	switch (event) {
	case EVENT_REFRESH:
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);

		backend = WFS_BACKEND_MIN;
		do {
			if (WFR_SUCCESS(status = WfsGetBackendName(backend, &backend_name))) {
				dlg_listbox_addwithid(ctrl, dlg, backend_name, backend);
			}
		} while (WfsGetBackendNext(&backend));

		dlg_listbox_select(ctrl, dlg, WfsGetBackend());
		dlg_update_done(ctrl, dlg);

		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, NULL, "updating backends list");

		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		id = dlg_listbox_getid(ctrl, dlg, dlg_listbox_index(ctrl, dlg));
		status = WfsSetBackend(id, id, false, NULL);
		ctrl_sessionsaver->handler(ctrl_sessionsaver, dlg, data, EVENT_REFRESH);

		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, NULL, "setting backend");

		break;
	}
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
