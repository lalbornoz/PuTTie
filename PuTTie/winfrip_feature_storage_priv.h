/*
 * winfrip_feature_storage_priv.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_FEATURE_STORAGE_PRIV_H
#define PUTTY_WINFRIP_FEATURE_STORAGE_PRIV_H

/*
 * Public type definitions private to PuTTie/winfrip_feature_storage*.c
 */

typedef enum WffsDirection {
	WFFS_DIR_FROM	= 0,
	WFFS_DIR_TO	= 1,
	WFFS_DIR_COUNT	= WFFS_DIR_TO + 1,
} WffsDirection;

typedef struct WffsContext {
	dlgcontrol *	droplist[WFFS_DIR_COUNT];
	dlgcontrol *	editbox[WFFS_DIR_COUNT];

	dlgcontrol *	listbox[WFFS_DIR_COUNT];

	dlgcontrol *	button_clear[WFFS_DIR_COUNT];
	dlgcontrol *	button_copy;
	dlgcontrol *	button_delete[WFFS_DIR_COUNT];
	dlgcontrol *	button_move;
	dlgcontrol *	button_rename[WFFS_DIR_COUNT];


	size_t		itemc[WFFS_DIR_COUNT];
	char **		itemv[WFFS_DIR_COUNT];
} WffsContext;
#define WFFS_CONTEXT_EMPTY {		\
	.droplist = {NULL, NULL},	\
	.editbox = {NULL, NULL},	\
	.listbox = {NULL, NULL},	\
					\
	.button_clear = {NULL, NULL},	\
	.button_copy = NULL,		\
	.button_delete = {NULL, NULL},	\
	.button_move = NULL,		\
	.button_rename = {NULL, NULL},	\
					\
	.itemc = {0, 0},		\
	.itemv = {NULL, NULL},		\
}
#define WFFS_CONTEXT_INIT(ctx)		\
	(ctx) = (WffsContext)WFFS_CONTEXT_EMPTY

/*
 * Public macros and subroutine prototypes private to PuTTie/winfrip_feature_storage*.c
 */

#define WFFS_GET_BACKEND(ctx, dir, dlg)				\
	(WfsBackend)dlg_listbox_getid(				\
		(ctx)->droplist[dir], (dlg),			\
		dlg_listbox_index((ctx)->droplist[dir], (dlg)));

#define WFFS_GET_ITEM(ctx, dir, dlg, nitem) ({			\
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

typedef WfrStatus	(*WffsClearItemsFn)(WfsBackend, bool);
typedef WfrStatus	(*WffsDeleteItemFn)(WfsBackend, bool, const char *);
typedef WfrStatus	(*WffsExportItemFn)(WfsBackend, WfsBackend, bool, char *);
typedef WfrStatus	(*WffsRenameItemFn)(WfsBackend, bool, const char *, const char *);
typedef WfrStatus	(*WffsUpdateItemListFn)(WfsBackend, bool, bool, bool *, char **, void *);

WfrStatus	WffsClearItems(WffsClearItemsFn clear_items_fn, dlgcontrol *ctrl, WffsContext *ctx, dlgparam *dlg, const char *items_title, WffsUpdateItemListFn update_fn);
WfrStatus	WffsDeleteItem(WffsDeleteItemFn delete_item_fn, dlgcontrol *ctrl, WffsContext *ctx, dlgparam * dlg, WffsUpdateItemListFn update_fn);
WfrStatus	WffsExportItem(dlgcontrol *ctrl, WffsContext *ctx, dlgparam *dlg, WffsExportItemFn export_item_fn, WffsUpdateItemListFn update_fn);
WfrStatus	WffsInitPanel(struct controlbox *b, handler_fn handler, const char *name_export, const char *name_from, const char *name_to, const char *path, const char *title);
WfrStatus	WffsRenameItem(dlgcontrol *ctrl, WffsContext *ctx, dlgparam *dlg, WffsRenameItemFn rename_item_fn, WffsUpdateItemListFn update_fn);
WfrStatus	WffsRefreshBackends(dlgcontrol *ctrl, dlgparam *dlg, bool fromfl);
WfrStatus	WffsRefreshItems(dlgcontrol *ctrl, WffsContext *ctx, dlgparam *dlg, WffsUpdateItemListFn update_fn);
WfrStatus	WffsSelectItem(dlgcontrol *ctrl, WffsContext *ctx, dlgparam *dlg, WffsUpdateItemListFn update_fn);
WfrStatus	WffsUpdateItemList(WfsBackend backend, dlgcontrol *ctrl, dlgparam *dlg, size_t *pitemc, char ***pitemv, WffsUpdateItemListFn update_fn, bool update_listbox);

#endif // !PUTTY_WINFRIP_FEATURE_STORAGE_PRIV_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
