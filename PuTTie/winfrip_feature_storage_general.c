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

/*
 * Private types
 */

typedef struct WffspConfigContext {
	dlgcontrol *	droplist_from;
	dlgcontrol *	droplist_to;
	dlgcontrol *	button_copy;
	dlgcontrol *	button_move;
} WffspConfigContext;
#define WFFSP_CONFIG_CONTEXT_EMPTY {			\
	.droplist_from = NULL,				\
	.droplist_to = NULL,				\
	.button_copy = NULL,				\
	.button_move = NULL,				\
}
#define WFFSP_CONFIG_CONTEXT_INIT(ctx)			\
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

static WfrStatus	WffspExportJumpList(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);
static WfrStatus	WffspRefreshBackends(dlgcontrol *ctrl, dlgparam *dlg, WffspConfigContext *ctx);

static void		WffspConfigGeneralHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);

/*
 * Private subroutines
 */

static WfrStatus
WffspExportJumpList(
	dlgcontrol *	ctrl,
	dlgparam *		dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend	backend_from, backend_to;
	bool		movefl;
	WfrStatus	status;


	backend_from = WFFSP_CONFIG_GET_BACKEND_FROM(ctx, dlg);
	backend_to = WFFSP_CONFIG_GET_BACKEND_TO(ctx, dlg);

	if (backend_from != backend_to) {
		movefl = ((ctrl == ctx->button_copy) ? false : true);
		status = WfsJumpListExport(backend_from, backend_to, movefl);
		if (WFR_STATUS_SUCCESS(status)) {
			(void)MessageBox(NULL, "Successfully exported jump list", "PuTTie", MB_OK | MB_ICONINFORMATION);
		}
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "exporting jump list");

	return status;
}

static WfrStatus
WffspRefreshBackends(
	dlgcontrol *		ctrl,
	dlgparam *		dlg,
	WffspConfigContext *	ctx
	)
{
	WfsBackend	backend;
	const char *	backend_name;
	WfrStatus	status;


	if ((ctrl == ctx->droplist_from)
	||  (ctrl == ctx->droplist_to))
	{
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);

		backend = WFS_BACKEND_MIN;
		do {
			if (WFR_STATUS_SUCCESS(status = WfsGetBackendName(backend, &backend_name))) {
				dlg_listbox_addwithid(ctrl, dlg, backend_name, backend);
			}
		} while (WfsGetBackendNext(&backend));

		if (ctrl == ctx->droplist_from) {
			dlg_listbox_select(ctrl, dlg, WFS_BACKEND_REGISTRY);
		} else if (ctrl == ctx->droplist_to) {
			dlg_listbox_select(ctrl, dlg, WFS_BACKEND_FILE);
		}

		dlg_update_done(ctrl, dlg);
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "refreshing backends");

	return status;
}


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
		if ((ctrl == ctx->button_copy)
		||  (ctrl == ctx->button_move))
		{
			(void)WffspExportJumpList(ctrl, dlg, ctx);
		}
		break;

	case EVENT_REFRESH:
		(void)WffspRefreshBackends(ctrl, dlg, ctx);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		break;
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


	ctx = snew(WffspConfigContext);
	WFFSP_CONFIG_CONTEXT_INIT(*ctx);

	s = ctrl_getset(b, "Frippery/Storage", "frip_storage_jumplist", "Jump lists");
	ctrl_columns(s, 2, 50, 50);
	ctx->droplist_from = ctrl_droplist(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigGeneralHandler, P(ctx));
	ctx->droplist_from->column = 0;
	ctx->droplist_to = ctrl_droplist(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, WffspConfigGeneralHandler, P(ctx));
	ctx->droplist_to->column = 1;
	ctrl_columns(s, 1, 100);
	ctrl_columns(s, 2, 50, 50);
	ctx->button_copy = ctrl_pushbutton(s, "Copy", 'p', WFP_HELP_CTX, WffspConfigGeneralHandler, P(ctx));
	ctx->button_copy->column = 0;
	ctx->button_move = ctrl_pushbutton(s, "Move", 'm', WFP_HELP_CTX, WffspConfigGeneralHandler, P(ctx));
	ctx->button_move->column = 1;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
