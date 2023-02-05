/*
 * winfrip_storage_priv.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_PRIV_H
#define PUTTY_WINFRIP_STORAGE_PRIV_H

#include "winfrip_rtl_pcre2.h"

/*
 * Public type definitions private to PuTTie/winfrip_storage*.c
 */

/*
 * Storage backend implementation type definitions
 */

typedef struct WfspBackend {
	const char *	name;
	const char *	arg;

	WfrStatus	(*CleanupHostCAs)(WfsBackend);
	WfrStatus	(*ClearHostCAs)(WfsBackend);
	WfrStatus	(*CloseHostCA)(WfsBackend, WfsHostCA *);
	WfrStatus	(*DeleteHostCA)(WfsBackend, const char *);
	WfrStatus	(*EnumerateHostCAs)(WfsBackend, bool, bool *, char **, void **);
	WfrStatus	(*LoadHostCA)(WfsBackend, const char *, WfsHostCA **);
	WfrStatus	(*RenameHostCA)(WfsBackend, const char *, const char *);
	WfrStatus	(*SaveHostCA)(WfsBackend, WfsHostCA *);

	WfrStatus	(*CleanupHostKeys)(WfsBackend);
	WfrStatus	(*ClearHostKeys)(WfsBackend);
	WfrStatus	(*DeleteHostKey)(WfsBackend, const char *);
	WfrStatus	(*EnumerateHostKeys)(WfsBackend, bool, bool *, char **, void **);
	WfrStatus	(*LoadHostKey)(WfsBackend, const char *, const char **);
	WfrStatus	(*RenameHostKey)(WfsBackend, const char *, const char *);
	WfrStatus	(*SaveHostKey)(WfsBackend, const char *, const char *);

	WfrStatus	(*ClearOptions)(WfsBackend);
	WfrStatus	(*LoadOptions)(WfsBackend);
	WfrStatus	(*SaveOptions)(WfsBackend, WfrTree *);

	WfrStatus	(*CleanupSessions)(WfsBackend);
	WfrStatus	(*ClearSessions)(WfsBackend);
	WfrStatus	(*CloseSession)(WfsBackend, WfsSession *);
	WfrStatus	(*DeleteSession)(WfsBackend, const char *);
	WfrStatus	(*EnumerateSessions)(WfsBackend, bool, bool *, char **, void **);
	WfrStatus	(*LoadSession)(WfsBackend, const char *, WfsSession **);
	WfrStatus	(*RenameSession)(WfsBackend, const char *, const char *);
	WfrStatus	(*SaveSession)(WfsBackend, WfsSession *);

	void		(*AddJumpList)(const char *const);
	WfrStatus	(*CleanupJumpList)(void);
	void		(*ClearJumpList)(void);
	WfrStatus	(*GetEntriesJumpList)(char **, size_t *);
	void		(*RemoveJumpList)(const char *const);
	WfrStatus	(*SetEntriesJumpList)(const char *, size_t);

	WfrStatus	(*AddPrivKeyList)(const char *const);
	WfrStatus	(*CleanupPrivKeyList)(void);
	WfrStatus	(*ClearPrivKeyList)(void);
	WfrStatus	(*GetEntriesPrivKeyList)(char **, size_t *);
	WfrStatus	(*RemovePrivKeyList)(const char *const);
	WfrStatus	(*SetEntriesPrivKeyList)(const char *, size_t);

	WfrStatus	(*CleanupContainer)(WfsBackend);
	WfrStatus	(*EnumerateCancel)(WfsBackend, void **);
	WfrStatus	(*Init)(void);
	WfrStatus	(*SetBackend)(WfsBackend);

	tree234 *	tree_host_ca;
	tree234 *	tree_host_key;
	tree234 *	tree_options;
	tree234 *	tree_session;
} WfspBackend;

#define WFSP_BACKEND_INIT(backend) ({		\
	(backend).tree_host_ca = NULL;		\
	(backend).tree_host_key = NULL;		\
	(backend).tree_options = NULL;		\
	(backend).tree_session = NULL;		\
	WFR_STATUS_CONDITION_SUCCESS;		\
})

/*
 * Public subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfrStatus	WfsGetBackendImpl(WfsBackend backend, void *pbackend);
WfrStatus	WfsTransformList(bool addfl, bool delfl, char **plist, size_t *plist_size, const char *const trans_item);
WfrStatus	WfrTreeCloneValue(WfrTreeItem *item, void **pvalue_new);
void		WfrTreeFreeItem(WfrTreeItem *item);

#endif // !PUTTY_WINFRIP_STORAGE_PRIV_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
