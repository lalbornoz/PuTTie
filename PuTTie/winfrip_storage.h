/*
 * winfrip_storage.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_H
#define PUTTY_WINFRIP_STORAGE_H

#include "winfrip_storage_tree.h"

/*
 * Public type definitions private to PuTTie/winfrip*.c
 */

typedef enum WfsBackend {
	WFS_BACKEND_EPHEMERAL				= 0,
	WFS_BACKEND_FILE					= 1,
	WFS_BACKEND_REGISTRY				= 2,
	WFS_BACKEND_DEFAULT					= WFS_BACKEND_REGISTRY,

	WFS_BACKEND_MIN						= WFS_BACKEND_EPHEMERAL,
	WFS_BACKEND_MAX						= WFS_BACKEND_REGISTRY,
} WfsBackend;

/*
 * Public macros private to PuTTie/winfrip_storage*.c
 */

#define WFSP_SESSION_NAME_DEFAULT(name)							\
		if(!(name) || !((name)[0])) {							\
			(name) = "Default Settings";						\
		}

#define WFSP_TREE234_FOREACH(status, tree, idx, item)			\
		for (int (idx) = 0; WFR_STATUS_SUCCESS(status)			\
		     && ((item) = index234((tree), (idx))); (idx)++)

/*
 * Public storage backend subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfsBackend WfsGetBackend(void);
WfrStatus WfsGetBackendArg(WfsBackend backend, const char **parg);
WfrStatus WfsGetBackendArgString(char **parg_string);
WfrStatus WfsGetBackendName(WfsBackend backend, const char **pname);
bool WfsGetBackendNext(WfsBackend *pbackend);
WfrStatus WfsSetBackend(WfsBackend new_backend, WfsBackend new_backend_from, bool reset);
WfrStatus WfsSetBackendFromArgV(int *pargc, char ***pargv);
WfrStatus WfsSetBackendFromCmdLine(char *cmdline);

/*
 * Public host key storage subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfrStatus WfsClearHostKeys(WfsBackend backend, bool delete_in_backend);
WfrStatus WfsDeleteHostKey(WfsBackend backend, bool delete_in_backend, const char *key_name);
WfrStatus WfsEnumerateHostKeys(WfsBackend backend, bool cached, bool initfl, bool *pdonefl, const char **pkey_name, void *state);
WfrStatus WfsExportHostKey(WfsBackend backend_from, WfsBackend backend_to, bool movefl, const char *key_name);
WfrStatus WfsExportHostKeys(WfsBackend backend_from, WfsBackend backend_to, bool clear_to);
WfrStatus WfsGetHostKey(WfsBackend backend, bool cached, const char *key_name, const char **pkey);
WfrStatus WfsPrintHostKeyName(const char *hostname, int port, const char *keytype, char **pkey_name);
WfrStatus WfsRenameHostKey(WfsBackend backend, bool rename_in_backend, const char *key_name, const char *key_name_new);
WfrStatus WfsSaveHostKey(WfsBackend backend, const char *key_name, const char *key);
WfrStatus WfsSetHostKey(WfsBackend backend, const char *key_name, const char *key);

/*
 * Public session storage subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfrStatus WfsAddSession(WfsBackend backend, const char *sessionname, WfspSession **psession);
WfrStatus WfsClearSession(WfsBackend backend, WfspSession *session, const char *sessionname);
WfrStatus WfsClearSessions(WfsBackend backend, bool delete_in_backend);
WfrStatus WfsCloseSession(WfsBackend backend, WfspSession *session);
WfrStatus WfsCopySession(WfsBackend backend_from, WfsBackend backend_to, const char *sessionname, WfspSession *session, WfspSession **psession);
WfrStatus WfsDeleteSession(WfsBackend backend, bool delete_in_backend, const char *sessionname);
WfrStatus WfsEnumerateSessions(WfsBackend backend, bool cached, bool initfl, bool *pdonefl, char **psessionname, void *state);
WfrStatus WfsExportSession(WfsBackend backend_from, WfsBackend backend_to, bool movefl, char *sessionname);
WfrStatus WfsExportSessions(WfsBackend backend_from, WfsBackend backend_to, bool clear_to);
WfrStatus WfsGetSession(WfsBackend backend, bool cached, const char *sessionname, WfspSession **psession);
WfrStatus WfsGetSessionKey(WfspSession *session, const char *key, WfspTreeItemType value_type, void **pvalue, size_t *pvalue_size);
WfrStatus WfsRenameSession(WfsBackend backend, bool rename_in_backend, const char *sessionname, const char *sessionname_new);
WfrStatus WfsSaveSession(WfsBackend backend, WfspSession *session);
WfrStatus WfsSetSessionKey(WfspSession *session, const char *key, void *value, size_t value_size, WfspTreeItemType value_type);

/*
 * Public jump list storage subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

void WfsJumpListAdd(WfsBackend backend, const char *const sessionname);
void WfsJumpListClear(WfsBackend backend);
void WfsJumpListRemove(WfsBackend backend, const char *const sessionname);

/*
 * Public subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfrStatus WfsInit(void);

#endif // !PUTTY_WINFRIP_STORAGE_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
