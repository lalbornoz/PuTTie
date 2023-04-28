/*
 * winfrip_rtl_windows.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdio.h>

#include <errno.h>
#include <sys/stat.h>

#include <windows.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_windows.h"

/*
 * Private variables
 */

static bool		WfrpVersionInit = false;
static unsigned int	WfrpVersionMajor = UINT_MAX;
static unsigned int	WfrpVersionMinor = UINT_MAX;

/*
 * Private subroutine prototypes
 */

static void WfrpInitVersion(void);

/*
 * Private subroutines
 */

static void
WfrpInitVersion(
	void
	)
{
	OSVERSIONINFO	VersionInformation;


	ZeroMemory(&VersionInformation, sizeof(VersionInformation));
	VersionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionExA(&VersionInformation)) {
		WfrpVersionMajor = VersionInformation.dwMajorVersion;
		WfrpVersionMinor = VersionInformation.dwMinorVersion;
	}
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

WfrStatus
WfrGetCommandLineAsArgVUtf8(
	int *		pargc,
	char ***	pargv
	)
{
	int		argc;
	char **		argv = NULL;
	int		argcW;
	LPWSTR *	argvW = NULL;
	LPWSTR		CommandLineW;
	WfrStatus	status;


	if (WFR_FAILURE_WINDOWS(status, (CommandLineW = GetCommandLineW()))
	||  WFR_FAILURE_WINDOWS(status, (argvW = CommandLineToArgvW(CommandLineW, &argcW)))
	||  !WFR_NEWN(status, argv, argcW, char *))
	{
		if (argvW) {
			(void)LocalFree(argvW);
		}
		return status;
	} else {
		argc = argcW;
		for (int narg = 0; narg < argc; narg++) {
			argv[narg] = NULL;
		}
	}

	for (int narg = 0; (narg < argcW) && argvW[narg]; narg++) {
		if (WFR_FAILURE(status = WfrConvertUtf16ToUtf8String(
				argvW[narg], wcslen(argvW[narg]),
				&argv[narg]))) {
			break;
		}
	}

	if (WFR_SUCCESS(status)) {
		*pargc = argc;
		*pargv = argv;
	} else if (argv) {
		for (int narg = 0; narg < argc; narg++) {
			WFR_FREE_IF_NOTNULL(argv[narg]);
		}
	}

	return status;
}

WfrStatus
WfrGetCommandLineAsUtf8(
	char **pCommandLine
	)
{
	char *		CommandLine;
	char *		CommandLine_;
	LPWSTR		CommandLineW;
	char *		sep;
	WfrStatus	status;


	if (WFR_FAILURE_WINDOWS(status, (CommandLineW = GetCommandLineW()))
	||  WFR_FAILURE(status = WfrConvertUtf16ToUtf8String(
			CommandLineW, wcslen(CommandLineW),
			&CommandLine)))
	{
		return status;
	} else if (CommandLine[0] == '"') {
		CommandLine_ = CommandLine;
		sep = strchr(++CommandLine_, '"');
		if (!sep) {
			WFR_FREE(CommandLine);
			(void)WFR_STATUS_BIND_POSIX(status, (CommandLine = strdup("")));
		} else {
			CommandLine_ = ++sep;
			if (CommandLine_[0] == ' ') {
				CommandLine_++;
				(void)WFR_STATUS_BIND_POSIX(status, (CommandLine_ = strdup(CommandLine_)));
				WFR_FREE(CommandLine);
				CommandLine = CommandLine_;
			} else {
				WFR_FREE(CommandLine);
				(void)WFR_STATUS_BIND_POSIX(status, (CommandLine = strdup("")));
			}
		}
	} else {
		sep = strchr(CommandLine, ' ');
		if (!sep) {
			WFR_FREE(CommandLine);
			(void)WFR_STATUS_BIND_POSIX(status, (CommandLine = strdup("")));
		} else {
			sep++;
			(void)WFR_STATUS_BIND_POSIX(status, (CommandLine_ = strdup(sep)));
			WFR_FREE(CommandLine);
			CommandLine = CommandLine_;
		}
	}

	if (WFR_SUCCESS(status)) {
		*pCommandLine = CommandLine;
	}

	return status;
}

unsigned int
WfrGetOsVersionMajor(
	void
	)
{
	if (!WfrpVersionInit) {
		WfrpInitVersion();
	}
	return WfrpVersionMajor;
}

unsigned int
WfrGetOsVersionMinor(
	void
	)
{
	if (!WfrpVersionInit) {
		WfrpInitVersion();
	}
	return WfrpVersionMinor;
}

bool
WfrIsVKeyDown(
	int	nVirtKey
	)
{
	return (GetKeyState(nVirtKey) < 0);
}

int
WfrMessageBox(
	HWND		hWnd,
	const char *	lpText,
	const char *	lpCaption,
	unsigned int	uType
	)
{
	wchar_t *	lpCaptionW = NULL;
	wchar_t *	lpTextW = NULL;
	int		rc;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(
			lpText, strlen(lpText), &lpTextW))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(
			lpCaption, strlen(lpCaption), &lpCaptionW)))
	{
		rc = MessageBoxW(hWnd, lpTextW, lpCaptionW, uType);
	} else {
		rc = MessageBoxA(hWnd, lpText, lpCaption, uType);
	}

	WFR_FREE_IF_NOTNULL(lpCaptionW);
	WFR_FREE_IF_NOTNULL(lpTextW);

	return rc;
}

int
WfrMessageBoxF(
	HWND		hWnd,
	const char *	lpCaption,
	unsigned int	uType,
	const char *	format,
			...
	)
{
	va_list		ap;
	static char	msg_buf[1024];


	va_start(ap, format);
	(void)vsnprintf(msg_buf, sizeof(msg_buf), format, ap);
	va_end(ap);
	msg_buf[sizeof(msg_buf) - 1] = '\0';

	return WfrMessageBox(hWnd, msg_buf, lpCaption, uType);
}

WfrStatus
WfrRequestFile(
	DWORD		Flags,
	HWND		hwndOwner,
	const char *	lpstrDefExt,
	const char *	lpstrFilter,
	const char *	lpstrInitialDir,
	const char *	lpstrTitle,
	size_t		nMaxFile,
	bool		preservefl,
	bool		savefl,
	char **		plpstrFile,
	WORD *		pnFileOffset
	)
{
	wchar_t *	lpstrDefExtW = NULL;
	char *		lpstrFile = NULL;
	wchar_t *	lpstrFileW = NULL;
	size_t		lpstrFileInitial_len;
	wchar_t *	lpstrFileInitialW = NULL;
	size_t		lpstrFileW_len;
	size_t		lpstrFilter_len;
	wchar_t *	lpstrFilterW = NULL;
	wchar_t *	lpstrInitialDirW = NULL;
	wchar_t *	lpstrTitleW = NULL;
	wchar_t		lpWorkingDirectoryW[MAX_PATH + 1];
	OPENFILENAMEW	ofW;
	struct _stat64	statbuf;
	WfrStatus	status;


	lpstrFileInitial_len = (*plpstrFile ? strlen(*plpstrFile) : 0);

	lpstrFilter_len = 1;
	for (const char *p = lpstrFilter;
	     (p[0] == '\0') ? (p[1] != '\0') : true;
	     p++, lpstrFilter_len++);


        if ((preservefl
	    ? WFR_FAILURE_WINDOWS(status, (GetCurrentDirectoryW(
			WFR_SIZEOF_WSTRING(lpWorkingDirectoryW),
			lpWorkingDirectoryW) > 0))
	    : false)
	||  (lpstrDefExt
	    ? WFR_FAILURE(status = WfrConvertUtf8ToUtf16String(
			lpstrDefExt, strlen(lpstrDefExt),
			&lpstrDefExtW))
	    : false)
	||  (*plpstrFile
	    ? WFR_FAILURE(status = WfrConvertUtf8ToUtf16String(
			  *plpstrFile, lpstrFileInitial_len,
			  &lpstrFileInitialW))
	    : false)
	||  !WFR_NEWN(status, lpstrFileW, nMaxFile, wchar_t)
	||  WFR_FAILURE(status = WfrConvertUtf8ToUtf16String(
			lpstrFilter, lpstrFilter_len,
			&lpstrFilterW))
	||  (lpstrInitialDir
	    ? WFR_FAILURE(status = WfrConvertUtf8ToUtf16String(
			lpstrInitialDir, strlen(lpstrInitialDir),
			&lpstrInitialDirW))
	    : false)
	||  WFR_FAILURE(status = WfrConvertUtf8ToUtf16String(
			lpstrTitle, strlen(lpstrTitle),
			&lpstrTitleW)))
	{
		goto out;
	} else {
		if ((*plpstrFile)) {
			memcpy(lpstrFileW, lpstrFileInitialW,
			       lpstrFileInitial_len * sizeof(*lpstrFileInitialW));
			lpstrFileW[lpstrFileInitial_len] = L'\0';
		} else {
			lpstrFileW[0] = L'\0';
		}
	}

	memset(&ofW, 0, sizeof(ofW));
	ofW.Flags = Flags;
	ofW.hwndOwner = hwndOwner;
	ofW.lpstrCustomFilter = NULL;
	ofW.lpstrDefExt = lpstrDefExtW;
	ofW.lpstrFile = lpstrFileW;
	ofW.lpstrFilter = lpstrFilterW;
	ofW.lpstrInitialDir = lpstrInitialDirW;
#ifdef OPENFILENAME_SIZE_VERSION_400
	ofW.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#else
	ofW.lStructSize = sizeof(ofW);
#endif
	ofW.nFilterIndex = 1;
	ofW.nMaxFile = nMaxFile;
	ofW.lpstrFileTitle = NULL;
	ofW.lpstrTitle = lpstrTitleW;

        (void)WFR_STATUS_BIND_WINDOWS(status,
		((savefl ? GetSaveFileNameW(&ofW) : GetOpenFileNameW(&ofW))));

	if (WFR_SUCCESS(status)) {
		if (preservefl) {
			(void)SetCurrentDirectoryW(lpWorkingDirectoryW);
		}

		lpstrFileW_len = 1;
		for (const wchar_t *p = lpstrFileW;
		     (p[0] == L'\0') ? (p[1] != L'\0') : true;
		     p++, lpstrFileW_len++);

		status = WfrConvertUtf16ToUtf8String(lpstrFileW, lpstrFileW_len, &lpstrFile);
		if (WFR_SUCCESS(status)) {
			if (!savefl) {
				(void)WFR_STATUS_BIND_POSIX(status, (_wstat64(lpstrFileW, &statbuf) == 0));
			}
		}

		if (WFR_SUCCESS(status)) {
			*plpstrFile = lpstrFile;
			if (pnFileOffset) {
				*pnFileOffset = ofW.nFileOffset;
			}
		} else {
			WFR_FREE(lpstrFile);
		}
	}

out:
	WFR_FREE_IF_NOTNULL(lpstrDefExtW);
	WFR_FREE_IF_NOTNULL(lpstrFileW);
	WFR_FREE_IF_NOTNULL(lpstrFileInitialW);
	WFR_FREE_IF_NOTNULL(lpstrFilterW);
	WFR_FREE_IF_NOTNULL(lpstrInitialDirW);
	WFR_FREE_IF_NOTNULL(lpstrTitleW);

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
