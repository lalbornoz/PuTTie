/*
 * winfrip_storage_backend_ephemeral.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include "PuTTie/winfrip_rtl_status.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_pcre2.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_backend_ephemeral.h"

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfspEphemeralCleanupHostKeys(
	WfsBackend	backend
	)
{
	return WfsClearHostKeys(backend, false);
}

WfrStatus
WfspEphemeralClearHostKeys(
	WfsBackend	backend
	)
{
	return WfsClearHostKeys(backend, false);
}

WfrStatus
WfspEphemeralDeleteHostKey(
	WfsBackend	backend,
	const char *	key_name
	)
{
	return WfsDeleteHostKey(backend, false, key_name);
}

WfrStatus
WfspEphemeralEnumerateHostKeys(
	WfsBackend	backend,
	bool		initfl,
	bool *		pdonefl,
	const char **	pkey_name,
	void *		state
	)
{
	return WfsEnumerateHostKeys(backend, true, initfl, pdonefl, pkey_name, state);
}

WfrStatus
WfspEphemeralLoadHostKey(
	WfsBackend	backend,
	const char *	key_name,
	const char **	pkey
	)
{
	const char *	key, *key_;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetHostKey(
			WFS_BACKEND_EPHEMERAL,
			true, key_name, &key)))
	{
		if (backend != WFS_BACKEND_EPHEMERAL) {
			if (!(key_ = strdup(key))) {
				status = WFR_STATUS_FROM_ERRNO();
			} else {
				status = WfsSetHostKey(backend, key_name, key_);
				if (WFR_STATUS_SUCCESS(status)) {
					*pkey = key_;
				} else {
					sfree((void *)key_);
				}
			}
		} else {
			*pkey = key;
		}
	}

	return status;
}

WfrStatus
WfspEphemeralRenameHostKey(
	WfsBackend	backend,
	const char *	key_name,
	const char *	key_name_new
	)
{
	return WfsRenameHostKey(backend, false, key_name, key_name_new);
}

WfrStatus
WfspEphemeralSaveHostKey(
	WfsBackend	backend,
	const char *	key_name,
	const char *	key
	)
{
	(void)backend;
	(void)key_name;
	(void)key;

	return WFR_STATUS_CONDITION_SUCCESS;
}


WfrStatus
WfspEphemeralCleanupSessions(
	WfsBackend	backend
	)
{
	return WfsClearSessions(backend, false);
}

WfrStatus
WfspEphemeralClearSessions(
	WfsBackend	backend
	)
{
	return WfsClearSessions(backend, false);
}

WfrStatus
WfspEphemeralCloseSession(
	WfsBackend	backend,
	WfspSession *	session
	)
{
	(void)backend;
	(void)session;

	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspEphemeralDeleteSession(
	WfsBackend	backend,
	const char *	sessionname
	)
{
	return WfsDeleteSession(backend, false, sessionname);
}

WfrStatus
WfspEphemeralEnumerateSessions(
	WfsBackend	backend,
	bool		initfl,
	bool *		pdonefl,
	char **		psessionname,
	void *		state
	)
{
	return WfsEnumerateSessions(backend, true, initfl, pdonefl, psessionname, state);
}

WfrStatus
WfspEphemeralLoadSession(
	WfsBackend	backend,
	const char *	sessionname,
	WfspSession **	psession
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetSession(
			WFS_BACKEND_EPHEMERAL, true,
			sessionname, psession)))
	{
		if (backend != WFS_BACKEND_EPHEMERAL) {
			status = WfsCopySession(
				WFS_BACKEND_EPHEMERAL, backend,
				sessionname, *psession, psession);
		}
	}

	return status;
}

WfrStatus
WfspEphemeralRenameSession(
	WfsBackend	backend,
	const char *	sessionname,
	const char *	sessionname_new
	)
{
	return WfsRenameSession(backend, false, sessionname, sessionname_new);
}

WfrStatus
WfspEphemeralSaveSession(
	WfsBackend	backend,
	WfspSession *	session
	)
{
	(void)backend;
	(void)session;

	return WFR_STATUS_CONDITION_SUCCESS;
}


void
WfspEphemeralJumpListAdd(
	const char *const	sessionname
	)
{
	(void)sessionname;

	/*
	 * Inhibit jump list processing
	 */
}

WfrStatus
WfspEphemeralJumpListCleanup(
	void
	)
{
	return WFR_STATUS_CONDITION_SUCCESS;
}

void
WfspEphemeralJumpListClear(
	void
	)
{
	/*
	 * Inhibit jump list processing
	 */
}

WfrStatus
WfspEphemeralJumpListGetEntries(
	char **		pjump_list,
	size_t *	pjump_list_size
	)
{
	*pjump_list = NULL;
	*pjump_list_size = 0;
	return WFR_STATUS_CONDITION_SUCCESS;
}

void
WfspEphemeralJumpListRemove(
	const char *const	sessionname
	)
{
	(void)sessionname;

	/*
	 * Inhibit jump list processing
	 */
}

WfrStatus
WfspEphemeralJumpListSetEntries(
	const char *	jump_list,
	size_t		jump_list_size
	)
{
	(void)jump_list;
	(void)jump_list_size;
	return WFR_STATUS_CONDITION_SUCCESS;
}



WfrStatus
WfspEphemeralCleanupContainer(
	WfsBackend	backend
	)
{
	(void)backend;
	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspEphemeralInit(
	void
	)
{
	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspEphemeralSetBackend(
	WfsBackend	backend_new
	)
{
	(void)backend_new;
	return WFR_STATUS_CONDITION_SUCCESS;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
