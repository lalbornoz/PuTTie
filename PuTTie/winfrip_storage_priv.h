/*
 * winfrip_storage_priv.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_PRIV_H
#define PUTTY_WINFRIP_STORAGE_PRIV_H

/*
 * Public type definitions private to PuTTie/winfrip_storage*.c
 */

/*
 * Storage backend implementation type definitions
 */

typedef struct WfspBackend {
	const char *	name;
	const char *	arg;

	WfrStatus	(*CleanupHostKeys)(WfsBackend);
	WfrStatus	(*ClearHostKeys)(WfsBackend);
	WfrStatus	(*DeleteHostKey)(WfsBackend, const char *);
	WfrStatus	(*EnumerateHostKeys)(WfsBackend, bool, bool *, const char **, void *);
	WfrStatus	(*LoadHostKey)(WfsBackend, const char *, const char **);
	WfrStatus	(*RenameHostKey)(WfsBackend, const char *, const char *);
	WfrStatus	(*SaveHostKey)(WfsBackend, const char *, const char *);

	WfrStatus	(*CleanupSessions)(WfsBackend);
	WfrStatus	(*ClearSessions)(WfsBackend);
	WfrStatus	(*CloseSession)(WfsBackend, WfspSession *);
	WfrStatus	(*DeleteSession)(WfsBackend, const char *);
	WfrStatus	(*EnumerateSessions)(WfsBackend, bool, bool *, char **, void *);
	WfrStatus	(*LoadSession)(WfsBackend, const char *, WfspSession **);
	WfrStatus	(*RenameSession)(WfsBackend, const char *, const char *);
	WfrStatus	(*SaveSession)(WfsBackend, WfspSession *);

	void		(*AddJumpList)(const char *const);
	WfrStatus	(*CleanupJumpList)(void);
	void		(*ClearJumpList)(void);
	WfrStatus	(*GetEntriesJumpList)(char **, size_t *);
	void		(*RemoveJumpList)(const char *const);
	WfrStatus	(*SetEntriesJumpList)(const char *, size_t);

	WfrStatus	(*CleanupContainer)(WfsBackend);
	WfrStatus	(*Init)(void);
	WfrStatus	(*SetBackend)(WfsBackend);

	tree234 *	tree_host_key, *tree_session;
} WfspBackend;

#define WFSP_BACKEND_INIT(backend)		\
	(backend).tree_host_key = NULL;		\
	(backend).tree_session = NULL;

/*
 * Public storage backend subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfrStatus	WfsGetBackendImpl(WfsBackend backend, void *pbackend);

#endif // !PUTTY_WINFRIP_STORAGE_PRIV_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
