/*
 * winfrip_storage_host_keys.c - pointless frippery & tremendous amounts of bloat
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
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_host_keys.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"
#include "PuTTie/winfrip_storage_backend_ephemeral.h"
#include "PuTTie/winfrip_storage_backend_file.h"
#include "PuTTie/winfrip_storage_backend_registry.h"

/*
 * Public host key storage subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfsCleanupHostKeys(
	WfsBackend	backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (WFR_STATUS_SUCCESS(status = WfsClearHostKeys(backend, false))) {
			status = backend_impl->CleanupHostKeys(backend);
		}
	}

	return status;
}

WfrStatus
WfsClearHostKeys(
	WfsBackend	backend,
	bool		delete_in_backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (delete_in_backend) {
			status = backend_impl->ClearHostKeys(backend);
		}

		if (WFR_STATUS_SUCCESS(status)) {
			status = WfsTreeClear(&backend_impl->tree_host_key, WfsTreeFreeItem);
		}
	}

	return status;
}

WfrStatus
WfsDeleteHostKey(
	WfsBackend	backend,
	bool		delete_in_backend,
	const char *	key_name
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (delete_in_backend) {
			status = backend_impl->DeleteHostKey(backend, key_name);
		}

		if (WFR_STATUS_SUCCESS(status)) {
			status = WfsTreeDelete(
				backend_impl->tree_host_key, NULL,
				key_name, WFS_TREE_ITYPE_HOST_KEY,
				WfsTreeFreeItem);

			if ((WFR_STATUS_CONDITION(status) == ENOENT)
			&&  delete_in_backend)
			{
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		}
	}

	return status;
}

WfrStatus
WfsEnumerateHostKeys(
	WfsBackend	backend,
	bool		cached,
	bool		initfl,
	bool *		pdonefl,
	char **		pkey_name,
	void *		state
	)
{
	WfspBackend *	backend_impl = NULL;
	WfsTreeItem *	item;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		switch (cached) {
		case true:
			status = WfsTreeEnumerate(
				backend_impl->tree_host_key,
				initfl, pdonefl, &item, state);
			if (!initfl && WFR_STATUS_SUCCESS(status) && !(*pdonefl)) {
				if (!(*pkey_name = strdup(item->key))) {
					status = WFR_STATUS_FROM_ERRNO();
				}
			}
			break;

		case false:
			status = backend_impl->EnumerateHostKeys(
				backend, initfl, pdonefl,
				pkey_name, state);
			break;
		}
	}

	return status;
}

WfrStatus
WfsExportHostKey(
	WfsBackend	backend_from,
	WfsBackend	backend_to,
	bool		movefl,
	char *		key_name
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	const char *	key;
	WfrStatus	status;


	if (WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	||  WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_STATUS_SUCCESS(status = backend_from_impl->LoadHostKey(backend_to, key_name, &key))
	&&  WFR_STATUS_SUCCESS(status = backend_to_impl->SaveHostKey(backend_to, key_name, key)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_SUCCESS(status) && movefl) {
		status = WfsDeleteHostKey(backend_from, true, key_name);
	}

	return status;
}

WfrStatus
WfsExportHostKeys(
	WfsBackend	backend_from,
	WfsBackend	backend_to,
	bool		clear_to,
	bool		continue_on_error,
	void		(*error_fn)(const char *, WfrStatus)
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	bool		donefl = false;
	void *		enum_state = NULL;
	const char *	key_name;
	WfrStatus	status;


	if (WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	||  WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_STATUS_SUCCESS(status) && clear_to) {
		if (WFR_STATUS_FAILURE(status = WfsClearHostKeys(backend_to, false))) {
			return status;
		}
	}

	status = WfsEnumerateHostKeys(
		backend_from, false, true, &donefl,
		(char **)&key_name, &enum_state);

	if (WFR_STATUS_SUCCESS(status)) {
		do {
			key_name = NULL;
			status = WfsEnumerateHostKeys(
				backend_from, false, false,
				&donefl, (char **)&key_name, enum_state);

			if (WFR_STATUS_SUCCESS(status) && key_name) {
				status = WfsExportHostKey(
					backend_from, backend_to,
					false, (char *)key_name);
			}

			if (WFR_STATUS_FAILURE(status)) {
				error_fn(key_name, status);
			}
		} while (!donefl && (WFR_STATUS_SUCCESS(status) || continue_on_error));
	}

	if (WFR_STATUS_FAILURE(status) && continue_on_error) {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_FAILURE(status)) {
		(void)WfsClearHostKeys(backend_to, false);
	}

	return status;
}

WfrStatus
WfsGetHostKey(
	WfsBackend	backend,
	bool		cached,
	const char *	key_name,
	const char **	pkey
	)
{
	WfspBackend *	backend_impl;
	WfsTreeItem *	item;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		switch (cached) {
		case true:
			status = WfsTreeGet(
				backend_impl->tree_host_key,
				key_name, WFS_TREE_ITYPE_HOST_KEY, &item);
			if (WFR_STATUS_SUCCESS(status)) {
				*pkey = item->value;
			}
			break;

		case false:
			status = backend_impl->LoadHostKey(backend, key_name, pkey);
			break;
		}
	}

	return status;
}

WfrStatus
WfsPrintHostKeyName(
	const char *	hostname,
	int		port,
	const char *	keytype,
	char **		pkey_name
	)
{
	char *		key_name;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfrSnDuprintf(
			&key_name, NULL, "%s@%u:%s",
			keytype, port, hostname)))
	{
		*pkey_name = key_name;
	}

	return status;
}

WfrStatus
WfsRenameHostKey(
	WfsBackend	backend,
	bool		rename_in_backend,
	const char *	key_name,
	const char *	key_name_new
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = WfsTreeRename(
			backend_impl->tree_host_key, NULL, key_name,
			WFS_TREE_ITYPE_HOST_KEY, key_name_new,
			WfsTreeFreeItem);

		if (rename_in_backend
		&&  (WFR_STATUS_SUCCESS(status)
		||   (WFR_STATUS_CONDITION(status) == ENOENT)))
		{
			status = backend_impl->RenameHostKey(backend, key_name, key_name_new);
		}
	}

	return status;
}

WfrStatus
WfsSaveHostKey(
	WfsBackend	backend,
	const char *	key_name,
	const char *	key
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->SaveHostKey(backend, key_name, key);
	}

	return status;
}

WfrStatus
WfsSetHostKey(
	WfsBackend	backend,
	const char *	key_name,
	const char *	key
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = WfsTreeSet(
			backend_impl->tree_host_key, key_name,
			WFS_TREE_ITYPE_HOST_KEY, (void *)key,
			strlen(key) + 1, WfsTreeFreeItem);
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
