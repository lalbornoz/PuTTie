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
WfsTreeCloneValue(
	WfrTreeItem *	item,
	void **		pvalue_new
	)
{
	WfrStatus	status;
	void *		value_new;


	switch (item->type) {
	default:
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
		break;

	case WFR_TREE_ITYPE_INT:
		if (WFR_NEWN(status, value_new, item->value_size, uint8_t)) {
			*(int *)value_new = *(int *)item->value;
			*pvalue_new = value_new;
		}
		break;

	case WFR_TREE_ITYPE_HOST_KEY:
	case WFR_TREE_ITYPE_STRING:
		if (WFR_NEWN(status, value_new, item->value_size, char)) {
			memcpy(value_new, item->value, item->value_size);
			*(char **)pvalue_new = (char *)value_new;
		}
		break;
	}

	return status;
}

void
WfsTreeFreeItem(
	WfrTreeItem *	item
	)
{
	WfsHostCA *	hca;
	WfsSession *	session;


	switch (item->type) {
	default:
		WFR_DEBUG_FAIL(); break;

	case WFR_TREE_ITYPE_HOST_CA:
		hca = (WfsHostCA *)item->value;
		WFR_FREE_IF_NOTNULL(hca->name);
		WFR_FREE_IF_NOTNULL(hca->public_key);
		WFR_FREE_IF_NOTNULL(hca->validity);
		WFR_FREE_IF_NOTNULL(item->value);
		item->value = NULL;
		break;

	case WFR_TREE_ITYPE_HOST_KEY:
	case WFR_TREE_ITYPE_INT:
	case WFR_TREE_ITYPE_STRING:
		WFR_FREE_IF_NOTNULL(item->value);
		item->value = NULL;
		break;

	case WFR_TREE_ITYPE_SESSION:
		session = (WfsSession *)item->value;
		if (WFR_FAILURE(WfrTreeClear(&session->tree, WfsTreeFreeItem))) {
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
