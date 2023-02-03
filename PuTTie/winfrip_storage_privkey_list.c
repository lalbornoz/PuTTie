/*
 * winfrip_storage_privkey_list.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdlib.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"
#include "PuTTie/winfrip_storage_backend_ephemeral.h"
#include "PuTTie/winfrip_storage_backend_file.h"
#include "PuTTie/winfrip_storage_backend_registry.h"

/*
 * Public Pageant private key list storage subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfsAddPrivKeyList(
	WfsBackend		backend,
	const char *const	privkey_name
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->AddPrivKeyList(privkey_name);
	}

	return status;
}

WfrStatus
WfsCleanupPrivKeyList(
	WfsBackend	backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->CleanupPrivKeyList();
	}

	return status;
}

WfrStatus
WfsClearPrivKeyList(
	WfsBackend	backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->ClearPrivKeyList();
	}

	return status;
}

WfrStatus
WfsExportPrivKeyList(
	WfsBackend	backend_from,
	WfsBackend	backend_to,
	bool		movefl
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	char *		privkey_list = NULL;
	size_t		privkey_list_size;
	WfrStatus	status;


	if (WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	||  WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_STATUS_SUCCESS(status = backend_from_impl->GetEntriesPrivKeyList(&privkey_list, &privkey_list_size))
	&&  WFR_STATUS_SUCCESS(status = backend_to_impl->SetEntriesPrivKeyList(privkey_list, privkey_list_size)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_SUCCESS(status) && movefl) {
		status = backend_from_impl->CleanupPrivKeyList();
	}

	WFR_FREE_IF_NOTNULL(privkey_list);

	return status;
}

WfrStatus
WfsGetEntriesPrivKeyList(
	WfsBackend	backend,
	char **		pprivkey_list,
	size_t *	pprivkey_list_size
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->GetEntriesPrivKeyList(pprivkey_list, pprivkey_list_size);
	}

	return status;
}

WfrStatus
WfsRemovePrivKeyList(
	WfsBackend		backend,
	const char *const	privkey_name
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->RemovePrivKeyList(privkey_name);
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
