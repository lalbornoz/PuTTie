/*
 * winfrip_storage_backend_registry.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdlib.h>
#include <errno.h>

#include <windows.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_load.h"
#include "PuTTie/winfrip_rtl_registry.h"
#include "PuTTie/winfrip_rtl_save.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_host_keys.h"
#include "PuTTie/winfrip_storage_jump_list.h"
#include "PuTTie/winfrip_storage_options.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"
#include "PuTTie/winfrip_storage_backend_registry.h"

/*
 * Private variables
 */

/*
 * Names of registry subkeys/value names containing PuTTie/PuTTy's top-level keys,
 * host CAs, host keys, the jump list, global options, the private key list,
 * and sessions, resp. [see windows/storage.c]
 */

static LPCSTR	WfspRegistryKey = "Software\\SimonTatham\\PuTTY";
static LPCSTR	WfspRegistryKeyName = "PuTTY";
static LPCSTR	WfspRegistryKeyParent = "Software\\SimonTatham";
static LPCSTR	WfspRegistryKeyParentKey = "Software";
static LPCSTR	WfspRegistryKeyParentName = "SimonTatham";

static LPCSTR	WfspRegistrySubKeyHostCAs = "Software\\SimonTatham\\PuTTY\\SshHostCAs";
static LPCSTR	WfspRegistrySubKeyHostCAsName = "SshHostCAs";

static LPCSTR	WfspRegistrySubKeyHostKeys = "Software\\SimonTatham\\PuTTY\\SshHostKeys";
static LPCSTR	WfspRegistrySubKeyHostKeysName = "SshHostKeys";

static LPCSTR	WfspRegistrySubKeyJumpList = "Software\\SimonTatham\\PuTTY\\Jumplist";
static LPCSTR	WfspRegistrySubKeyJumpListName = "Jumplist";
static LPCSTR	WfspRegistryValueJumpList = "Recent sessions";

static LPCSTR	WfspRegistrySubKeyOptionsName = "Options";

static LPCSTR	WfspRegistrySubKeyPrivKeyList = "Software\\SimonTatham\\PuTTY\\PrivKeyList";
static LPCSTR	WfspRegistrySubKeyPrivKeyListName = "PrivKeyList";
static LPCSTR	WfspRegistryValuePrivKeyList = "Private key list";

static LPCSTR	WfspRegistrySubKeySessions = "Software\\SimonTatham\\PuTTY\\Sessions";
static LPCSTR	WfspRegistrySubKeySessionsName = "Sessions";

/*
 * External subroutine prototypes
 */

/* [see windows/jump-list.c] */
void	update_jumplist(void);
void	clear_jumplist_PuTTY(void);

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfspRegistryCleanupHostKeys(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrDeleteRegSubKey(true, WfspRegistryKey, WfspRegistrySubKeyHostKeysName);
}

WfrStatus
WfspRegistryClearHostKeys(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrClearRegSubKey(true, WfspRegistrySubKeyHostKeys);
}

WfrStatus
WfspRegistryDeleteHostKey(
	WfsBackend	backend,
	const char *	key_name
	)
{
	(void)backend;
	return WfrDeleteRegValue(true, WfspRegistrySubKeyHostKeys, key_name);
}

WfrStatus
WfspRegistryEnumerateHostKeys(
	WfsBackend	backend,
	bool		initfl,
	bool *		pdonefl,
	char **		pkey_name,
	void **		pstate
	)
{
	char *		item_name;
	WfrStatus	status;


	(void)backend;

	if (initfl) {
		return WfrEnumerateRegInit(
			(WfrEnumerateRegState **)pstate,
			WfspRegistrySubKeyHostKeys, NULL);
	}

	if (WFR_SUCCESS(status = WfrEnumerateRegValues(
			pdonefl, NULL, NULL, &item_name,
			NULL, (WfrEnumerateRegState **)pstate)))
	{
		if (!(*pdonefl)) {
			*pkey_name = item_name;
		}
	}

	return status;
}

WfrStatus
WfspRegistryLoadHostKey(
	WfsBackend	backend,
	const char *	key_name,
	const char **	pkey
	)
{
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfrLoadRegStringValue(
			WfspRegistrySubKeyHostKeys, key_name, (char **)pkey, NULL)))
	{
		status = WfsSetHostKey(backend, false, key_name, *pkey);
	}

	return status;
}

WfrStatus
WfspRegistryRenameHostKey(
	WfsBackend	backend,
	const char *	key_name,
	const char *	key_name_new
	)
{
	(void)backend;
	return WfrRenameRegValue(WfspRegistrySubKeyHostKeys, key_name, key_name_new);
}

WfrStatus
WfspRegistrySaveHostKey(
	WfsBackend	backend,
	const char *	key_name,
	const char *	key
	)
{
	(void)backend;
	return WfrSetRegValue(WfspRegistrySubKeyHostKeys, key_name, key, strlen(key), REG_SZ);
}


WfrStatus
WfspRegistryCleanupHostCAs(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrDeleteRegSubKey(true, WfspRegistryKey, WfspRegistrySubKeyHostCAsName);
}

WfrStatus
WfspRegistryClearHostCAs(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrClearRegSubKey(true, WfspRegistrySubKeyHostCAs);
}

WfrStatus
WfspRegistryCloseHostCA(
	WfsBackend	backend,
	WfsHostCA *	hca
	)
{
	return WfsDeleteHostCA(backend, false, hca->name);
}

WfrStatus
WfspRegistryDeleteHostCA(
	WfsBackend	backend,
	const char *	name
	)
{
	(void)backend;
	return WfrDeleteRegValue(true, WfspRegistrySubKeyHostCAs, name);
}

WfrStatus
WfspRegistryEnumerateHostCAs(
	WfsBackend	backend,
	bool		initfl,
	bool *		pdonefl,
	char **		pname,
	void **		pstate
	)
{
	char *		item_name;
	WfrStatus	status;


	(void)backend;

	if (initfl) {
		return WfrEnumerateRegInit(
			(WfrEnumerateRegState **)pstate,
			WfspRegistrySubKeyHostCAs, NULL);
	}

	if (WFR_SUCCESS(status = WfrEnumerateRegKeys(
			pdonefl, &item_name, (WfrEnumerateRegState **)pstate)))
	{
		if (!(*pdonefl)) {
			*pname = item_name;
		}
	}

	return status;
}

WfrStatus
WfspRegistryLoadHostCA(
	WfsBackend	backend,
	const char *	name,
	WfsHostCA **	phca
	)
{
	enum WfspFileLHCABits {
		WFSP_FILE_LHCA_BIT_CA_PUBLIC_KEY	= 0,
		WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA1	= 1,
		WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA256	= 2,
		WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA512	= 3,
		WFSP_FILE_LHCA_BIT_VALIDITY_EXPRESSION	= 4,
	};

	enum WfspFileLHCABits	bits = 0;
	WfsHostCA *		hca;
	WfsHostCA		hca_tmpl;
	WfrStatus		status;


	WFS_HOST_CA_INIT(hca_tmpl);

	status = WfrLoadRegSubKey(
		WfspRegistrySubKeyHostCAs, name, &hca_tmpl, &bits,
		WFR_LAMBDA(WfrStatus, (void *param1, void *param2, const char *key, int type, const void *value, size_t value_len) {
			WfsHostCA *			hca = (WfsHostCA *)param1;
			enum WfspFileLHCABits *		bits = (enum WfspFileLHCABits *)param2;
			WfrStatus			status;

			(void)value_len;
			if (	   (strcmp(key, "PublicKey") == 0) && (type == WFR_TREE_ITYPE_STRING)) {
				*bits |= WFSP_FILE_LHCA_BIT_CA_PUBLIC_KEY;
				hca->public_key = value;
			} else if ((strcmp(key, "PermitRSASHA1") == 0) && (type == WFR_TREE_ITYPE_INT)) {
				*bits |= WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA1;
				hca->permit_rsa_sha1 = *(int *)value;
				WFR_FREE(value);
			} else if ((strcmp(key, "PermitRSASHA256") == 0) && (type == WFR_TREE_ITYPE_INT)) {
				*bits |= WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA256;
				hca->permit_rsa_sha256 = *(int *)value;
				WFR_FREE(value);
			} else if ((strcmp(key, "PermitRSASHA512") == 0) && (type == WFR_TREE_ITYPE_INT)) {
				*bits |= WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA512;
				hca->permit_rsa_sha512 = *(int *)value;
				WFR_FREE(value);
			} else if ((strcmp(key, "Validity") == 0) && (type == WFR_TREE_ITYPE_STRING)) {
				*bits |= WFSP_FILE_LHCA_BIT_VALIDITY_EXPRESSION;
				hca->validity = value;
			} else {
				status = WFR_STATUS_FROM_ERRNO1(EINVAL);
			}

			return status;

		}));

	if (WFR_SUCCESS(status)) {
		if (bits !=
		    ( WFSP_FILE_LHCA_BIT_CA_PUBLIC_KEY
		    | WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA1
		    | WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA256
		    | WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA512
		    | WFSP_FILE_LHCA_BIT_VALIDITY_EXPRESSION))
		{
			status = WFR_STATUS_FROM_ERRNO1(EINVAL);
		} else {
			status = WfsAddHostCA(
				backend, hca_tmpl.public_key, 0, name,
				hca_tmpl.permit_rsa_sha1, hca_tmpl.permit_rsa_sha256,
				hca_tmpl.permit_rsa_sha512, hca_tmpl.validity, &hca);
		}
	}

	if (WFR_SUCCESS(status)) {
		if (phca) {
			*phca = hca;
		}
	} else if (WFR_FAILURE(status)) {
		WFR_FREE_IF_NOTNULL(hca_tmpl.public_key);
		WFR_FREE_IF_NOTNULL(hca_tmpl.name);
		WFR_FREE_IF_NOTNULL(hca_tmpl.validity);
	}

	return status;
}

WfrStatus
WfspRegistryRenameHostCA(
	WfsBackend	backend,
	const char *	name,
	const char *	name_new
	)
{
	(void)backend;
	return WfrRenameRegValue(WfspRegistrySubKeyHostCAs, name, name_new);
}

WfrStatus
WfspRegistrySaveHostCA(
	WfsBackend	backend,
	WfsHostCA *	hca
	)
{
	(void)backend;
	return WfrSaveToRegSubKeyV(
		WfspRegistrySubKeyHostCAs, hca->name,
		"PublicKey", WFR_TREE_ITYPE_STRING, hca->public_key,
		"PermitRSASHA1", WFR_TREE_ITYPE_INT, &hca->permit_rsa_sha1,
		"PermitRSASHA256", WFR_TREE_ITYPE_INT, &hca->permit_rsa_sha256,
		"PermitRSASHA512", WFR_TREE_ITYPE_INT, &hca->permit_rsa_sha512,
		"Validity", WFR_TREE_ITYPE_STRING, hca->validity,
		NULL);
}


WfrStatus
WfspRegistryClearOptions(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrDeleteRegSubKey(true, WfspRegistryKey, WfspRegistrySubKeyOptionsName);
}

WfrStatus
WfspRegistryLoadOptions(
	WfsBackend	backend
	)
{
	(void)backend;

	return WfrLoadRegSubKey(
		WfspRegistryKey, WfspRegistrySubKeyOptionsName, &backend, NULL,
		WFR_LAMBDA(WfrStatus, (void *param1, void *param2, const char *key, int type, const void *value, size_t value_len) {
			(void)param2;
			return WfsSetOption(*(WfsBackend *)param1, false, false, key, value, value_len, type);
		}));
}

WfrStatus
WfspRegistrySaveOptions(
	WfsBackend	backend,
	WfrTree *	backend_tree
	)
{
	(void)backend;
	return WfrSaveTreeToRegSubKey(
		WfspRegistryKey,
		WfspRegistrySubKeyOptionsName,
		backend_tree);
}


WfrStatus
WfspRegistryCleanupSessions(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrDeleteRegSubKey(true, WfspRegistryKey, WfspRegistrySubKeySessionsName);
}

WfrStatus
WfspRegistryClearSessions(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrClearRegSubKey(true, WfspRegistrySubKeySessions);
}

WfrStatus
WfspRegistryCloseSession(
	WfsBackend	backend,
	WfsSession *	session
	)
{
	return WfsDeleteSession(backend, false, session->name);
}

WfrStatus
WfspRegistryDeleteSession(
	WfsBackend	backend,
	const char *	sessionname
	)
{
	(void)backend;
	return WfrDeleteRegSubKey(true, WfspRegistrySubKeySessions, sessionname);
}

WfrStatus
WfspRegistryEnumerateSessions(
	WfsBackend	backend,
	bool		initfl,
	bool *		pdonefl,
	char **		psessionname,
	void **		pstate
	)
{
	char *		item_name;
	WfrStatus	status;


	(void)backend;

	if (initfl) {
		return WfrEnumerateRegInit(
			(WfrEnumerateRegState **)pstate,
			WfspRegistrySubKeySessions, NULL);
	}

	if (WFR_SUCCESS(status = WfrEnumerateRegKeys(
			pdonefl, &item_name, (WfrEnumerateRegState **)pstate)))
	{
		if (!(*pdonefl)) {
			*psessionname = item_name;
		}
	}

	return status;
}

WfrStatus
WfspRegistryLoadSession(
	WfsBackend	backend,
	const char *	sessionname,
	WfsSession **	psession
	)
{
	bool			addedfl = false;
	WfsSession *		session = NULL;
	WfrStatus		status;


	status = WfsGetSession(backend, true, sessionname, &session);
	if (WFR_SUCCESS(status)) {
		status = WfsClearSession(backend, session, sessionname);
		addedfl = false;
	} else if (WFR_STATUS_IS_NOT_FOUND(status)) {
		status = WfsAddSession(backend, sessionname, &session);
		addedfl = WFR_SUCCESS(status);
	}

	if (WFR_SUCCESS(status)) {
		status = WfrLoadRegSubKey(
			WfspRegistrySubKeySessions, sessionname, session, NULL,
			WFR_LAMBDA(WfrStatus, (void *param1, void *param2, const char *key, int type, const void *value, size_t value_len) {
				(void)param2;
				return WfsSetSessionKey((WfsSession *)param1, key, (void *)value, value_len, type);
			}));
	}

	if (WFR_SUCCESS(status) && psession) {
		*psession = session;
	} else if (WFR_FAILURE(status) && addedfl && session) {
		(void)WfsDeleteSession(backend, false, sessionname);
	}

	return status;
}

WfrStatus
WfspRegistryRenameSession(
	WfsBackend	backend,
	const char *	sessionname,
	const char *	sessionname_new
	)
{
	(void)backend;
	return WfrRenameRegSubKey(WfspRegistrySubKeySessions, sessionname, sessionname_new);
}

WfrStatus
WfspRegistrySaveSession(
	WfsBackend	backend,
	WfsSession *	session
	)
{
	(void)backend;
	return WfrSaveTreeToRegSubKey(WfspRegistrySubKeySessions, session->name, session->tree);
}


void
WfspRegistryAddJumpList(
	const char *const	sessionname
	)
{
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfrUpdateRegStringList(
			true, false, WfspRegistrySubKeyJumpList,
			sessionname, WfspRegistryValueJumpList)))
	{
		update_jumplist();
	} else {
		/* Make sure we don't leave the jumplist dangling. */
		WfsClearJumpList(WfsGetBackend());
	}
}

WfrStatus
WfspRegistryCleanupJumpList(
	void
	)
{
	return WfrDeleteRegSubKey(true, WfspRegistryKey, WfspRegistrySubKeyJumpListName);
}

void
WfspRegistryClearJumpList(
	void
	)
{
	clear_jumplist_PuTTY();
}

WfrStatus
WfspRegistryGetEntriesJumpList(
	char **		pjump_list,
	size_t *	pjump_list_size
	)
{
	DWORD		jump_list_size_;
	WfrStatus	status;


	status = WfrGetRegStringListValue(
		WfspRegistrySubKeyJumpList,
		WfspRegistryValueJumpList,
		NULL, pjump_list, &jump_list_size_);
	if (WFR_FAILURE(status)) {
		if (WFR_NEWN(status, (*pjump_list), 2, char)) {
			(*pjump_list)[0] = '\0';
			(*pjump_list)[1] = '\0';
			if (pjump_list_size) {
				*pjump_list_size = 2;
			}
		}
	} else {
		if (pjump_list_size) {
			*pjump_list_size = (size_t)jump_list_size_;
		}
	}

	return status;
}

void
WfspRegistryRemoveJumpList(
	const char *const	sessionname
	)
{
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfrUpdateRegStringList(
			false, true, WfspRegistrySubKeyJumpList,
			sessionname, WfspRegistryValueJumpList))) {
		update_jumplist();
	} else {
		/* Make sure we don't leave the jumplist dangling. */
		WfsClearJumpList(WfsGetBackend());
	}
}

WfrStatus
WfspRegistrySetEntriesJumpList(
	const char *	jump_list,
	size_t		jump_list_size
	)
{
	return WfrSetRegValue(
		WfspRegistrySubKeyJumpList,
		WfspRegistryValueJumpList,
		jump_list, jump_list_size,
		REG_MULTI_SZ);
}


WfrStatus
WfspRegistryAddPrivKeyList(
	const char *const	privkey_name
	)
{
	return WfrUpdateRegStringList(
		true, false, WfspRegistrySubKeyPrivKeyList,
		privkey_name, WfspRegistryValuePrivKeyList);
}

WfrStatus
WfspRegistryCleanupPrivKeyList(
	void
	)
{
	return WfrDeleteRegSubKey(true, WfspRegistryKey, WfspRegistrySubKeyPrivKeyListName);
}

WfrStatus
WfspRegistryClearPrivKeyList(
	void
	)
{
	return WfspRegistryCleanupPrivKeyList();
}

WfrStatus
WfspRegistryGetEntriesPrivKeyList(
	char **		pprivkey_list,
	size_t *	pprivkey_list_size
	)
{
	DWORD		privkey_list_size_;
	WfrStatus	status;


	status = WfrGetRegStringListValue(
		WfspRegistrySubKeyPrivKeyList,
		WfspRegistryValuePrivKeyList,
		NULL, pprivkey_list, &privkey_list_size_);
	if (WFR_FAILURE(status)) {
		if (WFR_STATUS_IS_NOT_FOUND(status)
		&&  WFR_NEWN(status, (*pprivkey_list), 2, char))
		{
			(*pprivkey_list)[0] = '\0';
			(*pprivkey_list)[1] = '\0';
			if (pprivkey_list_size) {
				*pprivkey_list_size = 2;
			}
		}
	} else {
		if (pprivkey_list_size) {
			*pprivkey_list_size = (size_t)privkey_list_size_;
		}
	}

	return status;
}

WfrStatus
WfspRegistryRemovePrivKeyList(
	const char *const	privkey_name
	)
{
	return WfrUpdateRegStringList(
		false, true, WfspRegistrySubKeyPrivKeyList,
		privkey_name, WfspRegistryValuePrivKeyList);
}

WfrStatus
WfspRegistrySetEntriesPrivKeyList(
	const char *	privkey_list,
	size_t		privkey_list_size
	)
{
	return WfrSetRegValue(
		WfspRegistrySubKeyPrivKeyList,
		WfspRegistryValuePrivKeyList,
		privkey_list, privkey_list_size,
		REG_MULTI_SZ);
}


WfrStatus
WfspRegistryCleanupContainer(
	WfsBackend	backend
	)
{
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	(void)backend;

	if (WFR_SUCCESS(status)) {
		status = WfrDeleteRegSubKey(true, WfspRegistryKeyParent, WfspRegistryKeyName);
		if (WFR_STATUS_IS_NOT_FOUND(status)) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	if (WFR_SUCCESS(status)) {
		status = WfrDeleteRegSubKey(true, WfspRegistryKeyParentKey, WfspRegistryKeyParentName);
		if (WFR_STATUS_IS_NOT_FOUND(status)) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	return status;
}

WfrStatus
WfspRegistryEnumerateCancel(
	WfsBackend	backend,
	void **		pstate
	)
{
	(void)backend;
	WfrEnumerateRegCancel((WfrEnumerateRegState **)pstate);
	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspRegistryInit(
	void
	)
{
	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspRegistrySetBackend(
	WfsBackend	backend_new,
	char *		args_extra
	)
{
	WfrStatus	status;


	WFR_FREE_IF_NOTNULL(args_extra);

	if (WFR_SUCCESS(status = WfsClearHostCAs(backend_new, false))
	&&  WFR_SUCCESS(status = WfsClearHostKeys(backend_new, false))
	&&  WFR_SUCCESS(status = WfsClearSessions(backend_new, false)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
