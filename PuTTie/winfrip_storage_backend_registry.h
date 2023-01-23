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
	WfspRegistryClearHostKeys, WfspRegistryDeleteHostKey, WfspRegistryEnumerateHostKeys,	\
	WfspRegistryLoadHostKey, WfspRegistryRenameHostKey, WfspRegistrySaveHostKey,		\
												\
	WfspRegistryClearSessions, WfspRegistryCloseSession, WfspRegistryDeleteSession,		\
	WfspRegistryEnumerateSessions, WfspRegistryLoadSession, WfspRegistryRenameSession,	\
	WfspRegistrySaveSession,								\
												\
	WfspRegistryJumpListAdd, WfspRegistryJumpListCleanup, WfspRegistryJumpListClear,	\
	WfspRegistryJumpListGetEntries, WfspRegistryJumpListRemove,				\
	WfspRegistryJumpListSetEntries,								\
												\
	WfspRegistryInit, WfspRegistrySetBackend,						\
	}

WfrStatus	WfspRegistryClearHostKeys(WfsBackend backend);
WfrStatus	WfspRegistryDeleteHostKey(WfsBackend backend, const char *key_name);
WfrStatus	WfspRegistryEnumerateHostKeys(WfsBackend backend, bool initfl, bool *pdonefl, const char **pkey_name, void *state);
WfrStatus	WfspRegistryLoadHostKey(WfsBackend backend, const char *key_name, const char **pkey);
WfrStatus	WfspRegistryRenameHostKey(WfsBackend backend, const char *key_name, const char *key_name_new);
WfrStatus	WfspRegistrySaveHostKey(WfsBackend backend, const char *key_name, const char *key);

WfrStatus	WfspRegistryClearSessions(WfsBackend backend);
WfrStatus	WfspRegistryCloseSession(WfsBackend backend, WfspSession *session);
WfrStatus	WfspRegistryDeleteSession(WfsBackend backend, const char *sessionname);
WfrStatus	WfspRegistryEnumerateSessions(WfsBackend backend, bool initfl, bool *pdonefl, char **psessionname, void *state);
WfrStatus	WfspRegistryLoadSession(WfsBackend backend, const char *sessionname, WfspSession **psession);
WfrStatus	WfspRegistryRenameSession(WfsBackend backend, const char *sessionname, const char *sessionname_new);
WfrStatus	WfspRegistrySaveSession(WfsBackend backend, WfspSession *session);

void		WfspRegistryJumpListAdd(const char *const sessionname);
WfrStatus	WfspRegistryJumpListCleanup(void);
void		WfspRegistryJumpListClear(void);
WfrStatus	WfspRegistryJumpListGetEntries(char **pjump_list, size_t *pjump_list_size);
void		WfspRegistryJumpListRemove(const char *const sessionname);
WfrStatus	WfspRegistryJumpListSetEntries(const char *jump_list, size_t jump_list_size);

WfrStatus	WfspRegistryInit(void);
WfrStatus	WfspRegistrySetBackend(WfsBackend backend_new);

#endif // !PUTTY_WINFRIP_STORAGE_BACKEND_REGISTRY_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
