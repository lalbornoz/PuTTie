/*
 * winfrip_storage.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_H
#define PUTTY_WINFRIP_STORAGE_H

#include "winfrip_storage_tree.h"

/*
 * Public type definitions private to PuTTie/winfrip*.c
 */

typedef enum WfsBackend {
	WFS_BACKEND_EPHEMERAL	= 0,
	WFS_BACKEND_FILE	= 1,
	WFS_BACKEND_REGISTRY	= 2,
	WFS_BACKEND_DEFAULT	= WFS_BACKEND_REGISTRY,

	WFS_BACKEND_MIN		= WFS_BACKEND_EPHEMERAL,
	WFS_BACKEND_MAX		= WFS_BACKEND_REGISTRY,
} WfsBackend;

typedef enum WfsTreeItemType {
	WFS_TREE_ITYPE_HOST_CA		= 1,
	WFS_TREE_ITYPE_HOST_KEY		= 2,
	WFS_TREE_ITYPE_INT		= 3,
	WFS_TREE_ITYPE_SESSION		= 4,
	WFS_TREE_ITYPE_STRING		= 5,
	WFS_TREE_ITYPE_MAX		= WFS_TREE_ITYPE_STRING,
} WfsTreeItemType;

/*
 * Public macros private to PuTTie/winfrip_storage*.c
 */

#define WFS_SESSION_NAME_DEFAULT(name)			\
	if(!(name) || !((name)[0])) {			\
		(name) = "Default Settings";		\
	}

#define WFS_TREE234_FOREACH(status, tree, idx, item)	\
	for (int (idx) = 0; WFR_STATUS_SUCCESS(status)	\
	  && ((item) = index234((tree), (idx))); (idx)++)

/*
 * Public storage backend subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfsBackend	WfsGetBackend(void);
WfrStatus	WfsGetBackendArg(WfsBackend backend, const char **parg);
WfrStatus	WfsGetBackendArgString(char **parg_string);
WfrStatus	WfsGetBackendName(WfsBackend backend, const char **pname);
bool		WfsGetBackendNext(WfsBackend *pbackend);
WfrStatus	WfsSetBackend(WfsBackend new_backend, WfsBackend new_backend_from, bool reset);
WfrStatus	WfsSetBackendFromArgV(int *pargc, char ***pargv);
WfrStatus	WfsSetBackendFromCmdLine(char *cmdline);

/*
 * Public subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfrStatus	WfsCleanupContainer(WfsBackend backend);
WfrStatus	WfsEnumerateCancel(WfsBackend backend, void **pstate);
WfrStatus	WfsInit(void);

#endif // !PUTTY_WINFRIP_STORAGE_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
