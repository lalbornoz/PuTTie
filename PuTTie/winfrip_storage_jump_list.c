/*
 * winfrip_storage_jump_list.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "storage.h"
#pragma GCC diagnostic pop

#include <stdlib.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_adapter.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_jump_list.h"
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

	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
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


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
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


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
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

	if (WFR_FAILURE(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	||  WFR_FAILURE(status = WfsGetBackendImpl(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_SUCCESS(status = backend_from_impl->GetEntriesJumpList(&jump_list, &jump_list_size))
	&&  WFR_SUCCESS(status = backend_to_impl->SetEntriesJumpList(jump_list, jump_list_size)))
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


	if (WFR_SUCCESS(WfsGetBackendImpl(backend, &backend_impl))) {
		(void)backend_impl->GetEntriesJumpList(&jump_list, &jump_list_size);
	}

	return jump_list;
}

WfrStatus
WfsPurgeJumpList(
	WfsBackend	backend,
	size_t *	ppurge_count
	)
{
	WfspBackend *	backend_impl;
	bool		display_errorsfl;
	char *		jump_list = NULL;
	size_t		jump_list_size;
	size_t		purge_count;
	settings_r *	settings_tmp;
	WfrStatus	status;


	if (WFR_FAILURE(status = WfsGetBackendImpl(backend, &backend_impl))) {
		return status;
	}

	if (WFR_SUCCESS(status = backend_impl->GetEntriesJumpList(&jump_list, &jump_list_size))) {
		display_errorsfl = WfsDisableAdapterDisplayErrors();
		purge_count = 0;

		for (char *item = jump_list, *item_next = NULL;
		     item && *item; item = item_next)
		{
			if ((item_next = strchr(item, '\0'))) {
				item_next++;

				settings_tmp = open_settings_r(item);
				if (!settings_tmp) {
					WfsRemoveJumpList(backend, item);
					purge_count++;
				}
				close_settings_r(settings_tmp);
			}
		}

		WfsSetAdapterDisplayErrors(display_errorsfl);
		if (ppurge_count) {
			*ppurge_count = purge_count;
		}
	}

	WFR_FREE_IF_NOTNULL(jump_list);

	return status;
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

	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		backend_impl->RemoveJumpList(sessionname);
	}
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
