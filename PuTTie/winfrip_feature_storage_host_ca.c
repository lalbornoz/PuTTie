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
#include "PuTTie/winfrip_feature_storage_priv.h"
#include "PuTTie/winfrip_storage_host_ca.h"

/*
 * Private subroutine prototypes
 */

static void	WffspConfigHostCAsHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);

/*
 * Private subroutines
 */

static void
WffspConfigHostCAsHandler(
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
			(void)WffsClearItems(WfsClearHostCAs, ctrl, ctx, dlg, "host CAs", WfsEnumerateHostCAs);
		} else if ((ctrl == ctx->button_delete[WFFS_DIR_FROM])
			|| (ctrl == ctx->button_delete[WFFS_DIR_TO]))
		{
			(void)WffsDeleteItem(WfsDeleteHostCA, ctrl, ctx, dlg, WfsEnumerateHostCAs);
		} else if ((ctrl == ctx->button_copy)
			|| (ctrl == ctx->button_move))
		{
			(void)WffsExportItem(ctrl, ctx, dlg, WfsExportHostCA, WfsEnumerateHostCAs);
		} else if ((ctrl == ctx->button_rename[WFFS_DIR_FROM])
			|| (ctrl == ctx->button_rename[WFFS_DIR_TO]))
		{
			(void)WffsRenameItem(ctrl, ctx, dlg, WfsRenameHostCA, WfsEnumerateHostCAs);
		}
		break;

	case EVENT_REFRESH:
		(void)WffsRefreshItems(ctrl, ctx, dlg, WfsEnumerateHostCAs);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		(void)WffsSelectItem(ctrl, ctx, dlg, WfsEnumerateHostCAs);
		break;
	}
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
WffsHostCAConfigPanel(
	struct controlbox *	b
	)
{
	(void)WffsInitPanel(
		b, WffspConfigHostCAsHandler,
		"frip_host_cas_export",
		"frip_host_cas_from",
		"frip_host_cas_to",
		"Frippery/Host CAs",
		"Configure pointless frippery: host CAs");
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
