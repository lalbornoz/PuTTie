/*
 * winfrip_storage_tree.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_TREE_H
#define PUTTY_WINFRIP_STORAGE_TREE_H

#include "tree234.h"

/*
 * Public type definitions private to PuTTie/winfrip_storage*.c
 */

/*
 * Storage tree, tree item type, and tree descriptor definitions
 */

typedef tree234			WfsTree;
typedef int			WfsTreeItemTypeBase;
#define WFS_TREE_ITYPE_ANY	-1
#define WFS_TREE_ITYPE_NONE	0

typedef struct WfsTreeItem {
	char *			key;
	WfsTreeItemTypeBase	type;
	void *			value;
	size_t			value_size;
} WfsTreeItem;
#define WFS_TREE_ITEM_EMPTY {		\
	.key = NULL,			\
	.type = 0,			\
	.value = NULL,			\
	.value_size = 0,		\
}
#define WFS_TREE_ITEM_INIT(tree_item)	\
	(tree_item) = (WfsTreeItem)WFS_TREE_ITEM_EMPTY

/*
 * Storage tree clone value and free item function pointer types
 */

typedef WfrStatus (*WfsTreeCloneValueFn)(WfsTreeItem *, void **);
typedef void (*WfsTreeFreeItemFn)(WfsTreeItem *);

/*
 * Public session storage tree subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfrStatus	WfsTreeClear(WfsTree **tree, WfsTreeFreeItemFn free_item_fn);
WfrStatus	WfsTreeCloneValue(WfsTreeItem *item, void **pvalue_new);
WfrStatus	WfsTreeCopy(WfsTree *tree_from, WfsTree *tree_to, WfsTreeCloneValueFn clone_value_fn, WfsTreeFreeItemFn free_item_fn);
WfrStatus	WfsTreeDelete(WfsTree *tree, WfsTreeItem *item, const char *key, WfsTreeItemTypeBase type, WfsTreeFreeItemFn free_item_fn);
WfrStatus	WfsTreeEnumerate(WfsTree *tree, bool initfl, bool *pdonefl, WfsTreeItem **pitem, void *state);
WfrStatus	WfsTreeGet(WfsTree *tree, const char *key, WfsTreeItemTypeBase type, WfsTreeItem **pitem);
WfrStatus	WfsTreeInit(WfsTree **tree);
WfrStatus	WfsTreeRename(WfsTree *tree, WfsTreeItem *item, const char *key, WfsTreeItemTypeBase type, const char *key_new, WfsTreeFreeItemFn free_item_fn);
WfrStatus	WfsTreeSet(WfsTree *tree, const char *key, WfsTreeItemTypeBase type, void *value, size_t value_size, WfsTreeFreeItemFn free_item_fn);

#endif // !PUTTY_WINFRIP_STORAGE_TREE_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
