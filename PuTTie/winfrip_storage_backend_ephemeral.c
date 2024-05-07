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
#include "PuTTie/winfrip_storage_options.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"
#include "PuTTie/winfrip_storage_backend_ephemeral.h"

/*
 * Private variables
 */

static char *	WfspEphemeralJumpList = NULL;
static size_t	WfspEphemeralJumpListSize = 0;

static char *	WfspEphemeralPrivKeyList = NULL;
static size_t	WfspEphemeralPrivKeyListSize = 0;

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
	void **		pstate
	)
{
	return WfsEnumerateHostCAs(backend, true, initfl, pdonefl, pname, pstate);
}

WfrStatus
WfspEphemeralLoadHostCA(
	WfsBackend	backend,
	const char *	name,
	WfsHostCA **	phca
	)
{
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetHostCA(
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
	void **		pstate
	)
{
	return WfsEnumerateHostKeys(backend, true, initfl, pdonefl, pkey_name, pstate);
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


	if (WFR_SUCCESS(status = WfsGetHostKey(
			WFS_BACKEND_EPHEMERAL,
			true, key_name, &key)))
	{
		if (backend != WFS_BACKEND_EPHEMERAL) {
			if (WFR_SUCCESS_POSIX(status, (key_ = strdup(key)))) {
				status = WfsSetHostKey(backend, false, key_name, key_);
				if (WFR_SUCCESS(status)) {
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
WfspEphemeralClearOptions(
	WfsBackend	backend
	)
{
	(void)backend;
	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspEphemeralLoadOptions(
	WfsBackend	backend
	)
{
	(void)backend;
	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspEphemeralSaveOptions(
	WfsBackend	backend,
	WfrTree *	backend_tree
	)
{
	(void)backend;
	(void)backend_tree;

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
	void **		pstate
	)
{
	return WfsEnumerateSessions(backend, true, initfl, pdonefl, psessionname, pstate);
}

WfrStatus
WfspEphemeralLoadSession(
	WfsBackend	backend,
	const char *	sessionname,
	WfsSession **	psession
	)
{
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetSession(
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
	(void)WfrUpdateStringList(
		true, false, &WfspEphemeralJumpList,
		&WfspEphemeralJumpListSize, sessionname);
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
	WFR_FREE_IF_NOTNULL(WfspEphemeralJumpList);
	WfspEphemeralJumpListSize = 0;
}

WfrStatus
WfspEphemeralGetEntriesJumpList(
	char **		pjump_list,
	size_t *	pjump_list_size
	)
{
	char *		jump_list_copy;
	WfrStatus	status;


	if (WfspEphemeralJumpListSize > 0) {
		if (WFR_NEWN(status, jump_list_copy, WfspEphemeralJumpListSize, char)) {
			memcpy(jump_list_copy, WfspEphemeralJumpList, WfspEphemeralJumpListSize);
			*pjump_list = jump_list_copy;
			if (pjump_list_size) {
				*pjump_list_size = WfspEphemeralJumpListSize;
			}
		}
	} else {
		if (WFR_NEWN(status, jump_list_copy, 2, char)) {
			*pjump_list = jump_list_copy;
			(*pjump_list)[0] = '\0';
			(*pjump_list)[1] = '\0';
			if (pjump_list_size) {
				*pjump_list_size = 2;
			}
		}
	}

	return status;
}

void
WfspEphemeralRemoveJumpList(
	const char *const	sessionname
	)
{
	(void)WfrUpdateStringList(
		false, true, &WfspEphemeralJumpList,
		&WfspEphemeralJumpListSize, sessionname);
}

WfrStatus
WfspEphemeralSetEntriesJumpList(
	const char *	jump_list,
	size_t		jump_list_size
	)
{
	char *		jump_list_new;
	WfrStatus	status;


	if (WFR_NEWN(status, jump_list_new, jump_list_size, char)) {
		memcpy(jump_list_new, jump_list, jump_list_size);
		WFR_FREE_IF_NOTNULL(WfspEphemeralJumpList);
		WfspEphemeralJumpList = jump_list_new;
		WfspEphemeralJumpListSize = jump_list_size;
	}

	return status;
}


WfrStatus
WfspEphemeralAddPrivKeyList(
	const char *const	sessionname
	)
{
	return WfrUpdateStringList(
		true, false, &WfspEphemeralPrivKeyList,
		&WfspEphemeralPrivKeyListSize, sessionname);
}

WfrStatus
WfspEphemeralCleanupPrivKeyList(
	void
	)
{
	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspEphemeralClearPrivKeyList(
	void
	)
{
	WFR_FREE_IF_NOTNULL(WfspEphemeralPrivKeyList);
	WfspEphemeralPrivKeyListSize = 0;

	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspEphemeralGetEntriesPrivKeyList(
	char **		pprivkey_list,
	size_t *	pprivkey_list_size
	)
{
	char *		privkey_list_copy;
	WfrStatus	status;


	if (WfspEphemeralPrivKeyListSize > 0) {
		if (WFR_NEWN(status, privkey_list_copy, WfspEphemeralPrivKeyListSize, char)) {
			memcpy(privkey_list_copy, WfspEphemeralPrivKeyList, WfspEphemeralPrivKeyListSize);
			*pprivkey_list = privkey_list_copy;
			if (pprivkey_list_size) {
				*pprivkey_list_size = WfspEphemeralPrivKeyListSize;
			}
		}
	} else {
		if (WFR_NEWN(status, privkey_list_copy, 2, char)) {
			*pprivkey_list = privkey_list_copy;
			(*pprivkey_list)[0] = '\0';
			(*pprivkey_list)[1] = '\0';
			if (pprivkey_list_size) {
				*pprivkey_list_size = 2;
			}
		}
	}

	return status;
}

WfrStatus
WfspEphemeralRemovePrivKeyList(
	const char *const	sessionname
	)
{
	return WfrUpdateStringList(
		false, true, &WfspEphemeralPrivKeyList,
		&WfspEphemeralPrivKeyListSize, sessionname);
}

WfrStatus
WfspEphemeralSetEntriesPrivKeyList(
	const char *	privkey_list,
	size_t		privkey_list_size
	)
{
	char *		privkey_list_new;
	WfrStatus	status;


	if (WFR_NEWN(status, privkey_list_new, privkey_list_size, char)) {
		memcpy(privkey_list_new, privkey_list, privkey_list_size);
		WFR_FREE_IF_NOTNULL(WfspEphemeralPrivKeyList);
		WfspEphemeralPrivKeyList = privkey_list_new;
		WfspEphemeralPrivKeyListSize = privkey_list_size;
	}

	return status;
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
WfspEphemeralEnumerateCancel(
	WfsBackend	backend,
	void **		pstate
	)
{
	(void)backend;
	WfrTreeEnumerateCancel(pstate);
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
	WfsBackend	backend_new,
	char *		args_extra
	)
{
	(void)backend_new;
	WFR_FREE_IF_NOTNULL(args_extra);
	return WFR_STATUS_CONDITION_SUCCESS;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
