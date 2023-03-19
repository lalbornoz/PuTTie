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
	WfspEphemeralClearOptions, WfspEphemeralLoadOptions, WfspEphemeralSaveOptions,			\
													\
	WfspEphemeralCleanupSessions, WfspEphemeralClearSessions, WfspEphemeralCloseSession,		\
	WfspEphemeralDeleteSession, WfspEphemeralEnumerateSessions, WfspEphemeralLoadSession,		\
	WfspEphemeralRenameSession, WfspEphemeralSaveSession,						\
													\
	WfspEphemeralAddJumpList, WfspEphemeralCleanupJumpList, WfspEphemeralClearJumpList,		\
	WfspEphemeralGetEntriesJumpList, WfspEphemeralRemoveJumpList,					\
	WfspEphemeralSetEntriesJumpList,								\
													\
	WfspEphemeralAddPrivKeyList, WfspEphemeralCleanupPrivKeyList, WfspEphemeralClearPrivKeyList,	\
	WfspEphemeralGetEntriesPrivKeyList, WfspEphemeralRemovePrivKeyList,				\
	WfspEphemeralSetEntriesPrivKeyList,								\
													\
	WfspEphemeralCleanupContainer, WfspEphemeralEnumerateCancel,					\
	WfspEphemeralInit, WfspEphemeralSetBackend,							\
													\
	NULL, NULL, NULL, NULL,										\
}

WfrStatus	WfspEphemeralCleanupHostCAs(WfsBackend backend);
WfrStatus	WfspEphemeralClearHostCAs(WfsBackend backend);
WfrStatus	WfspEphemeralCloseHostCA(WfsBackend backend, WfsHostCA *hca);
WfrStatus	WfspEphemeralDeleteHostCA(WfsBackend backend, const char *name);
WfrStatus	WfspEphemeralEnumerateHostCAs(WfsBackend backend, bool initfl, bool *pdonefl, char **pname, void **pstate);
WfrStatus	WfspEphemeralLoadHostCA(WfsBackend backend, const char *name, WfsHostCA **phca);
WfrStatus	WfspEphemeralRenameHostCA(WfsBackend backend, const char *name, const char *name_new);
WfrStatus	WfspEphemeralSaveHostCA(WfsBackend backend, WfsHostCA *hca);

WfrStatus	WfspEphemeralCleanupHostKeys(WfsBackend backend);
WfrStatus	WfspEphemeralClearHostKeys(WfsBackend backend);
WfrStatus	WfspEphemeralDeleteHostKey(WfsBackend backend, const char *key_name);
WfrStatus	WfspEphemeralEnumerateHostKeys(WfsBackend backend, bool initfl, bool *pdonefl, char **pkey_name, void **pstate);
WfrStatus	WfspEphemeralLoadHostKey(WfsBackend backend, const char *key_name, const char **pkey);
WfrStatus	WfspEphemeralRenameHostKey(WfsBackend backend, const char *key_name, const char *key_name_new);
WfrStatus	WfspEphemeralSaveHostKey(WfsBackend backend, const char *key_name, const char *key);

WfrStatus	WfspEphemeralClearOptions(WfsBackend backend);
WfrStatus	WfspEphemeralLoadOptions(WfsBackend backend);
WfrStatus	WfspEphemeralSaveOptions(WfsBackend backend, WfrTree *backend_tree);

WfrStatus	WfspEphemeralCleanupSessions(WfsBackend backend);
WfrStatus	WfspEphemeralClearSessions(WfsBackend backend);
WfrStatus	WfspEphemeralCloseSession(WfsBackend backend, WfsSession *session);
WfrStatus	WfspEphemeralDeleteSession(WfsBackend backend, const char *sessionname);
WfrStatus	WfspEphemeralEnumerateSessions(WfsBackend backend, bool initfl, bool *pdonefl, char **psessionname, void **pstate);
WfrStatus	WfspEphemeralLoadSession(WfsBackend backend, const char *sessionname, WfsSession **psession);
WfrStatus	WfspEphemeralRenameSession(WfsBackend backend, const char *sessionname, const char *sessionname_new);
WfrStatus	WfspEphemeralSaveSession(WfsBackend backend, WfsSession *session);

void		WfspEphemeralAddJumpList(const char *const sessionname);
WfrStatus	WfspEphemeralCleanupJumpList(void);
void		WfspEphemeralClearJumpList(void);
WfrStatus	WfspEphemeralGetEntriesJumpList(char **pjump_list, size_t *pjump_list_size);
void		WfspEphemeralRemoveJumpList(const char *const sessionname);
WfrStatus	WfspEphemeralSetEntriesJumpList(const char *jump_list, size_t jump_list_size);

WfrStatus	WfspEphemeralAddPrivKeyList(const char *const privkey_name);
WfrStatus	WfspEphemeralCleanupPrivKeyList(void);
WfrStatus	WfspEphemeralClearPrivKeyList(void);
WfrStatus	WfspEphemeralGetEntriesPrivKeyList(char **pprivkey_list, size_t *pprivkey_list_size);
WfrStatus	WfspEphemeralRemovePrivKeyList(const char *const privkey_name);
WfrStatus	WfspEphemeralSetEntriesPrivKeyList(const char *privkey_list, size_t privkey_list_size);

WfrStatus	WfspEphemeralCleanupContainer(WfsBackend backend);
WfrStatus	WfspEphemeralEnumerateCancel(WfsBackend backend, void **pstate);
WfrStatus	WfspEphemeralInit(void);
WfrStatus	WfspEphemeralSetBackend(WfsBackend backend_new);

#endif // !PUTTY_WINFRIP_STORAGE_BACKEND_EPHEMERAL_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
