/*
 * winfrip_rtl_tree.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_TREE_H
#define PUTTY_WINFRIP_RTL_TREE_H

#include "tree234.h"

/*
 * Public macros private to PuTTie/winfrip*.c
 */

#define WFR_TREE234_FOREACH(status, tree, idx, item)	\
	for (int (idx) = 0; WFR_SUCCESS(status)		\
	  && ((item) = index234((tree), (idx))); (idx)++)

/*
 * Public type definitions private to PuTTie/winfrip*.c
 */

/*
 * Tree, tree item type, and tree descriptor definitions
 */

typedef tree234				WfrTree;
typedef int				WfrTreeItemTypeBase;
#define WFR_TREE_ITYPE_ANY		-1
#define WFR_TREE_ITYPE_NONE		0
#define WFR_TREE_ITYPE_INT		1
#define WFR_TREE_ITYPE_STRING		2
#define WFR_TREE_ITYPE_BASE_MAX		WFR_TREE_ITYPE_STRING

typedef struct WfrTreeItem {
	char *			key;
	WfrTreeItemTypeBase	type;
	void *			value;
	size_t			value_size;
} WfrTreeItem;
#define WFR_TREE_ITEM_EMPTY {					\
	.key = NULL,						\
	.type = 0,						\
	.value = NULL,						\
	.value_size = 0,					\
}
#define WFR_TREE_ITEM_INIT(tree_item) ({			\
	(tree_item) = (WfrTreeItem)WFR_TREE_ITEM_EMPTY;		\
	WFR_STATUS_CONDITION_SUCCESS;				\
})

/*
 * Tree clone value and free item function pointer types
 */

typedef WfrStatus (*WfrTreeCloneValueFn)(WfrTreeItem *, void **);
typedef void (*WfrTreeFreeItemFn)(WfrTreeItem *);

/*
 * Public tree subroutine prototypes private to PuTTie/winfrip*.c
 */

WfrStatus	WfrTreeClear(WfrTree **tree, WfrTreeFreeItemFn free_item_fn);
WfrStatus	WfrTreeCopy(WfrTree *tree_from, WfrTree *tree_to, WfrTreeCloneValueFn clone_value_fn, WfrTreeFreeItemFn free_item_fn);
WfrStatus	WfrTreeDelete(WfrTree *tree, WfrTreeItem *item, const char *key, WfrTreeItemTypeBase type, WfrTreeFreeItemFn free_item_fn);
WfrStatus	WfrTreeEnumerate(WfrTree *tree, bool initfl, bool *pdonefl, WfrTreeItem **pitem, void **pstate);
void		WfrTreeEnumerateCancel(void **pstate);
WfrStatus	WfrTreeGet(WfrTree *tree, const char *key, WfrTreeItemTypeBase type, WfrTreeItem **pitem);
WfrStatus	WfrTreeInit(WfrTree **tree);
WfrStatus	WfrTreeRename(WfrTree *tree, WfrTreeItem *item, const char *key, WfrTreeItemTypeBase type, const char *key_new, WfrTreeFreeItemFn free_item_fn);
WfrStatus	WfrTreeSet(WfrTree *tree, const char *key, WfrTreeItemTypeBase type, void *value, size_t value_size, WfrTreeFreeItemFn free_item_fn);

#endif // !PUTTY_WINFRIP_RTL_TREE_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
