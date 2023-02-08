/*
 * winfrip_storage_sessions.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_SESSIONS_H
#define PUTTY_WINFRIP_STORAGE_SESSIONS_H

/*
 * Public type definitions private to PuTTie/winfrip_storage*.c
 */

/*
 * Storage session type definitions
 * (encompasses session key tree, time of last modification, and session name)
 */

typedef struct WfsSession {
	tree234 *	tree;
	__time64_t	mtime;
	const char *	name;
} WfsSession;
#define WFS_SESSION_EMPTY {				\
	.tree = NULL,					\
	.mtime = 0,					\
	.name = NULL,					\
}
#define WFS_SESSION_INIT(session) ({			\
	(session) = (WfsSession)WFS_SESSION_EMPTY;	\
	WFR_STATUS_CONDITION_SUCCESS;			\
})

/*
 * Public session storage subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfrStatus	WfsAddSession(WfsBackend backend, const char *sessionname, WfsSession **psession);
WfrStatus	WfsCleanupSessions(WfsBackend backend);
WfrStatus	WfsClearSession(WfsBackend backend, WfsSession *session, const char *sessionname);
WfrStatus	WfsClearSessions(WfsBackend backend, bool delete_in_backend);
WfrStatus	WfsCloseSession(WfsBackend backend, WfsSession *session);
WfrStatus	WfsCopySession(WfsBackend backend_from, WfsBackend backend_to, const char *sessionname, WfsSession *session, WfsSession **psession);
WfrStatus	WfsDeleteSession(WfsBackend backend, bool delete_in_backend, const char *sessionname);
WfrStatus	WfsEnumerateSessions(WfsBackend backend, bool cached, bool initfl, bool *pdonefl, char **psessionname, void **pstate);
WfrStatus	WfsExportSession(WfsBackend backend_from, WfsBackend backend_to, bool movefl, char *sessionname);
WfrStatus	WfsExportSessions(WfsBackend backend_from, WfsBackend backend_to, bool clear_to, bool continue_on_error, WfsErrorFn error_fn);
WfrStatus	WfsGetSession(WfsBackend backend, bool cached, const char *sessionname, WfsSession **psession);
WfrStatus	WfsGetSessionKey(WfsSession *session, const char *key, WfrTreeItemType value_type, void **pvalue, size_t *pvalue_size);
WfrStatus	WfsRenameSession(WfsBackend backend, bool rename_in_backend, const char *sessionname, const char *sessionname_new);
WfrStatus	WfsSaveSession(WfsBackend backend, WfsSession *session);
WfrStatus	WfsSetSessionKey(WfsSession *session, const char *key, void *value, size_t value_size, WfrTreeItemType value_type);

#endif // !PUTTY_WINFRIP_STORAGE_SESSIONS_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
