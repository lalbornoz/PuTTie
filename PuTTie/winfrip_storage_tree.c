/*
 * winfrip_storage_tree.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#pragma GCC diagnostic pop

#ifdef _WINDOWS
#include "windows/platform.h"
#endif // _WINDOWS

#include <errno.h>
#include <time.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_storage.h"

/*
 * Private subroutine prototypes
 */

static int	WfspTree234Cmp(void *e1, void *e2);

/*
 * Private subroutines
 */

static int
WfspTree234Cmp(
	void *	e1,
	void *	e2
	)
{
	WfsTreeItem *	item1, *item2;


	item1 = e1; item2 = e2;
	return strcmp((char *)item1->key, (char *)item2->key);
}

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfsTreeClear(
	WfsTree **		tree,
	WfsTreeFreeItemFn	free_item_fn
	)
{
	WfsTreeItem *	item;
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	while ((item = delpos234(*tree, 0)) != NULL) {
		free_item_fn(item);
		WFR_FREE(item);
	}
	freetree234(*tree);
	*tree = newtree234(WfspTree234Cmp);

	return status;
}

WfrStatus
WfsTreeCopy(
	WfsTree *		tree_from,
	WfsTree *		tree_to,
	WfsTreeCloneValueFn	clone_value_fn,
	WfsTreeFreeItemFn	free_item_fn
	)
{
	WfsTreeItem *	item;
	WfrStatus	status;
	void *		value_new;


	WFS_TREE234_FOREACH(status, tree_from, idx, item) {
		if (WFR_STATUS_SUCCESS(status = clone_value_fn(item, &value_new)))
		{
			status = WfsTreeSet(
				tree_to, item->key, item->type,
				value_new, item->value_size,
				free_item_fn);
			if (WFR_STATUS_FAILURE(status)) {
				WFR_FREE(value_new);
			}
		}
	}

	return status;
}

WfrStatus
WfsTreeDelete(
	WfsTree *		tree,
	WfsTreeItem *		item,
	const char *		key,
	WfsTreeItemTypeBase	type,
	WfsTreeFreeItemFn	free_item_fn
	)
{
	WfrStatus	status;


	if (!item) {
		status = WfsTreeGet(tree, key, type, &item);
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_SUCCESS(status)) {
		del234(tree, item);
		free_item_fn(item);
		WFR_FREE(item);
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfsTreeEnumerate(
	WfsTree *		tree,
	bool			initfl,
	bool *			pdonefl,
	WfsTreeItem **		pitem,
	void **			pstate
	)
{
	WfsTreeItem *	item;
	WfrStatus	status;


	if (initfl) {
		if (!((*(int **)pstate) = WFR_NEW(int))) {
			status = WFR_STATUS_FROM_ERRNO();
		} else {
			**(int **)pstate = 0;
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		return status;
	}

	status = WFR_STATUS_CONDITION_SUCCESS;
	if ((item = index234(tree, **(int **)pstate))) {
		*pdonefl = false;
		*pitem = item;
		(**(int **)pstate)++;
	} else {
		*pdonefl = true;
		*pitem = NULL;
		**(int **)pstate = 0;
	}

	return status;
}

void
WfsTreeEnumerateCancel(
	void **		pstate
	)
{
	if (pstate) {
		WFR_FREE(*(int **)pstate);
		*(int **)pstate = NULL;
	}
}

WfrStatus
WfsTreeGet(
	WfsTree *		tree,
	const char *		key,
	WfsTreeItemTypeBase	type,
	WfsTreeItem **		pitem
	)
{
	WfsTreeItem *	item, item_find;
	WfrStatus	status;


	WFS_TREE_ITEM_INIT(item_find);
	item_find.key = (char *)key;
	item_find.type = type;

	if (!(item = find234(tree, &item_find, WfspTree234Cmp))) {
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);
	} else {
		*pitem = item;
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfsTreeInit(
	WfsTree **	tree
	)
{
	*tree = newtree234(WfspTree234Cmp);

	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfsTreeRename(
	WfsTree *		tree,
	WfsTreeItem *		item,
	const char *		key,
	WfsTreeItemTypeBase	type,
	const char *		key_new,
	WfsTreeFreeItemFn	free_item_fn
	)
{
	WfsTreeItem *	item_old;
	char *		key_new_ = NULL;
	size_t		key_new__size;
	WfrStatus	status;


	if (type == WFS_TREE_ITYPE_ANY) {
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
	} else {
		if (!item) {
			status = WfsTreeGet(tree, key, type, &item);
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	if (WFR_STATUS_SUCCESS(status)) {
		key_new__size = strlen(key_new) + 1;
		if (!(key_new_ = WFR_NEWN(key_new__size, char))) {
			status = WFR_STATUS_FROM_ERRNO();
		} else {
			status = WfsTreeGet(tree, key_new, -1, &item_old);
			if (WFR_STATUS_SUCCESS(status)) {
				status = WfsTreeDelete(tree, item_old, NULL, item_old->type, free_item_fn);
			} else if (WFR_STATUS_CONDITION(status) == ENOENT) {
				status = WFR_STATUS_CONDITION_SUCCESS;
			}

			if (WFR_STATUS_SUCCESS(status)) {
				(void)del234(tree, item);
				WFR_FREE(item->key); item->key = key_new_;
				(void)add234(tree, item);
				status = WFR_STATUS_CONDITION_SUCCESS;
			} else {
				WFR_FREE(key_new_);
			}
		}
	}

	return status;
}

WfrStatus
WfsTreeSet(
	WfsTree *		tree,
	const char *		key,
	WfsTreeItemTypeBase	type,
	void *			value,
	size_t			value_size,
	WfsTreeFreeItemFn	free_item_fn
	)
{
	WfsTreeItem *	item = NULL, *item_old;
	char *		key_new = NULL;
	WfrStatus	status;


	if (type == WFS_TREE_ITYPE_ANY) {
		return WFR_STATUS_FROM_ERRNO1(EINVAL);
	}

	if ((item = WFR_NEW(WfsTreeItem))
	&&  (key_new = WFR_NEWN(strlen(key) + 1, char)))
	{
		WFS_TREE_ITEM_INIT(*item);
		strcpy(key_new, key);
		item->key = key_new;
		item->type = type;
		item->value = value;
		item->value_size = value_size;

		status = WfsTreeGet(tree, key, -1, &item_old);
		if (WFR_STATUS_SUCCESS(status)) {
			status = WfsTreeDelete(tree, item_old, NULL, item_old->type, free_item_fn);
		} else if (WFR_STATUS_CONDITION(status) == ENOENT) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		if (WFR_STATUS_SUCCESS(status)) {
			(void)add234(tree, item);
		} else {
			free_item_fn(item);
			WFR_FREE(item);
		}
	} else {
		WFR_FREE_IF_NOTNULL(item);
		WFR_FREE_IF_NOTNULL(key_new);
		status = WFR_STATUS_FROM_ERRNO();
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
