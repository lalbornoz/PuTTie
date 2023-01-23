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
 * Storage session type definitions
 * (encompasses session key tree, time of last modification, and session name)
 */

typedef struct WfspSession {
	tree234 *	tree;
	__time64_t	mtime;
	const char *	name;
} WfspSession;
#define WFSP_SESSION_EMPTY {		\
	.tree = NULL,			\
	.mtime = 0,			\
	.name = NULL,			\
}
#define WFSP_SESSION_INIT(session)	\
	(session) = (WfspSession)WFSP_SESSION_EMPTY

/*
 * Storage tree and tree item type and descriptor definitions
 */

typedef tree234 WfspTree;

typedef enum WfspTreeItemType {
	WFSP_TREE_ITYPE_NONE		= 0,
	WFSP_TREE_ITYPE_ANY		= 1,
	WFSP_TREE_ITYPE_HOST_KEY	= 2,
	WFSP_TREE_ITYPE_INT		= 3,
	WFSP_TREE_ITYPE_SESSION		= 4,
	WFSP_TREE_ITYPE_STRING		= 5,
	WFSP_TREE_ITYPE_MAX		= WFSP_TREE_ITYPE_STRING,
} WfspTreeItemType;

typedef struct WfspTreeItem {
	char *				key;
	WfspTreeItemType		type;
	void *				value;
	size_t				value_size;
} WfspTreeItem;
#define WFSP_TREE_ITEM_EMPTY {			\
	.key = NULL,				\
	.type = WFSP_TREE_ITYPE_NONE,		\
	.value = NULL,				\
	.value_size = 0,			\
}
#define WFSP_TREE_ITEM_INIT(tree_item)		\
	(tree_item) = (WfspTreeItem)WFSP_TREE_ITEM_EMPTY

/*
 * Public session storage tree subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfrStatus	WfspTreeClear(WfspTree **tree);
WfrStatus	WfspTreeCloneValue(WfspTreeItem *item, void **pvalue_new);
WfrStatus	WfspTreeCopy(WfspTree *tree_from, WfspTree *tree_to);
WfrStatus	WfspTreeDelete(WfspTree *tree, WfspTreeItem *item, const char *key, WfspTreeItemType type);
WfrStatus	WfspTreeEnumerate(WfspTree *tree, bool initfl, bool *pdonefl, WfspTreeItem **pitem, void *state);
WfrStatus	WfspTreeGet(WfspTree *tree, const char *key, WfspTreeItemType type, WfspTreeItem **pitem);
WfrStatus	WfspTreeInit(WfspTree **tree);
WfrStatus	WfspTreeRename(WfspTree *tree, WfspTreeItem *item, const char *key, WfspTreeItemType type, const char *key_new);
WfrStatus	WfspTreeSet(WfspTree *tree, const char *key, WfspTreeItemType type, void *value, size_t value_size);

#endif // !PUTTY_WINFRIP_STORAGE_TREE_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
