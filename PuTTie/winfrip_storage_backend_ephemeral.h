/*
 * winfrip_storage_backend_ephemeral.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_BACKEND_EPHEMERAL_H
#define PUTTY_WINFRIP_STORAGE_BACKEND_EPHEMERAL_H

/*
 * Public subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

#define WFSP_EPHEMERAL_BACKEND {																\
		"Ephemeral",																			\
		"ephemeral",																			\
																								\
		WfspEphemeralClearHostKeys, WfspEphemeralDeleteHostKey, WfspEphemeralEnumerateHostKeys,	\
		WfspEphemeralLoadHostKey, WfspEphemeralRenameHostKey, WfspEphemeralSaveHostKey,			\
																								\
		WfspEphemeralClearSessions, WfspEphemeralCloseSession, WfspEphemeralDeleteSession,		\
		WfspEphemeralEnumerateSessions, WfspEphemeralLoadSession, WfspEphemeralRenameSession,	\
		WfspEphemeralSaveSession,																\
																								\
		WfspEphemeralJumpListAdd, WfspEphemeralJumpListClear, WfspEphemeralJumpListRemove,		\
																								\
		WfspEphemeralInit, WfspEphemeralSetBackend,												\
	}

WfrStatus WfspEphemeralClearHostKeys(WfsBackend backend);
WfrStatus WfspEphemeralDeleteHostKey(WfsBackend backend, const char *key_name);
WfrStatus WfspEphemeralEnumerateHostKeys(WfsBackend backend, bool initfl, bool *pdonefl, const char **pkey_name, void *state);
WfrStatus WfspEphemeralLoadHostKey(WfsBackend backend, const char *key_name, const char **pkey);
WfrStatus WfspEphemeralRenameHostKey(WfsBackend backend, const char *key_name, const char *key_name_new);
WfrStatus WfspEphemeralSaveHostKey(WfsBackend backend, const char *key_name, const char *key);

WfrStatus WfspEphemeralClearSessions(WfsBackend backend);
WfrStatus WfspEphemeralCloseSession(WfsBackend backend, WfspSession *session);
WfrStatus WfspEphemeralDeleteSession(WfsBackend backend, const char *sessionname);
WfrStatus WfspEphemeralEnumerateSessions(WfsBackend backend, bool initfl, bool *pdonefl, char **psessionname, void *state);
WfrStatus WfspEphemeralLoadSession(WfsBackend backend, const char *sessionname, WfspSession **psession);
WfrStatus WfspEphemeralRenameSession(WfsBackend backend, const char *sessionname, const char *sessionname_new);
WfrStatus WfspEphemeralSaveSession(WfsBackend backend, WfspSession *session);

void WfspEphemeralJumpListAdd(const char *const sessionname);
void WfspEphemeralJumpListClear(void);
void WfspEphemeralJumpListRemove(const char *const sessionname);

WfrStatus WfspEphemeralInit(void);
WfrStatus WfspEphemeralSetBackend(WfsBackend backend_new);

#endif // !PUTTY_WINFRIP_STORAGE_BACKEND_EPHEMERAL_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
