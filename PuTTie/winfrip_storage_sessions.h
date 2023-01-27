/*
 * winfrip_storage_sessions.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_SESSIONS_H
#define PUTTY_WINFRIP_STORAGE_SESSIONS_H

/*
 * Public session storage subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfrStatus	WfsAddSession(WfsBackend backend, const char *sessionname, WfspSession **psession);
WfrStatus	WfsCleanupSessions(WfsBackend backend);
WfrStatus	WfsClearSession(WfsBackend backend, WfspSession *session, const char *sessionname);
WfrStatus	WfsClearSessions(WfsBackend backend, bool delete_in_backend);
WfrStatus	WfsCloseSession(WfsBackend backend, WfspSession *session);
WfrStatus	WfsCopySession(WfsBackend backend_from, WfsBackend backend_to, const char *sessionname, WfspSession *session, WfspSession **psession);
WfrStatus	WfsDeleteSession(WfsBackend backend, bool delete_in_backend, const char *sessionname);
WfrStatus	WfsEnumerateSessions(WfsBackend backend, bool cached, bool initfl, bool *pdonefl, char **psessionname, void *state);
WfrStatus	WfsExportSession(WfsBackend backend_from, WfsBackend backend_to, bool movefl, char *sessionname);
WfrStatus	WfsExportSessions(WfsBackend backend_from, WfsBackend backend_to, bool clear_to, bool continue_on_error, void (*error_fn)(const char *, WfrStatus));
WfrStatus	WfsGetSession(WfsBackend backend, bool cached, const char *sessionname, WfspSession **psession);
WfrStatus	WfsGetSessionKey(WfspSession *session, const char *key, WfspTreeItemType value_type, void **pvalue, size_t *pvalue_size);
WfrStatus	WfsRenameSession(WfsBackend backend, bool rename_in_backend, const char *sessionname, const char *sessionname_new);
WfrStatus	WfsSaveSession(WfsBackend backend, WfspSession *session);
WfrStatus	WfsSetSessionKey(WfspSession *session, const char *key, void *value, size_t value_size, WfspTreeItemType value_type);

#endif // !PUTTY_WINFRIP_STORAGE_SESSIONS_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
