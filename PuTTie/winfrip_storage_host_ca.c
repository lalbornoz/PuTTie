/*
 * winfrip_storage_host_ca.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include "PuTTie/winfrip_rtl_status.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "PuTTie/winfrip_storage_wrap.h"
#include "storage.h"
#include "PuTTie/winfrip_storage_unwrap.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_priv.h"
#include "PuTTie/winfrip_storage_host_ca.h"
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
	const char *	name,
	bool		permit_rsa_sha1,
	bool		permit_rsa_sha256,
	bool		permit_rsa_sha512,
	const char *	validity,
	WfspHostCA **	phca
	)
{
	WfspBackend *	backend_impl;
	const char *	public_key_new = NULL;
	WfspHostCA *	hca_new = NULL;
	const char *	name_new;
	WfrStatus	status;
	const char *	validity_new = NULL;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (!(public_key_new = strdup(public_key))
		||  !(hca_new = snew(WfspHostCA))
		||  !(name_new = strdup(name))
		||  !(validity_new = strdup(validity)))
		{
			WFR_SFREE_IF_NOTNULL((void *)public_key_new);
			WFR_SFREE_IF_NOTNULL(hca_new);
			WFR_SFREE_IF_NOTNULL((void *)name_new);
			WFR_SFREE_IF_NOTNULL((void *)validity_new);
			status = WFR_STATUS_FROM_ERRNO();
		} else {
			WFSP_HOST_CA_INIT(*hca_new);
			hca_new->public_key = public_key_new;
			hca_new->name = name_new;
			hca_new->permit_rsa_sha1 = permit_rsa_sha1;
			hca_new->permit_rsa_sha256 = permit_rsa_sha256;
			hca_new->permit_rsa_sha512 = permit_rsa_sha512;
			hca_new->validity = validity_new;

			if (WFR_STATUS_SUCCESS(status = WfspTreeSet(
					backend_impl->tree_host_ca, name,
					WFSP_TREE_ITYPE_HOST_CA, hca_new,
					sizeof(*hca_new))))
			{
				status = WFR_STATUS_CONDITION_SUCCESS;
			}

			if (WFR_STATUS_SUCCESS(status)) {
				*phca = hca_new;
			} else {
				sfree(hca_new);
				sfree((void *)public_key_new);
				sfree((void *)name_new);
				sfree((void *)validity_new);
			}
		}
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


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (WFR_STATUS_SUCCESS(status = backend_impl->ClearHostCAs(backend))) {
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


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (delete_in_backend) {
			status = backend_impl->ClearHostCAs(backend);
		}

		if (WFR_STATUS_SUCCESS(status)) {
			status = WfspTreeClear(&backend_impl->tree_host_ca);
		}
	}

	return status;
}

WfrStatus
WfsCloseHostCA(
	WfsBackend	backend,
	WfspHostCA *	hca
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->CloseHostCA(backend, hca);
	}

	return status;
}

WfrStatus
WfsCopyHostCA(
	WfsBackend	backend_from,
	WfsBackend	backend_to,
	const char *	name,
	WfspHostCA *	hca,
	WfspHostCA **	phca
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	WfspHostCA *	hca_to;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	&&  WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend_to, &backend_to_impl)))
	{
		if (!hca) {
			status = WfsGetHostCA(backend_from, true, name, &hca);
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		if (WFR_STATUS_SUCCESS(status)
		&&  WFR_STATUS_SUCCESS(status = WfsAddHostCA(
				backend_to, hca->public_key, name,
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


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (delete_in_backend) {
			status = backend_impl->DeleteHostCA(backend, name);
		}

		if (WFR_STATUS_SUCCESS(status)) {
			status = WfspTreeDelete(
				backend_impl->tree_host_ca,
				NULL, name,
				WFSP_TREE_ITYPE_HOST_CA);

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
WfsEnumerateHostCAs(
	WfsBackend	backend,
	bool		cached,
	bool		initfl,
	bool *		pdonefl,
	char **		pname,
	void *		state
	)
{
	WfspBackend *	backend_impl;
	WfspTreeItem *	item;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		switch (cached) {
		case true:
			status = WfspTreeEnumerate(
				backend_impl->tree_host_ca, initfl,
				pdonefl, &item, state);
			if (!initfl && WFR_STATUS_SUCCESS(status) && !(*pdonefl)) {
				if (!(*pname = strdup(item->key))) {
					status = WFR_STATUS_FROM_ERRNO();
				}
			}
			break;

		case false:
			status = backend_impl->EnumerateHostCAs(
				backend, initfl, pdonefl,
				pname, state);
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
	WfspHostCA *	hca;
	WfrStatus	status;


	if (WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	||  WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_STATUS_SUCCESS(status = backend_from_impl->LoadHostCA(backend_to, name, &hca))
	&&  WFR_STATUS_SUCCESS(status = backend_to_impl->SaveHostCA(backend_to, hca)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_SUCCESS(status) && movefl) {
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
	void		(*error_fn)(const char *, WfrStatus)
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	bool		donefl = false;
	void *		enum_state = NULL;
	char *		name;
	WfrStatus	status;


	if (WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	||  WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_STATUS_SUCCESS(status) && clear_to) {
		if (WFR_STATUS_FAILURE(status = WfsClearHostCAs(backend_to, true))) {
			return status;
		}
	}

	status = WfsEnumerateHostCAs(
		backend_from, false, true,
		&donefl, &name, &enum_state);

	if (WFR_STATUS_SUCCESS(status)) {
		do {
			name = NULL;
			status = WfsEnumerateHostCAs(
				backend_from, false, false,
				&donefl, &name, enum_state);

			if (WFR_STATUS_SUCCESS(status) && name) {
				status = WfsExportHostCA(
					backend_from, backend_to,
					false, name);
			}

			if (WFR_STATUS_FAILURE(status)) {
				error_fn(name, status);
			}
		} while (!donefl && (WFR_STATUS_SUCCESS(status) || continue_on_error));
	}

	if (WFR_STATUS_FAILURE(status) && continue_on_error) {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_FAILURE(status)) {
		(void)WfsClearHostCAs(backend_to, true);
	}

	WFR_SFREE_IF_NOTNULL(enum_state);

	return status;
}

WfrStatus
WfsGetHostCA(
	WfsBackend	backend,
	bool		cached,
	const char *	name,
	WfspHostCA **	phca
	)
{
	WfspBackend *	backend_impl;
	WfspTreeItem *	item;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		switch (cached) {
		case true:
			status = WfspTreeGet(
				backend_impl->tree_host_ca, name,
				WFSP_TREE_ITYPE_HOST_CA, &item);
			if (WFR_STATUS_SUCCESS(status)) {
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


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = WfspTreeRename(
			backend_impl->tree_host_ca, NULL,
			name, WFSP_TREE_ITYPE_HOST_CA,
			name_new);

		if (rename_in_backend
		&&  (WFR_STATUS_SUCCESS(status)
		||   (WFR_STATUS_CONDITION(status) == ENOENT)))
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
	WfspHostCA *	hca
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->SaveHostCA(backend, hca);
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
