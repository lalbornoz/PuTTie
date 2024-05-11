/*
 * winfrip_rtl_windows.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_WINDOWS_H
#define PUTTY_WINFRIP_RTL_WINDOWS_H

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

#define WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, hWnd, fmt, ...)			\
	WFR_IF_STATUS_FAILURE_MESSAGEBOX1(hWnd, "PuTTie", (status), fmt, ## __VA_ARGS__)

#define WFR_IF_STATUS_FAILURE_MESSAGEBOX1(hWnd, caption, status, fmt, ...)		\
	if (WFR_FAILURE((status))) {							\
		(void)WfrMessageBoxF(							\
				(hWnd), (caption),					\
				MB_ICONERROR | MB_OK | MB_DEFBUTTON1,			\
				fmt ": %s",	## __VA_ARGS__,				\
				WfrStatusToErrorMessage((status)));			\
	}

WfrStatus	WfrGetCommandLineAsArgVUtf8(int *pargc, char ***pargv);
WfrStatus	WfrGetCommandLineAsUtf8(char **pCommandLine);
WfrStatus	WfrGetModuleBaseNameW(wchar_t **pmodule_nameW);
WfrStatus	WfrGetModuleDirNameW(wchar_t **pmodule_nameW);
unsigned int	WfrGetOsVersionMajor(void);
unsigned int	WfrGetOsVersionMinor(void);
bool		WfrIsVKeyDown(int nVirtKey);
int		WfrMessageBox(HWND hWnd, const char *lpText, const char *lpCaption, unsigned int uType);
int		WfrMessageBoxF(HWND hWnd, const char *lpCaption, unsigned int uType, const char *format, ...);
WfrStatus	WfrRequestFile(DWORD Flags, HWND hwndOwner, const char *lpstrDefExt, const char *lpstrFilter, const char *lpstrInitialDir, const char *lpstrTitle, size_t nMaxFile, bool preservefl, bool savefl, char **plpstrFile, WORD *pnFileOffset);

#endif // !PUTTY_WINFRIP_RTL_WINDOWS_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
