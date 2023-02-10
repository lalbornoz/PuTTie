/*
 * winfrip_storage_host_ca.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"
#include "PuTTie/winfrip_storage_backend_ephemeral.h"
#include "PuTTie/winfrip_storage_backend_file.h"
#include "PuTTie/winfrip_storage_backend_registry.h"

/*
 * Public host CAs storage subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfsAddHostCA(
	WfsBackend	backend,
	const char *	public_key,
	time_t		mtime,
	const char *	name,
	bool		permit_rsa_sha1,
	bool		permit_rsa_sha256,
	bool		permit_rsa_sha512,
	const char *	validity,
	WfsHostCA **	phca
	)
{
	WfspBackend *	backend_impl;
	const char *	public_key_new = NULL;
	WfsHostCA *	hca_new = NULL;
	const char *	name_new = NULL;
	WfrStatus	status;
	const char *	validity_new = NULL;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))
	&&  WFR_SUCCESS_POSIX(status, (public_key_new = strdup(public_key)))
	&&  WFR_SUCCESS_POSIX(status, (hca_new = WFR_NEW(WfsHostCA)))
	&&  WFR_SUCCESS_POSIX(status, (name_new = strdup(name)))
	&&  WFR_SUCCESS_POSIX(status, (validity_new = strdup(validity))))
	{
		WFS_HOST_CA_INIT(*hca_new);
		hca_new->public_key = public_key_new;
		hca_new->mtime = mtime;
		hca_new->name = name_new;
		hca_new->permit_rsa_sha1 = permit_rsa_sha1;
		hca_new->permit_rsa_sha256 = permit_rsa_sha256;
		hca_new->permit_rsa_sha512 = permit_rsa_sha512;
		hca_new->validity = validity_new;

		if (WFR_SUCCESS(status = WfrTreeSet(
				backend_impl->tree_host_ca, name,
				WFR_TREE_ITYPE_HOST_CA, hca_new,
				sizeof(*hca_new), WfsTreeFreeItem)))
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		if (WFR_SUCCESS(status)) {
			*phca = hca_new;
		}
	}

	if (WFR_FAILURE(status)) {
		WFR_FREE_IF_NOTNULL(hca_new);
		WFR_FREE_IF_NOTNULL(name_new);
		WFR_FREE_IF_NOTNULL(public_key_new);
		WFR_FREE_IF_NOTNULL(validity_new);
	}

	return status;
}

WfrStatus
WfsCleanupHostCAs(
	WfsBackend	backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (WFR_SUCCESS(status = WfsClearHostCAs(backend, false))) {
			status = backend_impl->CleanupHostCAs(backend);
		}
	}

	return status;
}

WfrStatus
WfsClearHostCAs(
	WfsBackend	backend,
	bool		delete_in_backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (delete_in_backend) {
			status = backend_impl->ClearHostCAs(backend);
		}

		if (WFR_SUCCESS(status)) {
			status = WfrTreeClear(&backend_impl->tree_host_ca, WfsTreeFreeItem);
		}
	}

	return status;
}

WfrStatus
WfsCloseHostCA(
	WfsBackend	backend,
	WfsHostCA *	hca
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->CloseHostCA(backend, hca);
	}

	return status;
}

WfrStatus
WfsCopyHostCA(
	WfsBackend	backend_from,
	WfsBackend	backend_to,
	const char *	name,
	WfsHostCA *	hca,
	WfsHostCA **	phca
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	WfsHostCA *	hca_to;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	&&  WFR_SUCCESS(status = WfsGetBackendImpl(backend_to, &backend_to_impl)))
	{
		if (!hca) {
			status = WfsGetHostCA(backend_from, true, name, &hca);
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		if (WFR_SUCCESS(status)
		&&  WFR_SUCCESS(status = WfsAddHostCA(
				backend_to, hca->public_key, hca->mtime, name,
				hca->permit_rsa_sha1, hca->permit_rsa_sha256,
				hca->permit_rsa_sha512, hca->validity,
				&hca_to)))
		{
			*phca = hca_to;
		}
	}

	return status;
}

WfrStatus
WfsDeleteHostCA(
	WfsBackend	backend,
	bool		delete_in_backend,
	const char *	name
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (delete_in_backend) {
			status = backend_impl->DeleteHostCA(backend, name);
		}

		if (WFR_SUCCESS(status)) {
			status = WfrTreeDelete(
				backend_impl->tree_host_ca,
				NULL, name, WFR_TREE_ITYPE_HOST_CA,
				WfsTreeFreeItem);

			if (WFR_STATUS_IS_NOT_FOUND(status)) {
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		}
	}

	return status;
}

WfrStatus
WfsEnumerateHostCAs(
	WfsBackend	backend,
	bool		cached,
	bool		initfl,
	bool *		pdonefl,
	char **		pname,
	void **		pstate
	)
{
	WfspBackend *	backend_impl;
	WfrTreeItem *	item;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		switch (cached) {
		case true:
			status = WfrTreeEnumerate(
				backend_impl->tree_host_ca, initfl,
				pdonefl, &item, pstate);
			if (!initfl && WFR_SUCCESS(status) && !(*pdonefl)) {
				WFR_STATUS_BIND_POSIX(status,
					(*pname = strdup(item->key)));
			}
			break;

		case false:
			status = backend_impl->EnumerateHostCAs(
				backend, initfl, pdonefl,
				pname, pstate);
			break;
		}
	}

	return status;
}

WfrStatus
WfsExportHostCA(
	WfsBackend	backend_from,
	WfsBackend	backend_to,
	bool		movefl,
	char *		name
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	WfsHostCA *	hca;
	WfrStatus	status;


	if (WFR_FAILURE(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	||  WFR_FAILURE(status = WfsGetBackendImpl(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_SUCCESS(status = backend_from_impl->LoadHostCA(backend_to, name, &hca))
	&&  WFR_SUCCESS(status = backend_to_impl->SaveHostCA(backend_to, hca)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_SUCCESS(status) && movefl) {
		status = WfsDeleteHostCA(backend_from, true, name);
	}

	return status;
}

WfrStatus
WfsExportHostCAs(
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
	char *		name;
	WfrStatus	status;


	if (WFR_FAILURE(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	||  WFR_FAILURE(status = WfsGetBackendImpl(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_SUCCESS(status) && clear_to) {
		if (WFR_FAILURE(status = WfsClearHostCAs(backend_to, true))) {
			return status;
		}
	}

	status = WfsEnumerateHostCAs(
		backend_from, false, true,
		&donefl, &name, &enum_state);

	if (WFR_SUCCESS(status)) {
		do {
			name = NULL;
			status = WfsEnumerateHostCAs(
				backend_from, false, false,
				&donefl, &name, &enum_state);

			if (WFR_SUCCESS(status) && name) {
				status = WfsExportHostCA(
					backend_from, backend_to,
					false, name);
			}

			if (WFR_FAILURE(status)) {
				error_fn(name, status);
			}
		} while (!donefl && (WFR_SUCCESS(status) || continue_on_error));
	}

	if (WFR_FAILURE(status) && continue_on_error) {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_FAILURE(status)) {
		(void)WfsClearHostCAs(backend_to, true);
	}

	return status;
}

WfrStatus
WfsGetHostCA(
	WfsBackend	backend,
	bool		cached,
	const char *	name,
	WfsHostCA **	phca
	)
{
	WfspBackend *	backend_impl;
	WfrTreeItem *	item;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		switch (cached) {
		case true:
			status = WfrTreeGet(
				backend_impl->tree_host_ca, name,
				WFR_TREE_ITYPE_HOST_CA, &item);
			if (WFR_SUCCESS(status)) {
				*phca = item->value;
			}
			break;

		case false:
			status = backend_impl->LoadHostCA(backend, name, phca);
			break;
		}
	}

	return status;
}

WfrStatus
WfsRenameHostCA(
	WfsBackend	backend,
	bool		rename_in_backend,
	const char *	name,
	const char *	name_new
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = WfrTreeRename(
			backend_impl->tree_host_ca, NULL,
			name, WFR_TREE_ITYPE_HOST_CA,
			name_new, WfsTreeFreeItem);

		if (rename_in_backend
		&&  (WFR_SUCCESS(status) || WFR_STATUS_IS_NOT_FOUND(status)))
		{
			status = backend_impl->RenameHostCA(
				backend, name, name_new);
		}
	}

	return status;
}

WfrStatus
WfsSaveHostCA(
	WfsBackend	backend,
	WfsHostCA *	hca
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->SaveHostCA(backend, hca);
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
