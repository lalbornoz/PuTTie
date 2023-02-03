/*
 * winfrip_storage_backend_file.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_BACKEND_FILE_H
#define PUTTY_WINFRIP_STORAGE_BACKEND_FILE_H

/*
 * Public subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

#define WFSP_FILE_BACKEND {								\
	"File",										\
	"file",										\
											\
	WfspFileCleanupHostCAs, WfspFileClearHostCAs, WfspFileCloseHostCA,		\
	WfspFileDeleteHostCA, WfspFileEnumerateHostCAs, WfspFileLoadHostCA,		\
	WfspFileRenameHostCA, WfspFileSaveHostCA,					\
											\
	WfspFileCleanupHostKeys, WfspFileClearHostKeys, WfspFileDeleteHostKey,		\
	WfspFileEnumerateHostKeys, WfspFileLoadHostKey, WfspFileRenameHostKey,		\
	WfspFileSaveHostKey,								\
											\
	WfspFileCleanupSessions, WfspFileClearSessions, WfspFileCloseSession,		\
	WfspFileDeleteSession, WfspFileEnumerateSessions, WfspFileLoadSession,		\
	WfspFileRenameSession, WfspFileSaveSession,					\
											\
	WfspFileAddJumpList, WfspFileCleanupJumpList, WfspFileClearJumpList,		\
	WfspFileGetEntriesJumpList, WfspFileRemoveJumpList,				\
	WfspFileSetEntriesJumpList,							\
											\
	WfspFileAddPrivKeyList, WfspFileCleanupPrivKeyList, WfspFileClearPrivKeyList,	\
	WfspFileGetEntriesPrivKeyList, WfspFileRemovePrivKeyList,			\
	WfspFileSetEntriesPrivKeyList,							\
											\
	WfspFileCleanupContainer, WfspFileInit, WfspFileSetBackend,			\
}

WfrStatus	WfspFileCleanupHostCAs(WfsBackend backend);
WfrStatus	WfspFileClearHostCAs(WfsBackend backend);
WfrStatus	WfspFileCloseHostCA(WfsBackend backend, WfsHostCA *hca);
WfrStatus	WfspFileDeleteHostCA(WfsBackend backend, const char *name);
WfrStatus	WfspFileEnumerateHostCAs(WfsBackend backend, bool initfl, bool *pdonefl, char **pname, void *state);
WfrStatus	WfspFileLoadHostCA(WfsBackend backend, const char *name, WfsHostCA **phca);
WfrStatus	WfspFileRenameHostCA(WfsBackend backend, const char *name, const char *name_new);
WfrStatus	WfspFileSaveHostCA(WfsBackend backend, WfsHostCA *hca);

WfrStatus	WfspFileCleanupHostKeys(WfsBackend backend);
WfrStatus	WfspFileClearHostKeys(WfsBackend backend);
WfrStatus	WfspFileDeleteHostKey(WfsBackend backend, const char *key_name);
WfrStatus	WfspFileEnumerateHostKeys(WfsBackend backend, bool initfl, bool *pdonefl, char **pkey_name, void *state);
WfrStatus	WfspFileLoadHostKey(WfsBackend backend, const char *key_name, const char **pkey);
WfrStatus	WfspFileRenameHostKey(WfsBackend backend, const char *key_name, const char *key_name_new);
WfrStatus	WfspFileSaveHostKey(WfsBackend backend, const char *key_name, const char *key);

WfrStatus	WfspFileCleanupSessions(WfsBackend backend);
WfrStatus	WfspFileClearSessions(WfsBackend backend);
WfrStatus	WfspFileCloseSession(WfsBackend backend, WfsSession *session);
WfrStatus	WfspFileDeleteSession(WfsBackend backend, const char *sessionname);
WfrStatus	WfspFileEnumerateSessions(WfsBackend backend, bool initfl, bool *pdonefl, char **psessionname, void *state);
WfrStatus	WfspFileLoadSession(WfsBackend backend, const char *sessionname, WfsSession **psession);
WfrStatus	WfspFileRenameSession(WfsBackend backend, const char *sessionname, const char *sessionname_new);
WfrStatus	WfspFileSaveSession(WfsBackend backend, WfsSession *session);

void		WfspFileAddJumpList(const char *const sessionname);
WfrStatus	WfspFileCleanupJumpList(void);
void		WfspFileClearJumpList(void);
WfrStatus	WfspFileGetEntriesJumpList(char **pjump_list, size_t *pjump_list_size);
void		WfspFileRemoveJumpList(const char *const sessionname);
WfrStatus	WfspFileSetEntriesJumpList(const char *jump_list, size_t jump_list_size);

WfrStatus	WfspFileAddPrivKeyList(const char *const privkey_name);
WfrStatus	WfspFileCleanupPrivKeyList(void);
WfrStatus	WfspFileClearPrivKeyList(void);
WfrStatus	WfspFileGetEntriesPrivKeyList(char **pprivkey_list, size_t *pprivkey_list_size);
WfrStatus	WfspFileRemovePrivKeyList(const char *const privkey_name);
WfrStatus	WfspFileSetEntriesPrivKeyList(const char *privkey_list, size_t privkey_list_size);

WfrStatus	WfspFileCleanupContainer(WfsBackend backend);
WfrStatus	WfspFileInit(void);
WfrStatus	WfspFileSetBackend(WfsBackend backend_new);

#endif // !PUTTY_WINFRIP_STORAGE_BACKEND_FILE_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
