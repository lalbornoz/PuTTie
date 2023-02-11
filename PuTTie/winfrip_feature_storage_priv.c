/*
 * winfrip_feature_storage_priv.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "dialog.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_feature_storage_priv.h"

/*
 * Public subroutines private to PuTTie/winfrip_feature_storage*.c
 */

WfrStatus
WffsClearItems(
	WffsClearItemsFn	clear_items_fn,
	dlgcontrol *		ctrl,
	WffsContext *		ctx,
	dlgparam *		dlg,
	const char *		items_title,
	WffsUpdateItemListFn	update_fn
	)
{
	WfsBackend	backend, backend_from, backend_to;
	const char *	backend_name;
	bool		confirmfl;
	WffsDirection	dir;
	LPSTR		lpText;
	WfrStatus	status;


	dir = ((ctrl == ctx->button_clear[WFFS_DIR_FROM]) ? WFFS_DIR_FROM : WFFS_DIR_TO);
	backend = WFFS_GET_BACKEND(ctx, dir, dlg);
	backend_from = WFFS_GET_BACKEND(ctx, WFFS_DIR_FROM, dlg);
	backend_to = WFFS_GET_BACKEND(ctx, WFFS_DIR_TO, dlg);

	if (WFR_SUCCESS(status = WfsGetBackendName(backend, &backend_name))
	&&  WFR_SUCCESS(status = WfrSnDuprintf(
			&lpText, NULL, "Clear all %s in the %s backend?",
			items_title, backend_name)))
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
			if (WFR_SUCCESS(status = clear_items_fn(backend, true)))
			{
				if (backend_from == backend_to) {
					if (WFR_SUCCESS(status = WffsUpdateItemList(
							backend_from, ctx->listbox[WFFS_DIR_FROM],
							dlg, &ctx->itemc[WFFS_DIR_FROM],
							&ctx->itemv[WFFS_DIR_FROM], update_fn, true))
					&&  WFR_SUCCESS(status = WffsUpdateItemList(
							backend_to, ctx->listbox[WFFS_DIR_TO],
							dlg, &ctx->itemc[WFFS_DIR_TO],
							&ctx->itemv[WFFS_DIR_TO], update_fn, true)))
					{
						status = WFR_STATUS_CONDITION_SUCCESS;
					}
				} else {
					status = WffsUpdateItemList(
						backend, ctx->listbox[dir], dlg,
						&ctx->itemc[dir], &ctx->itemv[dir],
						update_fn, true);
				}
			}
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "clearing items");

	return status;
}

WfrStatus
WffsDeleteItem(
	WffsDeleteItemFn	delete_item_fn,
	dlgcontrol *		ctrl,
	WffsContext *		ctx,
	dlgparam *		dlg,
	WffsUpdateItemListFn	update_fn
	)
{
	WfsBackend	backend, backend_from, backend_to;
	WffsDirection	dir;
	char *		name = NULL;
	int		nitem = -1;
	WfrStatus	status, status_;


	dir = ((ctrl == ctx->button_delete[WFFS_DIR_FROM]) ? WFFS_DIR_FROM : WFFS_DIR_TO);
	backend = WFFS_GET_BACKEND(ctx, dir, dlg);
	backend_from = WFFS_GET_BACKEND(ctx, WFFS_DIR_FROM, dlg);
	backend_to = WFFS_GET_BACKEND(ctx, WFFS_DIR_TO, dlg);

	if (WFR_SUCCESS(status = WFFS_GET_ITEM(ctx, dir, dlg, nitem))) {
		name = ctx->itemv[dir][nitem];
		status = delete_item_fn(backend, true, name);

		if (backend_from == backend_to) {
			if (WFR_SUCCESS(status_ = WffsUpdateItemList(
					backend_from, ctx->listbox[WFFS_DIR_FROM], dlg,
					&ctx->itemc[WFFS_DIR_FROM], &ctx->itemv[WFFS_DIR_FROM],
					update_fn, true))
			&&  WFR_SUCCESS(status_ = WffsUpdateItemList(
					backend_to, ctx->listbox[WFFS_DIR_TO], dlg,
					&ctx->itemc[WFFS_DIR_TO], &ctx->itemv[WFFS_DIR_TO],
					update_fn, true)))
			{
				status_ = WFR_STATUS_CONDITION_SUCCESS;
			}
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status_, "deleting item %s", name);
		} else {
			status_ = WffsUpdateItemList(
				backend, ctx->listbox[dir], dlg,
				&ctx->itemc[dir], &ctx->itemv[dir],
				update_fn, true);
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status_, "deleting item %s", name);
		}
	} else if (nitem == -1) {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "deleting item %s", name);

	return status;
}

WfrStatus
WffsExportItem(
	dlgcontrol *		ctrl,
	WffsContext *		ctx,
	dlgparam *		dlg,
	WffsExportItemFn	export_item_fn,
	WffsUpdateItemListFn	update_fn
	)
{
	WfsBackend	backend_from, backend_to;
	bool		movefl;
	char *		name;
	int		nitem;
	WfrStatus	status;


	backend_from = WFFS_GET_BACKEND(ctx, WFFS_DIR_FROM, dlg);
	backend_to = WFFS_GET_BACKEND(ctx, WFFS_DIR_TO, dlg);

	if (backend_from != backend_to) {
		movefl = ((ctrl == ctx->button_copy) ? false : true);

		status = WFR_STATUS_CONDITION_SUCCESS;
		for (int index = 0; (size_t)index < ctx->itemc[WFFS_DIR_FROM]; index++) {
			if (dlg_listbox_issel(ctx->listbox[WFFS_DIR_FROM], dlg, index)) {
				nitem = dlg_listbox_getid(ctx->listbox[WFFS_DIR_FROM], dlg, index);
				name = ctx->itemv[WFFS_DIR_FROM][nitem];

				status = export_item_fn(backend_from, backend_to, movefl, name);
				if (WFR_FAILURE(status)) {
					WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "exporting item %s", name);
					status = WFR_STATUS_CONDITION_SUCCESS;
				}
			}
		}

		if (WFR_SUCCESS(status)) {
			if (movefl) {
				status = WffsUpdateItemList(
					backend_from, ctx->listbox[WFFS_DIR_FROM], dlg,
					&ctx->itemc[WFFS_DIR_FROM], &ctx->itemv[WFFS_DIR_FROM],
					update_fn, true);

			}
			if (WFR_SUCCESS(status)) {
				status = WffsUpdateItemList(
					backend_to, ctx->listbox[WFFS_DIR_TO], dlg,
					&ctx->itemc[WFFS_DIR_TO], &ctx->itemv[WFFS_DIR_TO],
					update_fn, true);
			}
		}

		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "exporting item");
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WffsInitPanel(
	struct controlbox *	b,
	handler_fn		handler,
	const char *		name_export,
	const char *		name_from,
	const char *		name_to,
	const char *		path,
	const char *		title
	)
{
	WffsContext *		ctx;
	struct controlset *	s;
	WfrStatus		status;


	if (!WFR_NEW(status, ctx, WffsContext)) {
		return status;
	} else {
		WFFS_CONTEXT_INIT(*ctx);
		ctrl_settitle(b, path, title);
	}

	s = ctrl_getset(b, path, name_from, "From:");
	ctrl_columns(s, 2, 70, 30);
	ctx->editbox[WFFS_DIR_FROM] = ctrl_editbox(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, handler, P(ctx), P(NULL));
	ctx->editbox[WFFS_DIR_FROM]->column = 0;
	ctx->droplist[WFFS_DIR_FROM] = ctrl_droplist(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, handler, P(ctx));
	ctx->droplist[WFFS_DIR_FROM]->column = 1;
	/* Reset columns so that the buttons are alongside the list, rather
	 * than alongside that edit box. */
	ctrl_columns(s, 1, 100);
	ctrl_columns(s, 2, 75, 25);
	ctx->listbox[WFFS_DIR_FROM] = ctrl_listbox(s, NULL, NO_SHORTCUT, WFP_HELP_CTX, handler, P(ctx));
	ctx->listbox[WFFS_DIR_FROM]->column = 0;
	ctx->listbox[WFFS_DIR_FROM]->listbox.height = 6;
	ctx->listbox[WFFS_DIR_FROM]->listbox.multisel = 1;
	ctx->button_clear[WFFS_DIR_FROM] = ctrl_pushbutton(s, "Clear...", NO_SHORTCUT, WFP_HELP_CTX, handler, P(ctx));
	ctx->button_clear[WFFS_DIR_FROM]->column = 1;
	ctx->button_delete[WFFS_DIR_FROM] = ctrl_pushbutton(s, "Delete", NO_SHORTCUT, WFP_HELP_CTX, handler, P(ctx));
	ctx->button_delete[WFFS_DIR_FROM]->column = 1;
	ctx->button_rename[WFFS_DIR_FROM] = ctrl_pushbutton(s, "Rename", NO_SHORTCUT, WFP_HELP_CTX, handler, P(ctx));
	ctx->button_rename[WFFS_DIR_FROM]->column = 1;
	ctrl_columns(s, 1, 100);

	s = ctrl_getset(b, path, name_export, "Export from/to:");
	ctrl_columns(s, 2, 50, 50);
	ctx->button_copy = ctrl_pushbutton(s, "Copy", 'p', WFP_HELP_CTX, handler, P(ctx));
	ctx->button_copy->column = 0;
	ctx->button_move = ctrl_pushbutton(s, "Move", 'm', WFP_HELP_CTX, handler, P(ctx));
	ctx->button_move->column = 1;
	ctrl_columns(s, 1, 100);

	s = ctrl_getset(b, path, name_to, "To:");
	ctrl_columns(s, 2, 70, 30);
	ctx->editbox[WFFS_DIR_TO] = ctrl_editbox(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, handler, P(ctx), P(NULL));
	ctx->editbox[WFFS_DIR_TO]->column = 0;
	ctx->droplist[WFFS_DIR_TO] = ctrl_droplist(s, NULL, NO_SHORTCUT, 100, WFP_HELP_CTX, handler, P(ctx));
	ctx->droplist[WFFS_DIR_TO]->column = 1;
	/* Reset columns so that the buttons are alongside the list, rather
	 * than alongside that edit box. */
	ctrl_columns(s, 1, 100);
	ctrl_columns(s, 2, 75, 25);
	ctx->listbox[WFFS_DIR_TO] = ctrl_listbox(s, NULL, NO_SHORTCUT, WFP_HELP_CTX, handler, P(ctx));
	ctx->listbox[WFFS_DIR_TO]->column = 0;
	ctx->listbox[WFFS_DIR_TO]->listbox.height = 6;
	ctx->listbox[WFFS_DIR_TO]->listbox.multisel = 1;
	ctx->button_clear[WFFS_DIR_TO] = ctrl_pushbutton(s, "Clear...", NO_SHORTCUT, WFP_HELP_CTX, handler, P(ctx));
	ctx->button_clear[WFFS_DIR_TO]->column = 1;
	ctx->button_delete[WFFS_DIR_TO] = ctrl_pushbutton(s, "Delete", NO_SHORTCUT, WFP_HELP_CTX, handler, P(ctx));
	ctx->button_delete[WFFS_DIR_TO]->column = 1;
	ctx->button_rename[WFFS_DIR_TO] = ctrl_pushbutton(s, "Rename", NO_SHORTCUT, WFP_HELP_CTX, handler, P(ctx));
	ctx->button_rename[WFFS_DIR_TO]->column = 1;
	ctrl_columns(s, 1, 100);

	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WffsRenameItem(
	dlgcontrol *		ctrl,
	WffsContext *		ctx,
	dlgparam *		dlg,
	WffsRenameItemFn	rename_item_fn,
	WffsUpdateItemListFn	update_fn
	)
{
	WfsBackend	backend, backend_from, backend_to;
	WffsDirection	dir;
	char *		name = NULL, *name_new;
	int		nitem;
	WfrStatus	status;


	dir = ((ctrl == ctx->button_rename[WFFS_DIR_FROM]) ? WFFS_DIR_FROM : WFFS_DIR_TO);
	backend = WFFS_GET_BACKEND(ctx, dir, dlg);
	backend_from = WFFS_GET_BACKEND(ctx, WFFS_DIR_FROM, dlg);
	backend_to = WFFS_GET_BACKEND(ctx, WFFS_DIR_TO, dlg);
	name_new = dlg_editbox_get(ctx->editbox[dir], dlg);

	if (WFR_SUCCESS(status = WFFS_GET_ITEM(ctx, dir, dlg, nitem))) {
		name = ctx->itemv[dir][nitem];

		status = rename_item_fn(backend, true, name, name_new);
		if (WFR_FAILURE(status)) {
			WFR_FREE(name_new);
		} else {
			if (backend_from == backend_to) {
				if (WFR_SUCCESS(status = WffsUpdateItemList(
						backend_from, ctx->listbox[WFFS_DIR_FROM], dlg,
						&ctx->itemc[WFFS_DIR_FROM], &ctx->itemv[WFFS_DIR_FROM],
						update_fn, true))
				&&  WFR_SUCCESS(status = WffsUpdateItemList(
						backend_to, ctx->listbox[WFFS_DIR_TO], dlg,
						&ctx->itemc[WFFS_DIR_TO], &ctx->itemv[WFFS_DIR_TO],
						update_fn, true)))
				{
					status = WFR_STATUS_CONDITION_SUCCESS;
				}
			} else {
				status = WffsUpdateItemList(
					backend, ctx->listbox[dir], dlg,
					&ctx->itemc[dir], &ctx->itemv[dir],
					update_fn, true);
			}
		}
	} else {
		if (nitem == -1) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
		WFR_FREE(name_new);
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "renaming item %s", name);

	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WffsRefreshBackends(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	bool		fromfl
	)
{
	WfsBackend	backend;
	const char *	backend_name;
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	dlg_listbox_clear(ctrl, dlg);

	backend = WFS_BACKEND_MIN;
	do {
		if (WFR_SUCCESS(status = WfsGetBackendName(backend, &backend_name))) {
			dlg_listbox_addwithid(ctrl, dlg, backend_name, backend);
		}
	} while (WfsGetBackendNext(&backend));

	if (fromfl) {
		dlg_listbox_select(ctrl, dlg, WFS_BACKEND_REGISTRY);
	} else {
		dlg_listbox_select(ctrl, dlg, WFS_BACKEND_FILE);
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "refreshing backends");

	return status;
}

WfrStatus
WffsRefreshItems(
	dlgcontrol *		ctrl,
	WffsContext *		ctx,
	dlgparam *		dlg,
	WffsUpdateItemListFn	update_fn
	)
{
	WfsBackend	backend, backend_from, backend_to;
	WffsDirection	dir;
	WfrStatus	status;


	if ((ctrl == ctx->droplist[WFFS_DIR_FROM])
	||  (ctrl == ctx->droplist[WFFS_DIR_TO]))
	{
		dlg_update_start(ctrl, dlg);

		if (WFR_SUCCESS(status = WffsRefreshBackends(
				ctrl, dlg, (ctrl == ctx->droplist[WFFS_DIR_FROM]))))
		{
			dir = ((ctrl == ctx->droplist[WFFS_DIR_FROM]) ? WFFS_DIR_FROM : WFFS_DIR_TO);
			backend = WFFS_GET_BACKEND(ctx, dir, dlg);
			status = WffsUpdateItemList(
				backend, ctx->listbox[dir], dlg,
				&ctx->itemc[dir], &ctx->itemv[dir],
				update_fn, true);
		}

		dlg_update_done(ctrl, dlg);
	} else if (ctrl == ctx->listbox[WFFS_DIR_FROM]) {
		backend_from = WFFS_GET_BACKEND(ctx, WFFS_DIR_FROM, dlg);
		status = WffsUpdateItemList(
			backend_from, ctx->listbox[WFFS_DIR_FROM], dlg,
			&ctx->itemc[WFFS_DIR_FROM], &ctx->itemv[WFFS_DIR_FROM],
			update_fn, true);
	} else if (ctrl == ctx->listbox[WFFS_DIR_TO]) {
		backend_to = WFFS_GET_BACKEND(ctx, WFFS_DIR_TO, dlg);
		status = WffsUpdateItemList(
			backend_to, ctx->listbox[WFFS_DIR_TO], dlg,
			&ctx->itemc[WFFS_DIR_TO], &ctx->itemv[WFFS_DIR_TO],
			update_fn, true);
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "refreshing item list");

	return status;
}

WfrStatus
WffsSelectItem(
	dlgcontrol *		ctrl,
	WffsContext *		ctx,
	dlgparam *		dlg,
	WffsUpdateItemListFn	update_fn
	)
{
	WfsBackend	backend_from, backend_to;
	WffsDirection	dir;
	int		nitem;
	WfrStatus	status;


	if (ctrl == ctx->droplist[WFFS_DIR_FROM]) {
		backend_from = WFFS_GET_BACKEND(ctx, WFFS_DIR_FROM, dlg);
		status = WffsUpdateItemList(
			backend_from, ctx->listbox[WFFS_DIR_FROM], dlg,
			&ctx->itemc[WFFS_DIR_FROM], &ctx->itemv[WFFS_DIR_FROM],
			update_fn, true);
	} else if (ctrl == ctx->droplist[WFFS_DIR_TO]) {
		backend_to = WFFS_GET_BACKEND(ctx, WFFS_DIR_TO, dlg);
		status = WffsUpdateItemList(
			backend_to, ctx->listbox[WFFS_DIR_TO], dlg,
			&ctx->itemc[WFFS_DIR_TO], &ctx->itemv[WFFS_DIR_TO],
			update_fn, true);
	} else if ((ctrl == ctx->listbox[WFFS_DIR_FROM])
		|| (ctrl == ctx->listbox[WFFS_DIR_TO]))
	{
		dir = ((ctrl == ctx->listbox[WFFS_DIR_FROM]) ? WFFS_DIR_FROM : WFFS_DIR_TO);

		nitem = dlg_listbox_getid(ctrl, dlg, dlg_listbox_index(ctrl, dlg));
		if (nitem >= 0) {
			if (WFR_SUCCESS(status = WFFS_GET_ITEM(ctx, dir, dlg, nitem))) {
				dlg_editbox_set(ctx->editbox[dir], dlg, ctx->itemv[dir][nitem]);
			}
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "selecting item");

	return status;
}

WfrStatus
WffsUpdateItemList(
	WfsBackend		backend,
	dlgcontrol *		ctrl,
	dlgparam *		dlg,
	size_t *		pitemc,
	char ***		pitemv,
	WffsUpdateItemListFn	update_fn,
	bool			update_listbox
	)
{
	bool		donefl;
	void *		enum_state;
	char *		item_name;
	size_t		itemc_new = 0;
	char **		itemv_new = NULL;
	WfrStatus	status;


	donefl = false;

	if (WFR_SUCCESS(status = update_fn(
			backend, false, true,
			NULL, NULL, &enum_state)))
	{
		do {
			status = update_fn(
				backend, false, false,
				&donefl, &item_name, &enum_state);

			if (WFR_SUCCESS(status) && !donefl) {
				if (WFR_SUCCESS(status = WFR_RESIZE(
						itemv_new, itemc_new, itemc_new + 1, char *)))
				{
					itemv_new[itemc_new - 1] = item_name;
				}
			}
		} while (WFR_SUCCESS(status) && !donefl);

		if (WFR_SUCCESS(status)) {
			for (size_t nitem = 0; nitem < (*pitemc); nitem++) {
				WFR_FREE_IF_NOTNULL((*pitemv)[nitem]);
			}
			WFR_FREE_IF_NOTNULL((*pitemv));

			(*pitemc) = itemc_new;
			(*pitemv) = itemv_new;

			if (update_listbox) {
				dlg_update_start(ctrl, dlg);
				dlg_listbox_clear(ctrl, dlg);

				for (size_t nitem = 0; nitem < itemc_new; nitem++) {
					dlg_listbox_addwithid(ctrl, dlg, itemv_new[nitem], nitem);
				}

				dlg_update_done(ctrl, dlg);
			}
		} else {
			if (itemv_new) {
				for (size_t nitem = 0; nitem < itemc_new; nitem++) {
					WFR_FREE_IF_NOTNULL(itemv_new[nitem]);
				}
				WFR_FREE(itemv_new);
			}
		}
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
