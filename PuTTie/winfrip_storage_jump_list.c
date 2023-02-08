/*
 * winfrip_storage_jump_list.c - pointless frippery & tremendous amounts of bloat
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
 * Public jump list storage subroutines private to PuTTie/winfrip_storage*.c
 */

void
WfsAddJumpList(
	WfsBackend		backend,
	const char *const	sessionname
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	/* Do nothing on pre-Win7 systems. */
	if ((WfrGetOsVersionMajor() < 6)
	||  ((WfrGetOsVersionMajor() == 6) && (WfrGetOsVersionMinor() < 1)))
	{
		return;
	}

	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		backend_impl->AddJumpList(sessionname);
	}
}

WfrStatus
WfsCleanupJumpList(
	WfsBackend	backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->CleanupJumpList();
	}

	return status;
}

void
WfsClearJumpList(
	WfsBackend	backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		backend_impl->ClearJumpList();
	}
}

WfrStatus
WfsExportJumpList(
	WfsBackend	backend_from,
	WfsBackend	backend_to,
	bool		clear_to,
	bool		continue_on_error,
	WfsErrorFn	error_fn
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	char *		jump_list = NULL;
	size_t		jump_list_size;
	WfrStatus	status;


	(void)clear_to;
	(void)continue_on_error;
	(void)error_fn;

	if (WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	||  WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_STATUS_SUCCESS(status = backend_from_impl->GetEntriesJumpList(&jump_list, &jump_list_size))
	&&  WFR_STATUS_SUCCESS(status = backend_to_impl->SetEntriesJumpList(jump_list, jump_list_size)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_FREE_IF_NOTNULL(jump_list);

	return status;
}

char *
WfsGetEntriesJumpList(
	WfsBackend	backend
	)
{
	WfspBackend *	backend_impl;
	char *		jump_list = NULL;
	size_t		jump_list_size;


	if (WFR_STATUS_SUCCESS(WfsGetBackendImpl(backend, &backend_impl))) {
		(void)backend_impl->GetEntriesJumpList(&jump_list, &jump_list_size);
	}

	return jump_list;
}

void
WfsRemoveJumpList(
	WfsBackend		backend,
	const char *const	sessionname
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	/* Do nothing on pre-Win7 systems. */
	if ((WfrGetOsVersionMajor() < 6)
	||  ((WfrGetOsVersionMajor() == 6) && (WfrGetOsVersionMinor() < 1)))
	{
		return;
	}

	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		backend_impl->RemoveJumpList(sessionname);
	}
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
