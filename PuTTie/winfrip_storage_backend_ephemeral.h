/*
 * winfrip_storage_backend_ephemeral.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_BACKEND_EPHEMERAL_H
#define PUTTY_WINFRIP_STORAGE_BACKEND_EPHEMERAL_H

/*
 * Public subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

#define WFSP_EPHEMERAL_BACKEND {									\
	"Ephemeral",											\
	"ephemeral",											\
													\
	WfspEphemeralCleanupHostCAs, WfspEphemeralClearHostCAs, WfspEphemeralCloseHostCA,		\
	WfspEphemeralDeleteHostCA, WfspEphemeralEnumerateHostCAs, WfspEphemeralLoadHostCA,		\
	WfspEphemeralRenameHostCA, WfspEphemeralSaveHostCA,						\
													\
	WfspEphemeralCleanupHostKeys, WfspEphemeralClearHostKeys, WfspEphemeralDeleteHostKey,		\
	WfspEphemeralEnumerateHostKeys, WfspEphemeralLoadHostKey, WfspEphemeralRenameHostKey,		\
	WfspEphemeralSaveHostKey,									\
													\
	WfspEphemeralCleanupSessions, WfspEphemeralClearSessions, WfspEphemeralCloseSession,		\
	WfspEphemeralDeleteSession, WfspEphemeralEnumerateSessions, WfspEphemeralLoadSession,		\
	WfspEphemeralRenameSession, WfspEphemeralSaveSession,						\
													\
	WfspEphemeralAddJumpList, WfspEphemeralCleanupJumpList, WfspEphemeralClearJumpList,		\
	WfspEphemeralGetEntriesJumpList, WfspEphemeralRemoveJumpList,					\
	WfspEphemeralSetEntriesJumpList,								\
													\
	WfspEphemeralCleanupContainer, WfspEphemeralInit, WfspEphemeralSetBackend,			\
}

WfrStatus	WfspEphemeralCleanupHostCAs(WfsBackend backend);
WfrStatus	WfspEphemeralClearHostCAs(WfsBackend backend);
WfrStatus	WfspEphemeralCloseHostCA(WfsBackend backend, WfspHostCA *hca);
WfrStatus	WfspEphemeralDeleteHostCA(WfsBackend backend, const char *name);
WfrStatus	WfspEphemeralEnumerateHostCAs(WfsBackend backend, bool initfl, bool *pdonefl, char **pname, void *state);
WfrStatus	WfspEphemeralLoadHostCA(WfsBackend backend, const char *name, WfspHostCA **phca);
WfrStatus	WfspEphemeralRenameHostCA(WfsBackend backend, const char *name, const char *name_new);
WfrStatus	WfspEphemeralSaveHostCA(WfsBackend backend, WfspHostCA *hca);

WfrStatus	WfspEphemeralCleanupHostKeys(WfsBackend backend);
WfrStatus	WfspEphemeralClearHostKeys(WfsBackend backend);
WfrStatus	WfspEphemeralDeleteHostKey(WfsBackend backend, const char *key_name);
WfrStatus	WfspEphemeralEnumerateHostKeys(WfsBackend backend, bool initfl, bool *pdonefl, const char **pkey_name, void *state);
WfrStatus	WfspEphemeralLoadHostKey(WfsBackend backend, const char *key_name, const char **pkey);
WfrStatus	WfspEphemeralRenameHostKey(WfsBackend backend, const char *key_name, const char *key_name_new);
WfrStatus	WfspEphemeralSaveHostKey(WfsBackend backend, const char *key_name, const char *key);

WfrStatus	WfspEphemeralCleanupSessions(WfsBackend backend);
WfrStatus	WfspEphemeralClearSessions(WfsBackend backend);
WfrStatus	WfspEphemeralCloseSession(WfsBackend backend, WfspSession *session);
WfrStatus	WfspEphemeralDeleteSession(WfsBackend backend, const char *sessionname);
WfrStatus	WfspEphemeralEnumerateSessions(WfsBackend backend, bool initfl, bool *pdonefl, char **psessionname, void *state);
WfrStatus	WfspEphemeralLoadSession(WfsBackend backend, const char *sessionname, WfspSession **psession);
WfrStatus	WfspEphemeralRenameSession(WfsBackend backend, const char *sessionname, const char *sessionname_new);
WfrStatus	WfspEphemeralSaveSession(WfsBackend backend, WfspSession *session);

void		WfspEphemeralAddJumpList(const char *const sessionname);
WfrStatus	WfspEphemeralCleanupJumpList(void);
void		WfspEphemeralClearJumpList(void);
WfrStatus	WfspEphemeralGetEntriesJumpList(char **pjump_list, size_t *pjump_list_size);
void		WfspEphemeralRemoveJumpList(const char *const sessionname);
WfrStatus	WfspEphemeralSetEntriesJumpList(const char *jump_list, size_t jump_list_size);

WfrStatus	WfspEphemeralCleanupContainer(WfsBackend backend);
WfrStatus	WfspEphemeralInit(void);
WfrStatus	WfspEphemeralSetBackend(WfsBackend backend_new);

#endif // !PUTTY_WINFRIP_STORAGE_BACKEND_EPHEMERAL_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
