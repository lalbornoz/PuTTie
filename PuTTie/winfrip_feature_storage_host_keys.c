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
#include "PuTTie/winfrip_feature_storage_priv.h"
#include "PuTTie/winfrip_storage_host_keys.h"

/*
 * Private subroutine prototypes
 */

static void	WffspConfigHostKeysHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);

/*
 * Private subroutines
 */

static void
WffspConfigHostKeysHandler(
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
			(void)WffsClearItems(WfsClearHostKeys, ctrl, ctx, dlg, "host keys", WfsEnumerateHostKeys);
		} else if ((ctrl == ctx->button_delete[WFFS_DIR_FROM])
			|| (ctrl == ctx->button_delete[WFFS_DIR_TO]))
		{
			(void)WffsDeleteItem(WfsDeleteHostKey, ctrl, ctx, dlg, WfsEnumerateHostKeys);
		} else if ((ctrl == ctx->button_copy)
			|| (ctrl == ctx->button_move))
		{
			(void)WffsExportItem(ctrl, ctx, dlg, WfsExportHostKey, WfsEnumerateHostKeys);
		} else if ((ctrl == ctx->button_rename[WFFS_DIR_FROM])
			|| (ctrl == ctx->button_rename[WFFS_DIR_TO]))
		{
			(void)WffsRenameItem(ctrl, ctx, dlg, WfsRenameHostKey, WfsEnumerateHostKeys);
		}
		break;

	case EVENT_REFRESH:
		(void)WffsRefreshItems(ctrl, ctx, dlg, WfsEnumerateHostKeys);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		(void)WffsSelectItem(ctrl, ctx, dlg, WfsEnumerateHostKeys);
		break;
	}
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
WffsHostKeysConfigPanel(
	struct controlbox *	b
	)
{
	(void)WffsInitPanel(
		b, WffspConfigHostKeysHandler,
		"frip_host_keys_export",
		"frip_host_keys_from",
		"frip_host_keys_to",
		"Frippery/Host keys",
		"Configure pointless frippery: host keys");
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
