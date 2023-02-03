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
WfrStatus	WfrDeleteFile(bool escape_fnamefl, const char *dname, const char *ext, const char *fname);
WfrStatus	WfrDeleteFiles(const char *dname, const char *ext);
WfrStatus	WfrEnumerateFiles(const char *ext, bool *pdonefl, const char **pfname, WfrEnumerateFilesState **pstate);
void		WfrEnumerateFilesCancel(WfrEnumerateFilesState **pstate);
WfrStatus	WfrEnumerateFilesInit(const char *dname, WfrEnumerateFilesState **pstate);
WfrStatus	WfrEnumerateFilesV(const char *dname, const char *ext, size_t *pfilec, char ***pfilev);
WfrStatus	WfrEscapeFileName(const char *dname, const char *ext, const char *name, bool tmpfl, char *fname, size_t fname_size);
WfrStatus	WfrLoadListFromFile(const char *fname, char **plist, size_t *plist_size);
WfrStatus	WfrLoadRawFile(bool escape_fnamefl, const char *dname, const char *ext, const char *fname, char **pdata, size_t *pdata_size);
WfrStatus	WfrMakeDirectory(char *path, bool existsfl);
WfrStatus	WfrPathNameToAbsoluteW(wchar_t **pname);
WfrStatus	WfrPathNameToDirectory(char *pname, char **pdname);
WfrStatus	WfrPathNameToDirectoryW(wchar_t *pname, wchar_t **pdname);
WfrStatus	WfrRenameFile(bool escape_fnamefl, const char *dname, const char *ext, const char *fname, const char *fname_new);
WfrStatus	WfrSaveListToFile(const char *fname, const char *fname_tmp, const char *list, size_t list_size);
WfrStatus	WfrSaveRawFile(bool escape_fnamefl, bool recreate_dnamefl, char *dname, const char *ext, const char *fname, const char *data, size_t data_size);
WfrStatus	WfrUnescapeFileName(char *fname, const char **pname);

#endif // !PUTTY_WINFRIP_RTL_FILE_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
