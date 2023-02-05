/*
 * winfrip_storage_backend_file.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <windows.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_file.h"
#include "PuTTie/winfrip_rtl_load.h"
#include "PuTTie/winfrip_rtl_pcre2.h"
#include "PuTTie/winfrip_rtl_save.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_host_keys.h"
#include "PuTTie/winfrip_storage_jump_list.h"
#include "PuTTie/winfrip_storage_options.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"
#include "PuTTie/winfrip_storage_backend_file.h"

/*
 * Private variables
 */

/*
 * WfsppFileAppData: absolute pathnamne to the current user's application data directory (%APPDATA%)
 * WfsppFileDname: absolute pathname to the current user's PuTTie base directory
 * WfsppFileDnameHostKeys: absolute pathname to the current user's directory of PuTTie host key files
 * WfsppFileDnameSessions: absolute pathname to the current user's directory of PuTTie session files
 * WfsppFileExtHostKeys: file name extension of PuTTie host key files
 * WfsppFileExtSessions: file name extension of PuTTie session files
 * WfsppFileFnameJumpList{,Tmp}: absolute pathname to the current user's PuTTie jump list file
 * WfsppFileFnameOptions: absolute pathname to the current user's PuTTie global options file
 * WfsppFileFnamePrivKeyList{,Tmp}: absolute pathname to the current user's Pageant private key list file
 */

static char *		WfsppFileAppData = NULL;
static char		WfsppFileDname[MAX_PATH + 1] = "";
static char		WfsppFileDnameHostCAs[MAX_PATH + 1] = "";
static char		WfsppFileDnameHostKeys[MAX_PATH + 1] = "";
static char		WfsppFileDnameSessions[MAX_PATH + 1] = "";
static char		WfsppFileExtHostCAs[] = ".hostca";
static char		WfsppFileExtHostKeys[] = ".hostkey";
static char		WfsppFileExtSessions[] = ".ini";
static char		WfsppFileFnameJumpList[MAX_PATH + 1] = "";
static char		WfsppFileFnameJumpListTmp[MAX_PATH + 1] = "";
static char		WfsppFileFnameOptions[MAX_PATH + 1] = "";
static char		WfsppFileFnamePrivKeyList[MAX_PATH + 1] = "";
static char		WfsppFileFnamePrivKeyListTmp[MAX_PATH + 1] = "";

/*
 * External subroutine prototypes
 */

/* [see windows/jump-list.c] */
void			update_jumplist(void);
void			clear_jumplist_PuTTY(void);

/*
 * Private subroutine prototypes
 */

static WfrStatus	WfsppFileInitAppDataSubdir(void);
static WfrStatus	WfsppFileTransformList(bool addfl, bool delfl, const char *fname, const char *fname_tmp, const char *const trans_item);

/*
 * Private subroutines
 */

static WfrStatus
WfsppFileInitAppDataSubdir(
	void
	)
{
	char *		appdata;
	WfrStatus	status;


	if (!(appdata = getenv("APPDATA"))) {
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);
	} else {
		WfsppFileAppData = appdata;
		WFR_SNPRINTF(
			WfsppFileDname, sizeof(WfsppFileDname),
			"%s/PuTTie", WfsppFileAppData);
		WFR_SNPRINTF(
			WfsppFileDnameHostCAs, sizeof(WfsppFileDnameHostCAs),
			"%s/hostcas", WfsppFileDname);
		WFR_SNPRINTF(
			WfsppFileDnameHostKeys, sizeof(WfsppFileDnameHostKeys),
			"%s/hostkeys", WfsppFileDname);
		WFR_SNPRINTF(
			WfsppFileDnameSessions, sizeof(WfsppFileDnameSessions),
			"%s/sessions", WfsppFileDname);
		WFR_SNPRINTF(
			WfsppFileFnameOptions, sizeof(WfsppFileFnameOptions),
			"%s/options.ini", WfsppFileDname);
		WFR_SNPRINTF(
			WfsppFileFnameJumpList, sizeof(WfsppFileFnameJumpList),
			"%s/jump.list", WfsppFileDname);
		WFR_SNPRINTF(
			WfsppFileFnameJumpListTmp, sizeof(WfsppFileFnameJumpListTmp),
			"jump.list.XXXXXX");
		WFR_SNPRINTF(
			WfsppFileFnamePrivKeyList, sizeof(WfsppFileFnamePrivKeyList),
			"%s/privkey.list", WfsppFileDname);
		WFR_SNPRINTF(
			WfsppFileFnamePrivKeyListTmp, sizeof(WfsppFileFnamePrivKeyListTmp),
			"privkey.list.XXXXXX");

		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

static WfrStatus
WfsppFileTransformList(
	bool			addfl,
	bool			delfl,
	const char *		fname,
	const char *		fname_tmp,
	const char *const	trans_item
	)
{
	char *		list = NULL;
	size_t		list_size;
	WfrStatus	status;


	if (addfl || delfl) {
		if (WFR_STATUS_FAILURE(status = WfrLoadListFromFile(
				fname, &list, &list_size)))
		{
			list = NULL;
			list_size = 0;
		}

		if (WFR_STATUS_SUCCESS(status = WfsTransformList(
				addfl, delfl, &list,
				&list_size, trans_item)))
		{
			status = WfrSaveListToFile(fname, fname_tmp, list, list_size);
		}
	}

	WFR_FREE_IF_NOTNULL(list);

	return status;
}

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfspFileCleanupHostCAs(
	WfsBackend	backend
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfspFileClearHostCAs(backend))) {
		status = WfrDeleteDirectory(WfsppFileDnameHostCAs, true, true);
	}

	return status;
}

WfrStatus
WfspFileClearHostCAs(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrDeleteFiles(WfsppFileDnameHostCAs, WfsppFileExtHostCAs);
}

WfrStatus
WfspFileCloseHostCA(
	WfsBackend	backend,
	WfsHostCA *	hca
	)
{
	(void)backend;
	(void)hca;

	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspFileDeleteHostCA(
	WfsBackend	backend,
	const char *	name
	)
{
	(void)backend;
	return WfrDeleteFile(
		true, WfsppFileDnameHostCAs,
		WfsppFileExtHostCAs, name);
}

WfrStatus
WfspFileEnumerateHostCAs(
	WfsBackend	backend,
	bool		initfl,
	bool *		pdonefl,
	char **		pname,
	void **		pstate
	)
{
	const char *	name;
	WfrStatus	status;


	(void)backend;

	if (initfl) {
		return WfrEnumerateFilesInit(
			WfsppFileDnameHostCAs,
			(WfrEnumerateFilesState **)pstate);
	}

	if (WFR_STATUS_SUCCESS(status = WfrEnumerateFiles(
			WfsppFileExtHostCAs, pdonefl,
			&name, (WfrEnumerateFilesState **)pstate)))
	{
		if (!(*pdonefl)) {
			status = WfrUnescapeFileName((char *)name, (const char **)pname);
		}
	}

	return status;
}

WfrStatus
WfspFileLoadHostCA(
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
	char *			hca_data = NULL;
	size_t			hca_data_size;
	WfsHostCA		hca_tmpl;
	time_t			mtime;
	WfrStatus		status;


	WFS_HOST_CA_INIT(hca_tmpl);
	if (WFR_STATUS_SUCCESS(status = WfrLoadRawFile(
			true, WfsppFileDnameHostCAs, WfsppFileExtHostCAs,
			name, &hca_data, &hca_data_size, &mtime)))
	{
		if (WFR_STATUS_SUCCESS(status = WfsGetHostCA(backend, true, name, &hca))) {
			if (hca->mtime == mtime) {
				goto out;
			} else {
				hca = NULL;
			}
		}

		status = WfrLoadParse(
			hca_data, hca_data_size, &hca_tmpl, &bits,
			WFR_LAMBDA(WfrStatus, (void *param1, void *param2, const char *key, int type, const void *value, size_t value_size) {
				WfsHostCA *			hca = (WfsHostCA *)param1;
				enum WfspFileLHCABits *		bits = (enum WfspFileLHCABits *)param2;
				WfrStatus			status;

				(void)value_size;
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
	}

out:
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
				backend, hca_tmpl.public_key, mtime, name,
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

	WFR_FREE_IF_NOTNULL(hca_data);

	return status;
}

WfrStatus
WfspFileRenameHostCA(
	WfsBackend	backend,
	const char *	name,
	const char *	name_new
	)
{
	(void)backend;
	return WfrRenameFile(
		true, WfsppFileDnameHostCAs,
		WfsppFileExtHostCAs, name, name_new);
}

WfrStatus
WfspFileSaveHostCA(
	WfsBackend	backend,
	WfsHostCA *	hca
	)
{
	(void)backend;
	return WfrSaveToFileV(
		true, WfsppFileDnameHostCAs, WfsppFileExtHostCAs,
		hca->name,
		"PublicKey", WFR_TREE_ITYPE_STRING, hca->public_key,
		"PermitRSASHA1", WFR_TREE_ITYPE_INT, &hca->permit_rsa_sha1,
		"PermitRSASHA256", WFR_TREE_ITYPE_INT, &hca->permit_rsa_sha256,
		"PermitRSASHA512", WFR_TREE_ITYPE_INT, &hca->permit_rsa_sha512,
		"Validity", WFR_TREE_ITYPE_STRING, hca->validity,
		NULL);
}


WfrStatus
WfspFileCleanupHostKeys(
	WfsBackend	backend
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfspFileClearHostKeys(backend))) {
		status = WfrDeleteDirectory(WfsppFileDnameHostKeys, true, true);
	}

	return status;
}

WfrStatus
WfspFileClearHostKeys(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrDeleteFiles(WfsppFileDnameHostKeys, WfsppFileExtHostKeys);
}

WfrStatus
WfspFileDeleteHostKey(
	WfsBackend	backend,
	const char *	key_name
	)
{
	(void)backend;
	return WfrDeleteFile(
		true, WfsppFileDnameHostKeys,
		WfsppFileExtHostKeys, key_name);
}

WfrStatus
WfspFileEnumerateHostKeys(
	WfsBackend	backend,
	bool		initfl,
	bool *		pdonefl,
	char **		pkey_name,
	void **		pstate
	)
{
	const char *	name;
	WfrStatus	status;


	(void)backend;

	if (initfl) {
		return WfrEnumerateFilesInit(
			WfsppFileDnameHostKeys,
			(WfrEnumerateFilesState **)pstate);
	}

	if (WFR_STATUS_SUCCESS(status = WfrEnumerateFiles(
			WfsppFileExtHostKeys, pdonefl,
			&name, (WfrEnumerateFilesState **)pstate)))
	{
		if (!(*pdonefl)) {
			status = WfrUnescapeFileName((char *)name, (const char **)pkey_name);
		}
	}

	return status;
}

WfrStatus
WfspFileLoadHostKey(
	WfsBackend	backend,
	const char *	key_name,
	const char **	pkey
	)
{
	char *		key;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfrLoadRawFile(
			true, WfsppFileDnameHostKeys,
			WfsppFileExtHostKeys, key_name,
			&key, NULL, NULL)))
	{
		if (WFR_STATUS_SUCCESS(status = WfsSetHostKey(backend, key_name, key))) {
			*pkey = key;
		} else {
			WFR_FREE(key);
		}
	}

	return status;
}

WfrStatus
WfspFileRenameHostKey(
	WfsBackend	backend,
	const char *	key_name,
	const char *	key_name_new
	)
{
	(void)backend;
	return WfrRenameFile(
		true, WfsppFileDnameHostKeys, WfsppFileExtHostKeys,
		key_name, key_name_new);
}

WfrStatus
WfspFileSaveHostKey(
	WfsBackend	backend,
	const char *	key_name,
	const char *	key
	)
{
	(void)backend;
	return WfrSaveRawFile(
		true, true, WfsppFileDnameHostKeys,
		WfsppFileExtHostKeys, key_name, key,
		strlen(key));
}


WfrStatus
WfspFileClearOptions(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrDeleteFile(false, NULL, NULL, WfsppFileFnameOptions);
}

WfrStatus
WfspFileLoadOptions(
	WfsBackend	backend
	)
{
	char *			options_data = NULL;
	size_t			options_data_size;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WfrLoadRawFile(
			false, NULL, NULL, WfsppFileFnameOptions,
			&options_data, &options_data_size, NULL)))
	{
		status = WfrLoadParse(
			options_data, options_data_size, &backend, NULL,
			WFR_LAMBDA(WfrStatus, (void *param1, void *param2, const char *key, int type, const void *value, size_t value_size) {
				(void)param2;
				return WfsSetOption(*(WfsBackend *)param1, key, value, value_size, type);
			}));
	}

	WFR_FREE_IF_NOTNULL(options_data);

	return status;
}

WfrStatus
WfspFileSaveOptions(
	WfsBackend	backend,
	WfrTree *	backend_tree
	)
{
	(void)backend;
	return WfrSaveTreeToFile(false, NULL, NULL, WfsppFileFnameOptions, backend_tree);
}


WfrStatus
WfspFileCleanupSessions(
	WfsBackend	backend
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfspFileClearSessions(backend))) {
		status = WfrDeleteDirectory(WfsppFileDnameSessions, true, true);
	}

	return status;
}

WfrStatus
WfspFileClearSessions(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrDeleteFiles(WfsppFileDnameSessions, WfsppFileExtSessions);
}

WfrStatus
WfspFileCloseSession(
	WfsBackend	backend,
	WfsSession *	session
	)
{
	(void)backend;
	(void)session;

	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspFileDeleteSession(
	WfsBackend	backend,
	const char *	sessionname
	)
{
	(void)backend;
	return WfrDeleteFile(
		true, WfsppFileDnameSessions,
		WfsppFileExtSessions, sessionname);
}

WfrStatus
WfspFileEnumerateSessions(
	WfsBackend	backend,
	bool		initfl,
	bool *		pdonefl,
	char **		psessionname,
	void **		pstate
	)
{
	const char *	name;
	WfrStatus	status;


	(void)backend;

	if (initfl) {
		return WfrEnumerateFilesInit(
			WfsppFileDnameSessions,
			(WfrEnumerateFilesState **)pstate);
	}

	if (WFR_STATUS_SUCCESS(status = WfrEnumerateFiles(
			WfsppFileExtSessions, pdonefl,
			&name, (WfrEnumerateFilesState **)pstate)))
	{
		if (!(*pdonefl)) {
			status = WfrUnescapeFileName((char *)name, (const char **)psessionname);
		}
	}

	return status;
}

WfrStatus
WfspFileLoadSession(
	WfsBackend	backend,
	const char *	sessionname,
	WfsSession **	psession
	)
{
	bool			addedfl = false;
	time_t			mtime;
	WfsSession *		session = NULL;
	char *			session_data = NULL;
	size_t			session_data_size;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WfrLoadRawFile(
			true, WfsppFileDnameSessions, WfsppFileExtSessions,
			sessionname, &session_data, &session_data_size, &mtime)))
	{
		status = WfsGetSession(backend, true, sessionname, &session);
		if (WFR_STATUS_SUCCESS(status)) {
			if (session->mtime == mtime) {
				goto out;
			} else {
				session->mtime = mtime;
				status = WfsClearSession(backend, session, sessionname);
				addedfl = false;
			}
		} else if (WFR_STATUS_CONDITION(status) == ENOENT) {
			status = WfsAddSession(backend, sessionname, &session);
			addedfl = WFR_STATUS_SUCCESS(status);
		}

		status = WfrLoadParse(
			session_data, session_data_size, session, NULL,
			WFR_LAMBDA(WfrStatus, (void *param1, void *param2, const char *key, int type, const void *value, size_t value_size) {
				(void)param2;
				return WfsSetSessionKey((WfsSession *)param1, key, (void *)value, value_size, type);
			}));
	}

out:
	if (WFR_STATUS_SUCCESS(status) && psession) {
		*psession = session;
	} else if (WFR_STATUS_FAILURE(status) && addedfl && session) {
		(void)WfsDeleteSession(backend, false, sessionname);
	}

	WFR_FREE_IF_NOTNULL(session_data);

	return status;
}

WfrStatus
WfspFileRenameSession(
	WfsBackend	backend,
	const char *	sessionname,
	const char *	sessionname_new
	)
{
	(void)backend;
	return WfrRenameFile(
		true, WfsppFileDnameSessions, WfsppFileExtSessions,
		sessionname, sessionname_new);
}

WfrStatus
WfspFileSaveSession(
	WfsBackend	backend,
	WfsSession *	session
	)
{
	(void)backend;
	return WfrSaveTreeToFile(
		true, WfsppFileDnameSessions, WfsppFileExtSessions,
		session->name, session->tree);
}


void
WfspFileAddJumpList(
	const char *const	sessionname
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsppFileTransformList(
			true, false, WfsppFileFnameJumpList,
			WfsppFileFnameJumpListTmp, sessionname)))
	{
		update_jumplist();
	} else {
		/* Make sure we don't leave the jumplist dangling. */
		WfsClearJumpList(WfsGetBackend());
	}
}

WfrStatus
WfspFileCleanupJumpList(
	void
	)
{
	return WfrDeleteFile(false, NULL, NULL, WfsppFileFnameJumpList);
}

void
WfspFileClearJumpList(
	void
	)
{
	clear_jumplist_PuTTY();
}

WfrStatus
WfspFileGetEntriesJumpList(
	char **		pjump_list,
	size_t *	pjump_list_size
	)
{
	size_t		jump_list_size;
	WfrStatus	status;


	status = WfrLoadListFromFile(
		WfsppFileFnameJumpList,
		pjump_list, &jump_list_size);
	if (WFR_STATUS_FAILURE(status)) {
		if (WFR_STATUS_CONDITION(status) == ENOENT) {
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
			*pjump_list_size = jump_list_size;
		}
	}

	return status;
}

void
WfspFileRemoveJumpList(
	const char *const	sessionname
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsppFileTransformList(
			false, true, WfsppFileFnameJumpList,
			WfsppFileFnameJumpListTmp, sessionname)))
	{
		update_jumplist();
	} else {
		/* Make sure we don't leave the jumplist dangling. */
		WfsClearJumpList(WfsGetBackend());
	}
}

WfrStatus
WfspFileSetEntriesJumpList(
	const char *	jump_list,
	size_t		jump_list_size
	)
{
	return WfrSaveListToFile(
		WfsppFileFnameJumpList,
		WfsppFileFnameJumpListTmp,
		jump_list, jump_list_size);
}


WfrStatus
WfspFileAddPrivKeyList(
	const char *const	privkey_name
	)
{
	return WfsppFileTransformList(
		true, false, WfsppFileFnamePrivKeyList,
		WfsppFileFnamePrivKeyListTmp, privkey_name);
}

WfrStatus
WfspFileCleanupPrivKeyList(
	void
	)
{
	return WfrDeleteFile(false, NULL, NULL, WfsppFileFnamePrivKeyList);
}

WfrStatus
WfspFileClearPrivKeyList(
	void
	)
{
	return WfspFileCleanupPrivKeyList();
}

WfrStatus
WfspFileGetEntriesPrivKeyList(
	char **		pprivkey_list,
	size_t *	pprivkey_list_size
	)
{
	size_t		privkey_list_size;
	WfrStatus	status;


	status = WfrLoadListFromFile(
		WfsppFileFnamePrivKeyList,
		pprivkey_list, &privkey_list_size);
	if (WFR_STATUS_FAILURE(status)) {
		if (WFR_STATUS_CONDITION(status) == ENOENT) {
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
			*pprivkey_list_size = privkey_list_size;
		}
	}

	return status;
}

WfrStatus
WfspFileRemovePrivKeyList(
	const char *const	privkey_name
	)
{
	return WfsppFileTransformList(
		false, true, WfsppFileFnamePrivKeyList,
		WfsppFileFnamePrivKeyListTmp, privkey_name);
}

WfrStatus
WfspFileSetEntriesPrivKeyList(
	const char *	privkey_list,
	size_t		privkey_list_size
	)
{
	return WfrSaveListToFile(
		WfsppFileFnamePrivKeyList,
		WfsppFileFnamePrivKeyListTmp,
		privkey_list, privkey_list_size);
}


WfrStatus
WfspFileCleanupContainer(
	WfsBackend	backend
	)
{
	WfrStatus	status;


	(void)backend;
	status = WfrDeleteDirectory(WfsppFileDname, true, true);
	return status;
}

WfrStatus
WfspFileEnumerateCancel(
	WfsBackend	backend,
	void **		pstate
	)
{
	(void)backend;
	WfrEnumerateFilesCancel((WfrEnumerateFilesState **)pstate);
	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspFileInit(
	void
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsppFileInitAppDataSubdir())) {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfspFileSetBackend(
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
