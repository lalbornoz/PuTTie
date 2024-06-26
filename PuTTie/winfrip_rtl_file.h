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
	struct _wdirent *	dire;
	_WDIR *			dirp;
	const wchar_t *		path;
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
 * Public macros and subroutine prototypes private to PuTTie/winfrip*.c
 */

#define WFR_SNPRINTF_PNAME(pname, pname_size, dname, ext, fname)	\
	WFR_SNPRINTF(							\
		(pname), (pname_size),					\
		"%s%s%s%s",						\
		((dname) ? (dname) : ""),				\
		((dname) ? "\\" : ""),					\
		(fname),						\
		((ext) ? (ext) : ""))

#define WFR_SNPRINTF_PNAME_TMP(pname, pname_size, dname, ext, fname)	\
	WFR_SNPRINTF(							\
		(pname), (pname_size),					\
		"%s%s%s%s.XXXXXX",					\
		((dname) ? (dname) : ""),				\
		((dname) ? "\\" : ""),					\
		(fname),						\
		((ext) ? (ext) : ""))

#define WFR_SNPRINTF_PNAMEW(pname, pname_size, dname, fname)		\
	WFR_SNWPRINTF(							\
		(pname), (pname_size),					\
		L"%S%S%S",						\
		((dname) ? (dname) : L""),				\
		((dname) ? L"\\" : L""),				\
		(fname))

WfrStatus	WfrDeleteDirectory(const char *path, bool continue_on_errorfl, bool noentfl, bool recursefl);
WfrStatus	WfrDeleteFile(bool escape_fnamefl, bool noentfl, const char *dname, const char *ext, const char *fname);
WfrStatus	WfrDeleteFileW(const wchar_t *dname, const wchar_t *fname);
WfrStatus	WfrDeleteFiles(const char *dname, const char *ext);
WfrStatus	WfrEnumerateFiles(const char *ext, bool *pdonefl, const char **pfname, WfrEnumerateFilesState **pstate);
void		WfrEnumerateFilesCancel(WfrEnumerateFilesState **pstate);
WfrStatus	WfrEnumerateFilesInit(const char *dname, WfrEnumerateFilesState **pstate);
WfrStatus	WfrEnumerateFilesV(const char *dname, const char *filter_list, size_t *pfilec, char ***pfilev);
WfrStatus	WfrEscapeFileName(const char *dname, const char *ext, const char *name, bool tmpfl, char *pname, size_t pname_size);
WfrStatus	WfrMakeDirectory(char *path, bool existsfl);
WfrStatus	WfrMakeDirectoryW(wchar_t *pathW, bool existsfl);
WfrStatus	WfrMoveFileW(wchar_t *pname_oldW, wchar_t *pname_newW);
WfrStatus	WfrPathNameToAbsoluteW(const wchar_t *pname, wchar_t **ppname_abs);
WfrStatus	WfrPathNameToDirectory(char *pname, char **pdname);
WfrStatus	WfrPathNameToDirectoryW(wchar_t *pname, wchar_t **pdname);
WfrStatus	WfrRenameFile(bool escape_fnamefl, const char *dname, const char *ext, const char *fname, const char *fname_new);
WfrStatus	WfrUnescapeFileName(char *fname, const char **pname);
WfrStatus	WfrWatchDirectory(bool display_errorsfl, char **pdname, CRITICAL_SECTION *dname_cs, HWND hwnd, UINT window_msg, HANDLE *phEvent, HANDLE *phThread);

#endif // !PUTTY_WINFRIP_RTL_FILE_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
