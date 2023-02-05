/*
 * winfrip_rtl_tree.c - pointless frippery & tremendous amounts of bloat
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

static int	WfrpTree234Cmp(void *e1, void *e2);

/*
 * Private subroutines
 */

static int
WfrpTree234Cmp(
	void *	e1,
	void *	e2
	)
{
	WfrTreeItem *	item1, *item2;


	item1 = e1; item2 = e2;
	return strcmp((char *)item1->key, (char *)item2->key);
}

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfrTreeClear(
	WfrTree **		tree,
	WfrTreeFreeItemFn	free_item_fn
	)
{
	WfrTreeItem *	item;
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	while ((item = delpos234(*tree, 0)) != NULL) {
		free_item_fn(item);
		WFR_FREE(item);
	}
	freetree234(*tree);
	*tree = newtree234(WfrpTree234Cmp);

	return status;
}

WfrStatus
WfrTreeCopy(
	WfrTree *		tree_from,
	WfrTree *		tree_to,
	WfrTreeCloneValueFn	clone_value_fn,
	WfrTreeFreeItemFn	free_item_fn
	)
{
	WfrTreeItem *	item;
	WfrStatus	status;
	void *		value_new;


	WFR_TREE234_FOREACH(status, tree_from, idx, item) {
		if (WFR_STATUS_SUCCESS(status = clone_value_fn(item, &value_new)))
		{
			status = WfrTreeSet(
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
WfrTreeDelete(
	WfrTree *		tree,
	WfrTreeItem *		item,
	const char *		key,
	WfrTreeItemTypeBase	type,
	WfrTreeFreeItemFn	free_item_fn
	)
{
	WfrStatus	status;


	if (!item) {
		status = WfrTreeGet(tree, key, type, &item);
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
WfrTreeEnumerate(
	WfrTree *		tree,
	bool			initfl,
	bool *			pdonefl,
	WfrTreeItem **		pitem,
	void **			pstate
	)
{
	WfrTreeItem *	item;
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
WfrTreeEnumerateCancel(
	void **		pstate
	)
{
	if (pstate) {
		WFR_FREE(*(int **)pstate);
		*(int **)pstate = NULL;
	}
}

WfrStatus
WfrTreeGet(
	WfrTree *		tree,
	const char *		key,
	WfrTreeItemTypeBase	type,
	WfrTreeItem **		pitem
	)
{
	WfrTreeItem *	item, item_find;
	WfrStatus	status;


	WFR_TREE_ITEM_INIT(item_find);
	item_find.key = (char *)key;
	item_find.type = type;

	if (!(item = find234(tree, &item_find, WfrpTree234Cmp))) {
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);
	} else {
		*pitem = item;
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfrTreeInit(
	WfrTree **	tree
	)
{
	*tree = newtree234(WfrpTree234Cmp);

	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfrTreeRename(
	WfrTree *		tree,
	WfrTreeItem *		item,
	const char *		key,
	WfrTreeItemTypeBase	type,
	const char *		key_new,
	WfrTreeFreeItemFn	free_item_fn
	)
{
	WfrTreeItem *	item_old;
	char *		key_new_ = NULL;
	size_t		key_new__size;
	WfrStatus	status;


	if (type == WFR_TREE_ITYPE_ANY) {
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
	} else {
		if (!item) {
			status = WfrTreeGet(tree, key, type, &item);
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	if (WFR_STATUS_SUCCESS(status)) {
		key_new__size = strlen(key_new) + 1;
		if (!(key_new_ = WFR_NEWN(key_new__size, char))) {
			status = WFR_STATUS_FROM_ERRNO();
		} else {
			status = WfrTreeGet(tree, key_new, -1, &item_old);
			if (WFR_STATUS_SUCCESS(status)) {
				status = WfrTreeDelete(tree, item_old, NULL, item_old->type, free_item_fn);
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
WfrTreeSet(
	WfrTree *		tree,
	const char *		key,
	WfrTreeItemTypeBase	type,
	void *			value,
	size_t			value_size,
	WfrTreeFreeItemFn	free_item_fn
	)
{
	WfrTreeItem *	item = NULL, *item_old;
	char *		key_new = NULL;
	WfrStatus	status;


	if (type == WFR_TREE_ITYPE_ANY) {
		return WFR_STATUS_FROM_ERRNO1(EINVAL);
	}

	if ((item = WFR_NEW(WfrTreeItem))
	&&  (key_new = WFR_NEWN(strlen(key) + 1, char)))
	{
		WFR_TREE_ITEM_INIT(*item);
		strcpy(key_new, key);
		item->key = key_new;
		item->type = type;
		item->value = value;
		item->value_size = value_size;

		status = WfrTreeGet(tree, key, -1, &item_old);
		if (WFR_STATUS_SUCCESS(status)) {
			status = WfrTreeDelete(tree, item_old, NULL, item_old->type, free_item_fn);
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