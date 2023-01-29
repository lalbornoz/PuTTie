/*
 * winfrip_rtl_file.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_FILE_H
#define PUTTY_WINFRIP_RTL_FILE_H

#include <stdbool.h>
#include <dirent.h>

#include "winfrip_rtl_status.h"

/*
 * Public type definitions private to PuTTie/winfrip_storage*.c
 */

typedef struct WfrEnumerateFilesState {
	bool			donefl;
	struct dirent *		dire;
	DIR *			dirp;
	const char *		path;
} WfrEnumerateFilesState;
#define WFR_ENUMERATE_FILES_STATE_EMPTY {					\
	.donefl = false,							\
	.dire = NULL,								\
	.dirp = NULL,								\
	.path = NULL,								\
}
#define WFR_ENUMERATE_FILES_STATE_INIT(state) ({				\
	(state) = (WfrEnumerateFilesState)WFR_ENUMERATE_FILES_STATE_EMPTY;	\
	WFR_STATUS_CONDITION_SUCCESS;						\
})

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

WfrStatus	WfrDeleteDirectory(const char *path, bool noentfl, bool recursefl);
WfrStatus	WfrDeleteFiles(const char *dname, const char *ext);
WfrStatus	WfrEnumerateFiles(const char *ext, bool *pdonefl, const char **pfname, WfrEnumerateFilesState *state);
WfrStatus	WfrEnumerateFilesInit(const char *dname, WfrEnumerateFilesState **state);
WfrStatus	WfrEscapeFileName(const char *dname, const char *ext, const char *name, bool tmpfl, char *fname, size_t fname_size);
WfrStatus	WfrMakeDirectory(char *path, bool existsfl);
WfrStatus	WfrPathNameToAbsoluteW(wchar_t **pname);
WfrStatus	WfrPathNameToDirectoryW(wchar_t *pname, wchar_t **pdname);
WfrStatus	WfrUnescapeFileName(char *fname, const char **pname);

#endif // !PUTTY_WINFRIP_RTL_FILE_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
