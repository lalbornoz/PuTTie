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
 * Names of registry subkeys/value containing PuTTie/PuTTy's top-level keys,
 * host CAs, host keys, the jump list, global options, the private key list,
 * and sessions, and the value names of the jump list and the private key list,
 * resp. [see windows/storage.c]
 */

static LPCSTR	WfspRegistryKey = "Software\\SimonTatham\\PuTTY";
static LPCSTR	WfspRegistryKeyParent = "Software\\SimonTatham";
static LPCSTR	WfspRegistrySubKeyHostCAs = "Software\\SimonTatham\\PuTTY\\SshHostCAs";
static LPCSTR	WfspRegistrySubKeyHostCAsName = "SshHostCAs";
static LPCSTR	WfspRegistrySubKeyHostKeys = "Software\\SimonTatham\\PuTTY\\SshHostKeys";
static LPCSTR	WfspRegistrySubKeyHostKeysName = "SshHostKeys";
static LPCSTR	WfspRegistrySubKeyJumpList = "Software\\SimonTatham\\PuTTY\\Jumplist";
static LPCSTR	WfspRegistrySubKeyJumpListName = "Jumplist";
static LPCSTR	WfspRegistrySubKeyOptionsName = "Options";
static LPCSTR	WfspRegistrySubKeyPrivKeyList = "Software\\SimonTatham\\PuTTY\\PrivKeyList";
static LPCSTR	WfspRegistrySubKeyPrivKeyListName = "PrivKeyList";
static LPCSTR	WfspRegistrySubKeySessions = "Software\\SimonTatham\\PuTTY\\Sessions";
static LPCSTR	WfspRegistrySubKeySessionsName = "Sessions";
static LPCSTR	WfspRegistryValueJumpList = "Recent sessions";
static LPCSTR	WfspRegistryValuePrivKeyList = "Private key list";

/*
 * External subroutine prototypes
 */

/* [see windows/jump-list.c] */
void			update_jumplist(void);
void			clear_jumplist_PuTTY(void);

/*
 * Private subroutine prototypes
 */

static WfrStatus	WfsppRegistryGetList(const char *subkey, const char *value_name, HKEY *phKey, char **plist, DWORD *plist_size);
static WfrStatus	WfsppRegistryTransformList(bool addfl, bool delfl, const char *subkey, const char *const trans_item, const char *value_name);

/*
 * Private subroutines
 */

static WfrStatus
WfsppRegistryGetList(
	const char *	subkey,
	const char *	value_name,
	HKEY *		phKey,
	char **		plist,
	DWORD *		plist_size
	)
{
	HKEY		hKey = NULL;
	char *		list = NULL;
	DWORD		list_size = 0;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfrOpenRegKeyRo(
			HKEY_CURRENT_USER, &hKey, subkey))
	&&  WFR_STATUS_SUCCESS(status = WFR_STATUS_BIND_LSTATUS(RegGetValue(
			hKey, NULL, value_name, RRF_RT_REG_MULTI_SZ,
			NULL, NULL, &list_size))))
	{
		if (list_size < 2) {
			status = WFR_STATUS_FROM_ERRNO1(ENOENT);
		} else if (!(list = WFR_NEWN(list_size, char))) {
			status = WFR_STATUS_FROM_ERRNO();
		} else if (WFR_STATUS_SUCCESS(status = WFR_STATUS_BIND_LSTATUS(RegGetValue(
				hKey, NULL, value_name, RRF_RT_REG_MULTI_SZ,
				NULL, list, &list_size))))
		{
			list[list_size - 2] = '\0';
			list[list_size - 1] = '\0';

			*plist = list;
			if (plist_size) {
				*plist_size = list_size;
			}
		}
	}

	if (WFR_STATUS_FAILURE(status)) {
		WFR_FREE_IF_NOTNULL(list);
	}

	if (hKey) {
		if (phKey) {
			*phKey = hKey;
		} else {
			(void)RegCloseKey(hKey);
		}
	}

	return status;
}

static WfrStatus
WfsppRegistryTransformList(
	bool			addfl,
	bool			delfl,
	const char *		subkey,
	const char *const	trans_item,
	const char *		value_name
	)
{
	HKEY		hKey = NULL;
	char *		list = NULL;
	DWORD		list_size;
	size_t		list_size_;
	WfrStatus	status;


	if (addfl || delfl) {
		if (WFR_STATUS_FAILURE(status = WfsppRegistryGetList(
				subkey, value_name, &hKey, &list, &list_size)))
		{
			list = NULL;
			list_size_ = 0;
		} else {
			list_size_ = list_size;
		}

		if (WFR_STATUS_SUCCESS(status = WfsTransformList(
				addfl, delfl, &list, &list_size_, trans_item)))
		{
			status = WFR_STATUS_BIND_LSTATUS(RegSetValueEx(
				hKey, value_name, 0, REG_MULTI_SZ,
				(const BYTE *)list, (DWORD)list_size_));
		}
	}

	if (hKey != NULL) {
		(void)RegCloseKey(hKey);
	}
	WFR_FREE_IF_NOTNULL(list);

	return status;
}

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfspRegistryCleanupHostKeys(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrCleanupRegSubKey(WfspRegistryKey, WfspRegistrySubKeyHostKeysName);
}

WfrStatus
WfspRegistryClearHostKeys(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrClearRegSubKey(WfspRegistrySubKeyHostKeys);
}

WfrStatus
WfspRegistryDeleteHostKey(
	WfsBackend	backend,
	const char *	key_name
	)
{
	(void)backend;
	return WfrDeleteRegValue(WfspRegistrySubKeyHostKeys, key_name);
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

	if (WFR_STATUS_SUCCESS(status = WfrEnumerateRegValues(
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


	if (WFR_STATUS_SUCCESS(status = WfrLoadRegValue(
			WfspRegistrySubKeyHostKeys, key_name, (char **)pkey, NULL)))
	{
		status = WfsSetHostKey(backend, key_name, *pkey);
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
	return WfrSetRegValue(WfspRegistrySubKeyHostKeys, key_name, key, strlen(key));
}


WfrStatus
WfspRegistryCleanupHostCAs(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrCleanupRegSubKey(WfspRegistryKey, WfspRegistrySubKeyHostCAsName);
}

WfrStatus
WfspRegistryClearHostCAs(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrClearRegSubKey(WfspRegistrySubKeyHostCAs);
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
	return WfrDeleteRegValue(WfspRegistrySubKeyHostCAs, name);
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

	if (WFR_STATUS_SUCCESS(status = WfrEnumerateRegKeys(
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

	if (WFR_STATUS_SUCCESS(status)) {
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

	if (WFR_STATUS_SUCCESS(status)) {
		if (phca) {
			*phca = hca;
		}
	} else if (WFR_STATUS_FAILURE(status)) {
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
	return WfrCleanupRegSubKey(WfspRegistryKey, WfspRegistrySubKeyOptionsName);
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
			return WfsSetOption(*(WfsBackend *)param1, false, key, value, value_len, type);
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
	return WfrCleanupRegSubKey(WfspRegistryKey, WfspRegistrySubKeySessionsName);
}

WfrStatus
WfspRegistryClearSessions(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrClearRegSubKey(WfspRegistrySubKeySessions);
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
	return WfrDeleteRegSubKey(WfspRegistrySubKeySessions, sessionname);
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

	if (WFR_STATUS_SUCCESS(status = WfrEnumerateRegKeys(
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
	if (WFR_STATUS_SUCCESS(status)) {
		status = WfsClearSession(backend, session, sessionname);
		addedfl = false;
	} else if (WFR_STATUS_CONDITION(status) == ENOENT) {
		status = WfsAddSession(backend, sessionname, &session);
		addedfl = WFR_STATUS_SUCCESS(status);
	}

	if (WFR_STATUS_SUCCESS(status)) {
		status = WfrLoadRegSubKey(
			WfspRegistrySubKeySessions, sessionname, session, NULL,
			WFR_LAMBDA(WfrStatus, (void *param1, void *param2, const char *key, int type, const void *value, size_t value_len) {
				(void)param2;
				return WfsSetSessionKey((WfsSession *)param1, key, (void *)value, value_len, type);
			}));
	}

	if (WFR_STATUS_SUCCESS(status) && psession) {
		*psession = session;
	} else if (WFR_STATUS_FAILURE(status) && addedfl && session) {
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


	if (WFR_STATUS_SUCCESS(status = WfsppRegistryTransformList(
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
	return WfrCleanupRegSubKey(WfspRegistryKey, WfspRegistrySubKeyJumpListName);
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


	status = WfsppRegistryGetList(
		WfspRegistrySubKeyJumpList,
		WfspRegistryValueJumpList,
		NULL, pjump_list, &jump_list_size_);
	if (WFR_STATUS_FAILURE(status)) {
		if ((WFR_STATUS_CONDITION(status) == ERROR_FILE_NOT_FOUND)
		||  (WFR_STATUS_CONDITION(status) == ERROR_PATH_NOT_FOUND))
		{
			if (!((*pjump_list = WFR_NEWN(2, char)))) {
				status = WFR_STATUS_FROM_ERRNO();
			} else {
				(*pjump_list)[0] = '\0';
				(*pjump_list)[1] = '\0';
				if (pjump_list_size) {
					*pjump_list_size = 2;
				}
				status = WFR_STATUS_CONDITION_SUCCESS;
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


	if (WFR_STATUS_SUCCESS(status = WfsppRegistryTransformList(
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
	HKEY		hKey;
	WfrStatus	status;


	(void)jump_list_size;
	if (WFR_STATUS_SUCCESS(status = WfrCreateRegKey(HKEY_CURRENT_USER, &hKey, WfspRegistrySubKeyJumpList))
	&&  WFR_STATUS_SUCCESS(status = WFR_STATUS_BIND_LSTATUS(RegSetValueEx(
			hKey, WfspRegistryValueJumpList, 0,
			REG_MULTI_SZ, (const BYTE *)jump_list,
			jump_list_size))))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}


WfrStatus
WfspRegistryAddPrivKeyList(
	const char *const	privkey_name
	)
{
	return WfsppRegistryTransformList(
		true, false, WfspRegistrySubKeyPrivKeyList,
		privkey_name, WfspRegistryValuePrivKeyList);
}

WfrStatus
WfspRegistryCleanupPrivKeyList(
	void
	)
{
	HKEY		hKey = NULL;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfrOpenRegKeyRw(HKEY_CURRENT_USER, &hKey, WfspRegistryKey))
	&&  WFR_STATUS_SUCCESS(status = WFR_STATUS_BIND_LSTATUS(RegDeleteTree(hKey, WfspRegistrySubKeyPrivKeyListName))))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_FAILURE(status)
	&&  ((WFR_STATUS_CONDITION(status) == ERROR_FILE_NOT_FOUND)
	||   (WFR_STATUS_CONDITION(status) == ERROR_PATH_NOT_FOUND)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}
	if (hKey) {
		(void)RegCloseKey(hKey);
	}

	return status;
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


	status = WfsppRegistryGetList(
		WfspRegistrySubKeyPrivKeyList,
		WfspRegistryValuePrivKeyList,
		NULL, pprivkey_list, &privkey_list_size_);
	if (WFR_STATUS_FAILURE(status)) {
		if ((WFR_STATUS_CONDITION(status) == ERROR_FILE_NOT_FOUND)
		||  (WFR_STATUS_CONDITION(status) == ERROR_PATH_NOT_FOUND))
		{
			if (!((*pprivkey_list = WFR_NEWN(2, char)))) {
				status = WFR_STATUS_FROM_ERRNO();
			} else {
				(*pprivkey_list)[0] = '\0';
				(*pprivkey_list)[1] = '\0';
				if (pprivkey_list_size) {
					*pprivkey_list_size = 2;
				}
				status = WFR_STATUS_CONDITION_SUCCESS;
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
	return WfsppRegistryTransformList(
		true, false, WfspRegistrySubKeyPrivKeyList,
		privkey_name, WfspRegistryValuePrivKeyList);
}

WfrStatus
WfspRegistrySetEntriesPrivKeyList(
	const char *	privkey_list,
	size_t		privkey_list_size
	)
{
	HKEY		hKey;
	WfrStatus	status;


	(void)privkey_list_size;
	if (WFR_STATUS_SUCCESS(status = WfrCreateRegKey(HKEY_CURRENT_USER, &hKey, WfspRegistrySubKeyPrivKeyList))
	&&  WFR_STATUS_SUCCESS(status = WFR_STATUS_BIND_LSTATUS(RegSetValueEx(
			hKey, WfspRegistryValuePrivKeyList, 0,
			REG_MULTI_SZ, (const BYTE *)privkey_list,
			privkey_list_size))))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}


WfrStatus
WfspRegistryCleanupContainer(
	WfsBackend	backend
	)
{
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	(void)backend;

	if (WFR_STATUS_SUCCESS(status)) {
		status = WFR_STATUS_BIND_LSTATUS(RegDeleteTree(HKEY_CURRENT_USER, WfspRegistryKey));
		if (WFR_STATUS_FAILURE(status)
		&&  ((WFR_STATUS_CONDITION(status) == ERROR_FILE_NOT_FOUND)
		||   (WFR_STATUS_CONDITION(status) == ERROR_PATH_NOT_FOUND)))
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	if (WFR_STATUS_SUCCESS(status)) {
		status = WFR_STATUS_BIND_LSTATUS(RegDeleteTree(HKEY_CURRENT_USER, WfspRegistryKeyParent));
		if (WFR_STATUS_FAILURE(status)
		&&  ((WFR_STATUS_CONDITION(status) == ERROR_FILE_NOT_FOUND)
		||   (WFR_STATUS_CONDITION(status) == ERROR_PATH_NOT_FOUND)))
		{
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
	WfsBackend	backend_new
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsClearHostCAs(backend_new, false))
	&&  WFR_STATUS_SUCCESS(status = WfsClearHostKeys(backend_new, false))
	&&  WFR_STATUS_SUCCESS(status = WfsClearSessions(backend_new, false)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
