/*
 * winfrip_storage.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_H
#define PUTTY_WINFRIP_STORAGE_H

#include "winfrip_rtl_tree.h"

/*
 * Public type definitions private to PuTTie/winfrip*.c
 */

#ifndef WFS_DEFAULT_STORAGE_BACKEND
#define WFS_DEFAULT_STORAGE_BACKEND	WFS_BACKEND_REGISTRY
#endif /* !WFS_DEFAULT_STORAGE_BACKEND */

typedef enum WfsBackend {
	WFS_BACKEND_EPHEMERAL	= 0,
	WFS_BACKEND_FILE	= 1,
	WFS_BACKEND_REGISTRY	= 2,
	WFS_BACKEND_DEFAULT	= WFS_BACKEND_REGISTRY,

	WFS_BACKEND_MIN		= WFS_BACKEND_EPHEMERAL,
	WFS_BACKEND_MAX		= WFS_BACKEND_REGISTRY,
} WfsBackend;

typedef enum WfrTreeItemType {
	WFR_TREE_ITYPE_HOST_CA		= WFR_TREE_ITYPE_BASE_MAX + 1,
	WFR_TREE_ITYPE_HOST_KEY		= WFR_TREE_ITYPE_BASE_MAX + 2,
	WFR_TREE_ITYPE_SESSION		= WFR_TREE_ITYPE_BASE_MAX + 3,
	WFR_TREE_ITYPE_MAX		= WFR_TREE_ITYPE_SESSION,
} WfrTreeItemType;


typedef void (*WfsErrorFn)(const char *, WfrStatus);

/*
 * Public macros private to PuTTie/winfrip_storage*.c
 */

#define WFS_SESSION_NAME_DEFAULT(name) ({			\
	bool	defaultfl;					\
								\
	if(!(name) || !((name)[0])) {				\
		(name) = "Default Settings";			\
		defaultfl = true;				\
	} else if (strcmp((name), "Default Settings") == 0) {	\
		defaultfl = true;				\
	} else {						\
		defaultfl = false;				\
	}							\
								\
	defaultfl;						\
})

#define WFS_SESSION_NAME_IS_DEFAULT(name) 			\
	(strcmp((name), "Default Settings") == 0)

/*
 * Public storage backend subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfsBackend	WfsGetBackend(void);
WfrStatus	WfsGetBackendArg(WfsBackend backend, const char **parg);
WfrStatus	WfsGetBackendArgString(char **parg_string);
WfrStatus	WfsGetBackendName(WfsBackend backend, const char **pname);
bool		WfsGetBackendNext(WfsBackend *pbackend);
WfrStatus	WfsSetBackend(WfsBackend new_backend, WfsBackend new_backend_from, bool reset, char *args_extra);
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
