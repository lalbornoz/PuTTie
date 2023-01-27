/*
 * winfrip_storage_backend_file.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_BACKEND_FILE_H
#define PUTTY_WINFRIP_STORAGE_BACKEND_FILE_H

#include <dirent.h>

/*
 * Public type definitions private to PuTTie/winfrip_storage*.c
 */

typedef struct WfspFileEnumerateState {
	bool			donefl;
	struct dirent *		dire;
	DIR *			dirp;
} WfspFileEnumerateState;
#define WFSP_FILE_ENUMERATE_STATE_EMPTY {						\
	.donefl = false,								\
	.dire = NULL,									\
	.dirp = NULL,									\
}
#define WFSP_FILE_ENUMERATE_STATE_INIT(state)						\
	(state) = (WfspFileEnumerateState)WFSP_FILE_ENUMERATE_STATE_EMPTY

/*
 * Public subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

#define WFSP_FILE_BACKEND {								\
	"File",										\
	"file",										\
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
	WfspFileCleanupContainer, WfspFileInit, WfspFileSetBackend,			\
}

WfrStatus	WfspFileCleanupHostKeys(WfsBackend backend);
WfrStatus	WfspFileClearHostKeys(WfsBackend backend);
WfrStatus	WfspFileDeleteHostKey(WfsBackend backend, const char *key_name);
WfrStatus	WfspFileEnumerateHostKeys(WfsBackend backend, bool initfl, bool *pdonefl, const char **pkey_name, void *state);
WfrStatus	WfspFileLoadHostKey(WfsBackend backend, const char *key_name, const char **pkey);
WfrStatus	WfspFileRenameHostKey(WfsBackend backend, const char *key_name, const char *key_name_new);
WfrStatus	WfspFileSaveHostKey(WfsBackend backend, const char *key_name, const char *key);

WfrStatus	WfspFileCleanupSessions(WfsBackend backend);
WfrStatus	WfspFileClearSessions(WfsBackend backend);
WfrStatus	WfspFileCloseSession(WfsBackend backend, WfspSession *session);
WfrStatus	WfspFileDeleteSession(WfsBackend backend, const char *sessionname);
WfrStatus	WfspFileEnumerateSessions(WfsBackend backend, bool initfl, bool *pdonefl, char **psessionname, void *state);
WfrStatus	WfspFileLoadSession(WfsBackend backend, const char *sessionname, WfspSession **psession);
WfrStatus	WfspFileRenameSession(WfsBackend backend, const char *sessionname, const char *sessionname_new);
WfrStatus	WfspFileSaveSession(WfsBackend backend, WfspSession *session);

void		WfspFileAddJumpList(const char *const sessionname);
WfrStatus	WfspFileCleanupJumpList(void);
void		WfspFileClearJumpList(void);
WfrStatus	WfspFileGetEntriesJumpList(char **pjump_list, size_t *pjump_list_size);
void		WfspFileRemoveJumpList(const char *const sessionname);
WfrStatus	WfspFileSetEntriesJumpList(const char *jump_list, size_t jump_list_size);

WfrStatus	WfspFileCleanupContainer(WfsBackend backend);
WfrStatus	WfspFileInit(void);
WfrStatus	WfspFileSetBackend(WfsBackend backend_new);

#endif // !PUTTY_WINFRIP_STORAGE_BACKEND_FILE_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
