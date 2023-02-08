/*
 * winfrip_storage_options.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_options.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"
#include "PuTTie/winfrip_storage_backend_ephemeral.h"
#include "PuTTie/winfrip_storage_backend_file.h"
#include "PuTTie/winfrip_storage_backend_registry.h"

/*
 * Public global options storage subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfsCleanupOptions(
	WfsBackend	backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = WfsClearOptions(backend, true);
	}

	return status;
}

WfrStatus
WfsClearOptions(
	WfsBackend	backend,
	bool		delete_in_backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (delete_in_backend) {
			status = backend_impl->ClearOptions(backend);
		}

		if (WFR_STATUS_IS_NOT_FOUND(status) && delete_in_backend) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		if (WFR_STATUS_SUCCESS(status)) {
			status = WfrTreeClear(&backend_impl->tree_options, WfsTreeFreeItem);
		}
	}

	return status;
}

WfrStatus
WfsCopyOption(
	WfsBackend	backend_from,
	WfsBackend	backend_to,
	bool		save_in_backend,
	const char *	key
	)
{
	WfspBackend *		backend_from_impl, *backend_to_impl;
	size_t			option_size;
	WfrTreeItemType		option_type;
	void *			option_value;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	&&  WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend_to, &backend_to_impl))
	&&  WFR_STATUS_SUCCESS(status = WfsGetOption(backend_from, key, NULL, &option_value, &option_size, &option_type))
	&&  WFR_STATUS_SUCCESS(status = WfsSetOption(backend_to, true, false, key, option_value, option_size, option_type)))
	{
		if (save_in_backend) {
			status = backend_to_impl->SaveOptions(backend_to, backend_to_impl->tree_options);
		}
	}

	return status;
}

WfrStatus
WfsDeleteOption(
	WfsBackend	backend,
	bool		delete_in_backend,
	const char *	key
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (delete_in_backend) {
			status = backend_impl->SaveOptions(backend, backend_impl->tree_options);
		}

		if (WFR_STATUS_SUCCESS(status)) {
			status = WfrTreeDelete(
				backend_impl->tree_options, NULL,
				key, WFR_TREE_ITYPE_ANY, WfsTreeFreeItem);

			if (WFR_STATUS_IS_NOT_FOUND(status)) {
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		}
	}

	return status;
}

WfrStatus
WfsEnumerateOptions(
	WfsBackend	backend,
	bool		initfl,
	bool *		pdonefl,
	char **		pkey,
	void **		pstate
	)
{
	WfspBackend *	backend_impl;
	WfrTreeItem *	item;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = WfrTreeEnumerate(
			backend_impl->tree_options, initfl,
			pdonefl, &item, pstate);
		if (!initfl && WFR_STATUS_SUCCESS(status) && !(*pdonefl)) {
			if (!(*pkey = strdup(item->key))) {
				status = WFR_STATUS_FROM_ERRNO();
			}
		}
	}

	return status;
}

WfrStatus
WfsExportOptions(
	WfsBackend	backend_from,
	WfsBackend	backend_to,
	bool		clear_to,
	bool		continue_on_error,
	WfsErrorFn	error_fn
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	bool		donefl = false;
	void *		enum_state = NULL;
	char *		key;
	WfrStatus	status;


	if (WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	||  WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_STATUS_FAILURE(status = WfsLoadOptions(backend_from))) {
		return status;
	}

	if (WFR_STATUS_SUCCESS(status) && clear_to) {
		if (WFR_STATUS_FAILURE(status = WfsClearOptions(backend_to, true))) {
			return status;
		}
	}

	status = WfsEnumerateOptions(backend_from, true, &donefl, &key, &enum_state);

	if (WFR_STATUS_SUCCESS(status)) {
		do {
			key = NULL;
			status = WfsEnumerateOptions(backend_from, false, &donefl, &key, &enum_state);

			if (WFR_STATUS_SUCCESS(status) && key) {
				status = WfsCopyOption(backend_from, backend_to, false, key);
			}

			if (WFR_STATUS_FAILURE(status)) {
				error_fn(key, status);
			}
		} while (!donefl && (WFR_STATUS_SUCCESS(status) || continue_on_error));
	}

	if (WFR_STATUS_FAILURE(status) && continue_on_error) {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_SUCCESS(status)) {
		status = backend_to_impl->SaveOptions(backend_to, backend_to_impl->tree_options);
	} else {
		(void)WfsClearOptions(backend_to, true);
	}

	return status;
}

WfrStatus
WfsGetOption(
	WfsBackend		backend,
	const char *		key,
	WfrTreeItem **		pitem,
	void **			pvalue,
	size_t *		pvalue_size,
	WfrTreeItemType *	pvalue_type
	)
{
	WfspBackend *	backend_impl;
	WfrTreeItem *	item;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = WfrTreeGet(
			backend_impl->tree_options, key,
			WFR_TREE_ITYPE_ANY, &item);

		if (WFR_STATUS_SUCCESS(status)) {
			if (pitem) {
				*pitem = item;
			}
			if (pvalue) {
				*pvalue = item->value;
			}
			if (pvalue_size) {
				*pvalue_size = item->value_size;
			}
			if (pvalue_type) {
				*pvalue_type = item->type;
			}
		}
	}

	return status;
}

WfrStatus
WfsGetOptionIntWithDefault(
	WfsBackend		backend,
	const char *		key,
	int			value_default,
	int **			pvalue
	)
{
	int *		option_value;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetOptionWithDefault(
			backend, key, &value_default, sizeof(value_default),
			WFR_TREE_ITYPE_INT, (void **)&option_value, NULL, NULL)))
	{
		*pvalue = option_value;
	}

	return status;
}

WfrStatus
WfsGetOptionWithDefault(
	WfsBackend		backend,
	const char *		key,
	void *			value_default,
	size_t			value_default_size,
	WfrTreeItemType		value_default_type,
	void **			pvalue,
	size_t *		pvalue_size,
	WfrTreeItemType *	pvalue_type
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetOption(
			backend, key, NULL, pvalue, pvalue_size, pvalue_type)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	} else if (WFR_STATUS_IS_NOT_FOUND(status)) {
		status = WfsSetOption(
			backend, true, true, key, value_default,
			value_default_size, value_default_type);

		if (WFR_STATUS_SUCCESS(status)) {
			status = WfsGetOption(backend, key, NULL, pvalue, pvalue_size, pvalue_type);
		}
	}

	return status;
}

WfrStatus
WfsLoadOptions(
	WfsBackend	backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->LoadOptions(backend);
	}

	return status;
}

WfrStatus
WfsRenameOption(
	WfsBackend	backend,
	bool		rename_in_backend,
	const char *	key,
	const char *	key_new
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = WfrTreeRename(
			backend_impl->tree_options, NULL,
			key, WFR_TREE_ITYPE_ANY,
			key_new, WfsTreeFreeItem);

		if (rename_in_backend
		&&  (WFR_STATUS_SUCCESS(status) || WFR_STATUS_IS_NOT_FOUND(status)))
		{
			if (WFR_STATUS_SUCCESS(status)) {
				status = backend_impl->SaveOptions(backend, backend_impl->tree_options);
			}
		}
	}

	return status;
}

WfrStatus
WfsSaveOptions(
	WfsBackend	backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->SaveOptions(backend, backend_impl->tree_options);
	}

	return status;
}

WfrStatus
WfsSetOption(
	WfsBackend		backend,
	bool			clone_value,
	bool			set_in_backend,
	const char *		key,
	const void *		value,
	size_t			value_size,
	WfrTreeItemType		value_type
	)
{
	WfspBackend *	backend_impl;
	WfrTreeItem	item_clone;
	WfrStatus	status;
	void *		value_ptr;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (clone_value) {
			WFR_TREE_ITEM_INIT(item_clone);
			item_clone.type = value_type;
			item_clone.value = (void *)value;
			item_clone.value_size = value_size;

			status = WfsTreeCloneValue(&item_clone, &value_ptr);
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
			value_ptr = (void *)value;
		}

		if (WFR_STATUS_SUCCESS(status)) {
			status = WfrTreeSet(
				backend_impl->tree_options, key, value_type,
				(void *)value_ptr, value_size, WfsTreeFreeItem);
		}

		if (WFR_STATUS_SUCCESS(status)
		&&  set_in_backend)
		{
			status = backend_impl->SaveOptions(backend, backend_impl->tree_options);
		}
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
