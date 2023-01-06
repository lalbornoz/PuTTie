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

static int		WfsppTree234Cmp(void *e1, void *e2);
static void		WfsppTreeFreeItem(WfspTreeItem *item);

/*
 * Private subroutines
 */

static int
WfsppTree234Cmp(
	void *	e1,
	void *	e2
	)
{
	int				cmp;
	WfspTreeItem *	item1, *item2;


	item1 = e1; item2 = e2;
	switch (item1->type) {
	default:
		WFR_DEBUG_FAIL();
		return -1;

	case WFSP_TREE_ITYPE_ANY:
	case WFSP_TREE_ITYPE_HOST_KEY:
	case WFSP_TREE_ITYPE_INT:
	case WFSP_TREE_ITYPE_SESSION:
	case WFSP_TREE_ITYPE_STRING:
		cmp = strcmp((char *)item1->key, (char *)item2->key);
		break;
	}

	return cmp;
}

static void
WfsppTreeFreeItem(
	WfspTreeItem *	item
	)
{
	WfspSession *	session;


	switch (item->type) {
	default:
		WFR_DEBUG_FAIL(); break;

	case WFSP_TREE_ITYPE_HOST_KEY:
	case WFSP_TREE_ITYPE_INT:
	case WFSP_TREE_ITYPE_STRING:
		WFR_SFREE_IF_NOTNULL(item->value);
		item->value = NULL;
		break;

	case WFSP_TREE_ITYPE_SESSION:
		session = (WfspSession *)item->value;
		if (WFR_STATUS_FAILURE(WfspTreeClear(&session->tree))) {
			WFR_DEBUG_FAIL();
		}
		WFR_SFREE_IF_NOTNULL((void *)session->name);
		WFR_SFREE_IF_NOTNULL(session->tree);

		WFR_SFREE_IF_NOTNULL(item->value);
		item->value = NULL;
		break;
	}

	WFR_SFREE_IF_NOTNULL(item->key);
	item->key = NULL;
}

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfspTreeClear(
	WfspTree **		tree
	)
{
	WfspTreeItem *	item;
	WfrStatus		status = WFR_STATUS_CONDITION_SUCCESS;


	WFSP_TREE234_FOREACH(status, *tree, idx, item) {
		WfsppTreeFreeItem(item);
		sfree(item);
	}
	freetree234(*tree);
	*tree = newtree234(WfsppTree234Cmp);

	return status;
}

WfrStatus
WfspTreeCloneValue(
	WfspTreeItem *	item,
	void **			pvalue_new
	)
{
	WfrStatus	status;
	void *		value_new;


	switch (item->type) {
	default:
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
		break;

	case WFSP_TREE_ITYPE_INT:
		if ((value_new = snewn(item->value_size, uint8_t))) {
			*(int *)value_new = *(int *)item->value;
			*pvalue_new = value_new;
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else {
			status = WFR_STATUS_FROM_ERRNO();
		}
		break;

	case WFSP_TREE_ITYPE_HOST_KEY:
	case WFSP_TREE_ITYPE_STRING:
		if ((value_new = snewn(item->value_size, char))) {
			memcpy(value_new, item->value, item->value_size);
			*(char **)pvalue_new = (char *)value_new;
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else {
			status = WFR_STATUS_FROM_ERRNO();
		}
		break;
	}

	return status;
}

WfrStatus
WfspTreeCopy(
	WfspTree *	tree_from,
	WfspTree *	tree_to
	)
{
	WfspTreeItem *	item;
	WfrStatus		status;
	void *			value_new;


	WFSP_TREE234_FOREACH(status, tree_from, idx, item) {
		if (WFR_STATUS_SUCCESS(status = WfspTreeCloneValue(item, &value_new)))
		{
			status = WfspTreeSet(
						tree_to, item->key, item->type,
						value_new, item->value_size);
			if (WFR_STATUS_FAILURE(status)) {
				sfree(value_new);
			}
		}
	}

	return status;
}

WfrStatus
WfspTreeDelete(
	WfspTree *			tree,
	WfspTreeItem *		item,
	const char *		key,
	WfspTreeItemType	type
	)
{
	WfrStatus	status;


	if (!item) {
		status = WfspTreeGet(tree, key, type, &item);
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_SUCCESS(status)) {
		del234(tree, item);
		WfsppTreeFreeItem(item);
		sfree(item);
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfspTreeEnumerate(
	WfspTree *			tree,
	bool				initfl,
	bool *				pdonefl,
	WfspTreeItem **		pitem,
	void *				state
	)
{
	WfspTreeItem *	item;
	WfrStatus		status;


	if (initfl) {
		if (!((*(int **)state) = snew(int))) {
			status = WFR_STATUS_FROM_ERRNO();
		} else {
			**(int **)state = 0;
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		return status;
	}

	status = WFR_STATUS_CONDITION_SUCCESS;
	if ((item = index234(tree, *(int *)state))) {
		*pdonefl = false;
		*pitem = item;
		(*(int *)state)++;
	} else {
		*pdonefl = true;
		*pitem = NULL;
		*(int *)state = 0;
	}

	return status;
}

WfrStatus
WfspTreeGet(
	WfspTree *			tree,
	const char *		key,
	WfspTreeItemType	type,
	WfspTreeItem **		pitem
	)
{
	WfspTreeItem *	item, item_find;
	WfrStatus		status;


	WFSP_TREE_ITEM_INIT(item_find);
	item_find.key = (char *)key;
	item_find.type = type;

	if (!(item = find234(tree, &item_find, WfsppTree234Cmp))) {
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);
	} else {
		*pitem = item;
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfspTreeInit(
	WfspTree **		tree
	)
{
	*tree = newtree234(WfsppTree234Cmp);

	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspTreeRename(
	WfspTree *			tree,
	WfspTreeItem *		item,
	const char *		key,
	WfspTreeItemType	type,
	const char *		key_new
	)
{
	WfspTreeItem *	item_old;
	char *			key_new_ = NULL;
	size_t			key_new__size;
	WfrStatus		status;


	if (type == WFSP_TREE_ITYPE_ANY) {
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
	} else {
		if (!item) {
			status = WfspTreeGet(tree, key, type, &item);
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	if (WFR_STATUS_SUCCESS(status)) {
		key_new__size = strlen(key_new) + 1;
		if (!(key_new_ = snewn(key_new__size, char))) {
			status = WFR_STATUS_FROM_ERRNO();
		} else {
			status = WfspTreeGet(tree, key_new, WFSP_TREE_ITYPE_ANY, &item_old);
			if (WFR_STATUS_SUCCESS(status)) {
				status = WfspTreeDelete(tree, item_old, NULL, item_old->type);
			} else if (WFR_STATUS_CONDITION(status) == ENOENT) {
				status = WFR_STATUS_CONDITION_SUCCESS;
			}

			if (WFR_STATUS_SUCCESS(status)) {
				(void)del234(tree, item);
				sfree(item->key); item->key = key_new_;
				(void)add234(tree, item);
				status = WFR_STATUS_CONDITION_SUCCESS;
			} else {
				sfree(key_new_);
			}
		}
	}

	return status;
}

WfrStatus
WfspTreeSet(
	WfspTree *			tree,
	const char *		key,
	WfspTreeItemType	type,
	void *				value,
	size_t				value_size
	)
{
	WfspTreeItem *	item = NULL, *item_old;
	char *			key_new = NULL;
	WfrStatus		status;


	if (type == WFSP_TREE_ITYPE_ANY) {
		return WFR_STATUS_FROM_ERRNO1(EINVAL);
	}

	if ((item = snew(WfspTreeItem))
	&&  (key_new = snewn(strlen(key) + 1, char)))
	{
		WFSP_TREE_ITEM_INIT(*item);
		strcpy(key_new, key);
		item->key = key_new;
		item->type = type;
		item->value = value;
		item->value_size = value_size;

		status = WfspTreeGet(tree, key, WFSP_TREE_ITYPE_ANY, &item_old);
		if (WFR_STATUS_SUCCESS(status)) {
			status = WfspTreeDelete(tree, item_old, NULL, item_old->type);
		} else if (WFR_STATUS_CONDITION(status) == ENOENT) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		if (WFR_STATUS_SUCCESS(status)) {
			(void)add234(tree, item);
		} else {
			WfsppTreeFreeItem(item);
			sfree(item);
		}
	} else {
		WFR_SFREE_IF_NOTNULL(item);
		WFR_SFREE_IF_NOTNULL(key_new);
		status = WFR_STATUS_FROM_ERRNO();
	}

	return status;
}

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
