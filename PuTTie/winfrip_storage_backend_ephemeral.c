/*
 * winfrip_storage_backend_ephemeral.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <string.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_pcre2.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_host_keys.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_backend_ephemeral.h"

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfspEphemeralCleanupHostCAs(
	WfsBackend	backend
	)
{
	return WfsClearHostCAs(backend, false);
}

WfrStatus
WfspEphemeralClearHostCAs(
	WfsBackend	backend
	)
{
	return WfsClearHostCAs(backend, false);
}

WfrStatus
WfspEphemeralCloseHostCA(
	WfsBackend	backend,
	WfsHostCA *	hca
	)
{
	(void)backend;
	(void)hca;

	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspEphemeralDeleteHostCA(
	WfsBackend	backend,
	const char *	name
	)
{
	return WfsDeleteHostCA(backend, false, name);
}

WfrStatus
WfspEphemeralEnumerateHostCAs(
	WfsBackend	backend,
	bool		initfl,
	bool *		pdonefl,
	char **		pname,
	void *		state
	)
{
	return WfsEnumerateHostCAs(backend, true, initfl, pdonefl, pname, state);
}

WfrStatus
WfspEphemeralLoadHostCA(
	WfsBackend	backend,
	const char *	name,
	WfsHostCA **	phca
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetHostCA(
			WFS_BACKEND_EPHEMERAL, true,
			name, phca)))
	{
		if (backend != WFS_BACKEND_EPHEMERAL) {
			status = WfsCopyHostCA(
				WFS_BACKEND_EPHEMERAL, backend,
				name, *phca, phca);
		}
	}

	return status;
}

WfrStatus
WfspEphemeralRenameHostCA(
	WfsBackend	backend,
	const char *	name,
	const char *	name_new
	)
{
	return WfsRenameHostCA(backend, false, name, name_new);
}

WfrStatus
WfspEphemeralSaveHostCA(
	WfsBackend	backend,
	WfsHostCA *	hca
	)
{
	(void)backend;
	(void)hca;

	return WFR_STATUS_CONDITION_SUCCESS;
}


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
	char **		pkey_name,
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
					WFR_FREE(key_);
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
	WfsSession *	session
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
	WfsSession **	psession
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
	WfsSession *	session
	)
{
	(void)backend;
	(void)session;

	return WFR_STATUS_CONDITION_SUCCESS;
}


void
WfspEphemeralAddJumpList(
	const char *const	sessionname
	)
{
	(void)sessionname;

	/*
	 * Inhibit jump list processing
	 */
}

WfrStatus
WfspEphemeralCleanupJumpList(
	void
	)
{
	return WFR_STATUS_CONDITION_SUCCESS;
}

void
WfspEphemeralClearJumpList(
	void
	)
{
	/*
	 * Inhibit jump list processing
	 */
}

WfrStatus
WfspEphemeralGetEntriesJumpList(
	char **		pjump_list,
	size_t *	pjump_list_size
	)
{
	WfrStatus	status;


	if (!((*pjump_list = WFR_NEWN(2, char)))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		(*pjump_list)[0] = '\0';
		(*pjump_list)[1] = '\0';
		*pjump_list_size = 2;
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

void
WfspEphemeralRemoveJumpList(
	const char *const	sessionname
	)
{
	(void)sessionname;

	/*
	 * Inhibit jump list processing
	 */
}

WfrStatus
WfspEphemeralSetEntriesJumpList(
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
