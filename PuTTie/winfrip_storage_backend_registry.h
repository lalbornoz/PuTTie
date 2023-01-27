/*
 * winfrip_storage_backend_registry.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_BACKEND_REGISTRY_H
#define PUTTY_WINFRIP_STORAGE_BACKEND_REGISTRY_H

/*
 * Public type definitions private to PuTTie/winfrip_storage*.c
 */

typedef struct WfspRegistryEnumerateState {
	bool	donefl;
	DWORD	dwIndex;
	HKEY	hKey;
} WfspRegistryEnumerateState;
#define WFSP_REGISTRY_ENUMERATE_STATE_EMPTY {							\
	.donefl = false,									\
	.dwIndex = 0,										\
	.hKey = 0,										\
}
#define WFSP_REGISTRY_ENUMERATE_STATE_INIT(state)						\
	(state) = (WfspRegistryEnumerateState)WFSP_REGISTRY_ENUMERATE_STATE_EMPTY

/*
 * Public subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

#define WFSP_REGISTRY_BACKEND {									\
	"Registry",										\
	"registry",										\
												\
	WfspRegistryCleanupHostCAs, WfspRegistryClearHostCAs, WfspRegistryCloseHostCA,		\
	WfspRegistryDeleteHostCA, WfspRegistryEnumerateHostCAs, WfspRegistryLoadHostCA,		\
	WfspRegistryRenameHostCA, WfspRegistrySaveHostCA,					\
												\
	WfspRegistryCleanupHostKeys, WfspRegistryClearHostKeys, WfspRegistryDeleteHostKey,	\
	WfspRegistryEnumerateHostKeys, WfspRegistryLoadHostKey, WfspRegistryRenameHostKey,	\
	WfspRegistrySaveHostKey,								\
												\
	WfspRegistryCleanupSessions, WfspRegistryClearSessions, WfspRegistryCloseSession,	\
	WfspRegistryDeleteSession, WfspRegistryEnumerateSessions, WfspRegistryLoadSession,	\
	WfspRegistryRenameSession, WfspRegistrySaveSession,					\
												\
	WfspRegistryAddJumpList, WfspRegistryCleanupJumpList, WfspRegistryClearJumpList,	\
	WfspRegistryGetEntriesJumpList, WfspRegistryRemoveJumpList,				\
	WfspRegistrySetEntriesJumpList,								\
												\
	WfspRegistryCleanupContainer, WfspRegistryInit, WfspRegistrySetBackend,			\
}

WfrStatus	WfspRegistryCleanupHostCAs(WfsBackend backend);
WfrStatus	WfspRegistryClearHostCAs(WfsBackend backend);
WfrStatus	WfspRegistryCloseHostCA(WfsBackend backend, WfsHostCA *hca);
WfrStatus	WfspRegistryDeleteHostCA(WfsBackend backend, const char *name);
WfrStatus	WfspRegistryEnumerateHostCAs(WfsBackend backend, bool initfl, bool *pdonefl, char **pname, void *state);
WfrStatus	WfspRegistryLoadHostCA(WfsBackend backend, const char *name, WfsHostCA **phca);
WfrStatus	WfspRegistryRenameHostCA(WfsBackend backend, const char *name, const char *name_new);
WfrStatus	WfspRegistrySaveHostCA(WfsBackend backend, WfsHostCA *hca);

WfrStatus	WfspRegistryCleanupHostKeys(WfsBackend backend);
WfrStatus	WfspRegistryClearHostKeys(WfsBackend backend);
WfrStatus	WfspRegistryDeleteHostKey(WfsBackend backend, const char *key_name);
WfrStatus	WfspRegistryEnumerateHostKeys(WfsBackend backend, bool initfl, bool *pdonefl, const char **pkey_name, void *state);
WfrStatus	WfspRegistryLoadHostKey(WfsBackend backend, const char *key_name, const char **pkey);
WfrStatus	WfspRegistryRenameHostKey(WfsBackend backend, const char *key_name, const char *key_name_new);
WfrStatus	WfspRegistrySaveHostKey(WfsBackend backend, const char *key_name, const char *key);

WfrStatus	WfspRegistryCleanupSessions(WfsBackend backend);
WfrStatus	WfspRegistryClearSessions(WfsBackend backend);
WfrStatus	WfspRegistryCloseSession(WfsBackend backend, WfsSession *session);
WfrStatus	WfspRegistryDeleteSession(WfsBackend backend, const char *sessionname);
WfrStatus	WfspRegistryEnumerateSessions(WfsBackend backend, bool initfl, bool *pdonefl, char **psessionname, void *state);
WfrStatus	WfspRegistryLoadSession(WfsBackend backend, const char *sessionname, WfsSession **psession);
WfrStatus	WfspRegistryRenameSession(WfsBackend backend, const char *sessionname, const char *sessionname_new);
WfrStatus	WfspRegistrySaveSession(WfsBackend backend, WfsSession *session);

void		WfspRegistryAddJumpList(const char *const sessionname);
WfrStatus	WfspRegistryCleanupJumpList(void);
void		WfspRegistryClearJumpList(void);
WfrStatus	WfspRegistryGetEntriesJumpList(char **pjump_list, size_t *pjump_list_size);
void		WfspRegistryRemoveJumpList(const char *const sessionname);
WfrStatus	WfspRegistrySetEntriesJumpList(const char *jump_list, size_t jump_list_size);

WfrStatus	WfspRegistryCleanupContainer(WfsBackend backend);
WfrStatus	WfspRegistryInit(void);
WfrStatus	WfspRegistrySetBackend(WfsBackend backend_new);

#endif // !PUTTY_WINFRIP_STORAGE_BACKEND_REGISTRY_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
