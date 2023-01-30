/*
 * winfrip_storage_priv.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "PuTTie/winfrip_storage_wrap.h"
#include "storage.h"
#include "PuTTie/winfrip_storage_unwrap.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_debug.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"
#include "PuTTie/winfrip_storage_backend_ephemeral.h"
#include "PuTTie/winfrip_storage_backend_file.h"
#include "PuTTie/winfrip_storage_backend_registry.h"

/*
 * Private variables
 */

/*
 * Storage backends for the ephemeral, file, and registry backends
 */

static WfspBackend	WfspBackends[WFS_BACKEND_MAX + 1] = {
	[WFS_BACKEND_EPHEMERAL]		= WFSP_EPHEMERAL_BACKEND,
	[WFS_BACKEND_FILE]		= WFSP_FILE_BACKEND,
	[WFS_BACKEND_REGISTRY]		= WFSP_REGISTRY_BACKEND,
};

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfsGetBackendImpl(
	WfsBackend	backend,
	void *		pbackend
	)
{
	WfrStatus	status;


	if (((backend) < WFS_BACKEND_MIN)
	||  ((backend) > WFS_BACKEND_MAX)) {
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
	} else {
		*((WfspBackend **)pbackend) = &WfspBackends[(backend)];
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfsTransformJumpList(
	bool			addfl,
	bool			delfl,
	char **			pjump_list,
	size_t *		pjump_list_size,
	const char *const	trans_item
	)
{
	size_t		item_len;
	char *		jump_list_new = NULL, *jump_list_new_last;
	ptrdiff_t	jump_list_new_delta;
	size_t		jump_list_new_size = 0;
	WfrStatus	status;
	size_t		trans_item_len;


	if (addfl || delfl) {
		trans_item_len = strlen(trans_item);

		if (*pjump_list == NULL) {
			if (!(*pjump_list = WFR_NEWN(2, char))) {
				status = WFR_STATUS_FROM_ERRNO();
			} else {
				(*pjump_list)[0] = '\0'; (*pjump_list)[1] = '\0';
				*pjump_list_size = 1;
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		}

		jump_list_new_size = trans_item_len + 1 + *pjump_list_size;
		if (!(jump_list_new = WFR_NEWN(jump_list_new_size, char))) {
			status = WFR_STATUS_FROM_ERRNO();
		} else {
			memset(jump_list_new, '\0', jump_list_new_size);
			jump_list_new_last = jump_list_new;

			if (addfl) {
				memcpy(jump_list_new_last, trans_item, trans_item_len + 1);
				jump_list_new_last += trans_item_len + 1;
			}

			for (char *item = *pjump_list, *item_next = NULL;
			     item && *item; item = item_next)
			{
				if ((item_next = strchr(item, '\0'))) {
					item_len = item_next - item;
					item_next++;

					if ((trans_item_len != item_len)
					||  (strncmp(trans_item, item, item_len) != 0))
					{
						memcpy(jump_list_new_last, item, item_len);
						jump_list_new_last += item_len + 1;
					}
				}
			}

			if (&jump_list_new_last[0] < &jump_list_new[jump_list_new_size - 1]) {
				jump_list_new_delta = (&jump_list_new[jump_list_new_size - 1] - &jump_list_new_last[0]);
				status = WFR_RESIZE(
					jump_list_new, jump_list_new_size,
					jump_list_new_size - jump_list_new_delta, char);
			} else {
				status = WFR_STATUS_CONDITION_SUCCESS;
			}

			if (WFR_STATUS_SUCCESS(status)) {
				WFR_FREE(*pjump_list);
				*pjump_list = jump_list_new;
				*pjump_list_size = jump_list_new_size;
			}
		}
	}

	if (WFR_STATUS_FAILURE(status)) {
		WFR_FREE_IF_NOTNULL(jump_list_new);
	}

	return status;
}


WfrStatus
WfsTreeCloneValue(
	WfsTreeItem *	item,
	void **		pvalue_new
	)
{
	WfrStatus	status;
	void *		value_new;


	switch (item->type) {
	default:
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
		break;

	case WFS_TREE_ITYPE_INT:
		if ((value_new = WFR_NEWN(item->value_size, uint8_t))) {
			*(int *)value_new = *(int *)item->value;
			*pvalue_new = value_new;
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else {
			status = WFR_STATUS_FROM_ERRNO();
		}
		break;

	case WFS_TREE_ITYPE_HOST_KEY:
	case WFS_TREE_ITYPE_STRING:
		if ((value_new = WFR_NEWN(item->value_size, char))) {
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

void
WfsTreeFreeItem(
	WfsTreeItem *	item
	)
{
	WfsHostCA *	hca;
	WfsSession *	session;


	switch (item->type) {
	default:
		WFR_DEBUG_FAIL(); break;

	case WFS_TREE_ITYPE_HOST_CA:
		hca = (WfsHostCA *)item->value;
		WFR_FREE_IF_NOTNULL(hca->name);
		WFR_FREE_IF_NOTNULL(hca->public_key);
		WFR_FREE_IF_NOTNULL(hca->validity);
		WFR_FREE_IF_NOTNULL(item->value);
		item->value = NULL;
		break;

	case WFS_TREE_ITYPE_HOST_KEY:
	case WFS_TREE_ITYPE_INT:
	case WFS_TREE_ITYPE_STRING:
		WFR_FREE_IF_NOTNULL(item->value);
		item->value = NULL;
		break;

	case WFS_TREE_ITYPE_SESSION:
		session = (WfsSession *)item->value;
		if (WFR_STATUS_FAILURE(WfsTreeClear(&session->tree, WfsTreeFreeItem))) {
			WFR_DEBUG_FAIL();
		}
		WFR_FREE_IF_NOTNULL(session->name);
		WFR_FREE_IF_NOTNULL(session->tree);

		WFR_FREE_IF_NOTNULL(item->value);
		item->value = NULL;
		break;
	}

	WFR_FREE_IF_NOTNULL(item->key);
	item->key = NULL;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */