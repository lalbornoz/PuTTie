/*
 * winfrip_storage_adapter.c - pointless frippery & tremendous amounts of bloat
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
#include "PuTTie/winfrip_storage_adapter.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_host_keys.h"
#include "PuTTie/winfrip_storage_jump_list.h"
#include "PuTTie/winfrip_storage_options.h"
#include "PuTTie/winfrip_storage_privkey_list.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"

/*
 * Private variables
 */

static bool	WfspDisplayErrors = true;

/*
 * Public wrapped PuTTY storage.h type definitions and subroutines
 */

/* ----------------------------------------------------------------------
 * Cleanup function: remove all of PuTTY's persistent state.
 */

void
cleanup_all(
	void
	)
{
	WfsBackend	backend;


	backend = WfsGetBackend();
	(void)WfsCleanupHostCAs(backend);
	(void)WfsCleanupHostKeys(backend);
	(void)WfsCleanupSessions(backend);
	(void)WfsCleanupJumpList(backend);
	(void)WfsCleanupPrivKeyList(backend);
	(void)WfsCleanupOptions(backend);
	(void)WfsCleanupContainer(backend);
}

/*
 * Public wrapped PuTTY windows/jump-list.c subroutine prototypes
 */

void
add_session_to_jumplist(
	const char *const	sessionname
	)
{
	WfsAddJumpList(WfsGetBackend(), sessionname);
}

void
clear_jumplist(
	void
	)
{
	WfsClearJumpList(WfsGetBackend());
}

char *
get_jumplist_registry_entries(
	void
	)
{
	return WfsGetEntriesJumpList(WfsGetBackend());
}

void
remove_session_from_jumplist(
	const char *const	sessionname
	)
{
	WfsRemoveJumpList(WfsGetBackend(), sessionname);
}

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c
 */

bool
WfsDisableAdapterDisplayErrors(
	void
	)
{
	bool	old_fl = WfspDisplayErrors;


	WfspDisplayErrors = false;
	return old_fl;
}

bool
WfsEnableAdapterDisplayErrors(
	void
	)
{
	bool	old_fl = WfspDisplayErrors;


	WfspDisplayErrors = true;
	return old_fl;
}

bool
WfsGetAdapterDisplayErrors(
	void
	)
{
	return WfspDisplayErrors;
}
void
WfsSetAdapterDisplayErrors(
	bool	display_errorsfl
	)
{
	WfspDisplayErrors = display_errorsfl;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
