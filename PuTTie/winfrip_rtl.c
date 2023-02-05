/*
 * winfrip_rtl.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <wchar.h>

#include <windows.h>

#include "PuTTie/winfrip_rtl.h"
#ifndef WINFRIP_RTL_NO_PCRE2
#include "PuTTie/winfrip_rtl_pcre2.h"
#endif /* WINFRIP_RTL_NO_PCRE2 */

/*
 * Private variables
 */

static const char *	WfrpGdiPlusStatusErrorMessage[] = {
	"Ok",
	"GenericError",
	"InvalidParameter",
	"OutOfMemory",
	"ObjectBusy",
	"InsufficientBuffer",
	"NotImplemented",
	"Win32Error",
	"WrongState",
	"Aborted",
	"FileNotFound",
	"ValueOverflow",
	"AccessDenied",
	"UnknownImageFormat",
	"FontFamilyNotFound",
	"FontStyleNotFound",
	"NotTrueTypeFont",
	"UnsupportedGdiplusVersion",
	"GdiplusNotInitialized",
	"PropertyNotFound",
	"PropertyNotSupported",
	"ProfileNotFound",
};

static const wchar_t *	WfrpGdiPlusStatusErrorMessageW[] = {
	L"Ok",
	L"GenericError",
	L"InvalidParameter",
	L"OutOfMemory",
	L"ObjectBusy",
	L"InsufficientBuffer",
	L"NotImplemented",
	L"Win32Error",
	L"WrongState",
	L"Aborted",
	L"FileNotFound",
	L"ValueOverflow",
	L"AccessDenied",
	L"UnknownImageFormat",
	L"FontFamilyNotFound",
	L"FontStyleNotFound",
	L"NotTrueTypeFont",
	L"UnsupportedGdiplusVersion",
	L"GdiplusNotInitialized",
	L"PropertyNotFound",
	L"PropertyNotSupported",
	L"ProfileNotFound",
};

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
WfrMessageBoxF(
	const char *	lpCaption,
	unsigned int	uType,
	const char *	format,
			...
	)
{
	va_list		ap;
	static char	msg_buf[255];


	va_start(ap, format);
	(void)vsnprintf(msg_buf, sizeof(msg_buf), format, ap);
	va_end(ap);
	msg_buf[sizeof(msg_buf) - 1] = '\0';

	return MessageBoxA(NULL, msg_buf, lpCaption, uType);
}

int
WfrMessageBoxFW(
	const wchar_t *		lpCaption,
	unsigned int		uType,
	const wchar_t *		format,
				...
	)
{
	va_list		ap;
	static wchar_t	msg_buf[255];


	va_start(ap, format);
	(void)vsnwprintf(msg_buf, WFR_SIZEOF_WSTRING(msg_buf), format, ap);
	va_end(ap);
	msg_buf[WFR_SIZEOF_WSTRING(msg_buf) - 1] = L'\0';

	return MessageBoxW(NULL, msg_buf, lpCaption, uType);
}

WfrStatus
WfrSnDuprintf(
	char **		ps,
	size_t *	pn,
	const char *	format,
			...
	)
{
	va_list		ap;
	WfrStatus	status;
	char *		s = NULL;
	int		s_len;
	size_t		s_size;


	if (!(s = WFR_NEWN(1, char))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		s_size = 1;
		va_start(ap, format);
		s_len = vsnprintf(s, s_size, format, ap);
		va_end(ap);

		if (s_len < 0) {
			status = WFR_STATUS_FROM_ERRNO();
		} else if (s_len == 0) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else if (WFR_STATUS_SUCCESS(status = WFR_RESIZE(
				s, s_size, (size_t)s_len + 1, char)))
		{
			va_start(ap, format);
			s_len = vsnprintf(s, s_size, format, ap);
			va_end(ap);

			s[((s_len < 0) ? 0 : s_len)] = '\0';
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	if (WFR_STATUS_SUCCESS(status)) {
		*ps = s;
		if (pn) {
			*pn = s_size;
		}
	} else {
		WFR_FREE_IF_NOTNULL(s);
	}

	return status;
}

const char *
WfrStatusToErrorMessage(
	WfrStatus	status
	)
{
	WfrStatusCondition	condition;
	static char		condition_msg[128];
#ifndef WINFRIP_RTL_NO_PCRE2
	static wchar_t		condition_msg_w[128];
#endif /* WINFRIP_RTL_NO_PCRE2 */
	static char		error_msg[256];
	static HMODULE		hModule_ntdll = NULL;


	switch (WFR_STATUS_FACILITY(status)) {
	default:
		WFR_STRNCPY(condition_msg, "(unknown facility)", sizeof(condition_msg));
		condition_msg[sizeof(condition_msg) - 1] = '\0';
		break;

	case WFR_STATUS_FACILITY_GDI_PLUS:
		condition = WFR_STATUS_CONDITION(status);
		if (condition < WFR_ARRAYCOUNT(WfrpGdiPlusStatusErrorMessage)) {
			WFR_STRNCPY(condition_msg, WfrpGdiPlusStatusErrorMessage[condition], sizeof(condition_msg));
		} else {
			WFR_STRNCPY(condition_msg, "(unknown GDI+ status)", sizeof(condition_msg));
		}
		break;

	case WFR_STATUS_FACILITY_POSIX:
		WFR_STRNCPY(condition_msg, strerror(status.condition), sizeof(condition_msg));
		break;

	case WFR_STATUS_FACILITY_WINDOWS:
		condition_msg[0] = '\0';
		(void)FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, status.condition,
			LANG_USER_DEFAULT,
			condition_msg, sizeof(condition_msg),
			NULL);
		break;

	case WFR_STATUS_FACILITY_WINDOWS_NT:
		if (!hModule_ntdll) {
			if (!(hModule_ntdll = LoadLibrary("NTDLL.DLL"))) {
				return WfrStatusToErrorMessage(WFR_STATUS_FROM_WINDOWS());
			}
		}

		condition_msg[0] = '\0';
		(void)FormatMessageA(
			FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM,
			hModule_ntdll, status.condition,
			LANG_USER_DEFAULT,
			condition_msg, sizeof(condition_msg),
			NULL);
		break;

#ifndef WINFRIP_RTL_NO_PCRE2
	case WFR_STATUS_FACILITY_PCRE2:
		condition_msg[0] = '\0';
		condition_msg_w[0] = L'\0';
		(void)pcre2_get_error_message(
			status.condition,
			condition_msg_w, sizeof(condition_msg_w));
		(void)WideCharToMultiByte(
			CP_UTF8, 0, condition_msg_w,
			wcslen(condition_msg_w) + 1,
			condition_msg, sizeof(condition_msg),
			NULL, NULL);
		break;
#endif /* WINFRIP_RTL_NO_PCRE2 */
	}

	WFR_SNPRINTF(
		error_msg, sizeof(error_msg),
		"Error in %s:%d: %s",
		status.file, status.line, condition_msg);

	return error_msg;
}

const wchar_t *
WfrStatusToErrorMessageW(
	WfrStatus	status
	)
{
	WfrStatusCondition	condition;
	static wchar_t		condition_msg[128];
	static wchar_t		error_msg[256];
	static HMODULE		hModule_ntdllW = NULL;


	switch (WFR_STATUS_FACILITY(status)) {
	default:
		WFR_WCSNCPY(condition_msg, L"(unknown facility)", WFR_SIZEOF_WSTRING(condition_msg));
		break;

	case WFR_STATUS_FACILITY_GDI_PLUS:
		condition = WFR_STATUS_CONDITION(status);
		if (condition < WFR_ARRAYCOUNT(WfrpGdiPlusStatusErrorMessageW)) {
			WFR_WCSNCPY(condition_msg, WfrpGdiPlusStatusErrorMessageW[condition], WFR_SIZEOF_WSTRING(condition_msg));
		} else {
			WFR_WCSNCPY(condition_msg, L"(unknown GDI+ status)", WFR_SIZEOF_WSTRING(condition_msg));
		}
		break;

	case WFR_STATUS_FACILITY_POSIX:
		WFR_WCSNCPY(condition_msg, _wcserror(status.condition), WFR_SIZEOF_WSTRING(condition_msg));
		break;

	case WFR_STATUS_FACILITY_WINDOWS:
		condition_msg[0] = L'\0';
		(void)FormatMessageW(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, status.condition, LANG_USER_DEFAULT,
			condition_msg, WFR_SIZEOF_WSTRING(condition_msg), NULL);
		break;

	case WFR_STATUS_FACILITY_WINDOWS_NT:
		if (!hModule_ntdllW) {
			if (!(hModule_ntdllW = LoadLibrary("NTDLL.DLL"))) {
				return WfrStatusToErrorMessageW(WFR_STATUS_FROM_WINDOWS());
			}
		}

		condition_msg[0] = L'\0';
		(void)FormatMessageW(
			FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM,
			hModule_ntdllW, status.condition,
			LANG_USER_DEFAULT,
			condition_msg, WFR_SIZEOF_WSTRING(condition_msg),
			NULL);
		break;
	}

	snwprintf(
		error_msg, WFR_SIZEOF_WSTRING(error_msg),
		L"Error in %S:%d: %S",
		status.file, status.line, condition_msg);

	return error_msg;
}

WfrStatus
WfrToWcsDup(
	char *		in,
	size_t		in_size,
	wchar_t **	pout_w
	)
{
	wchar_t *	out_w;
	int		out_w_len, out_w_size;
	WfrStatus	status;


	out_w_len = MultiByteToWideChar(CP_ACP, 0, in, in_size, NULL, 0);
	if (out_w_len > 0) {
		out_w_size = out_w_len * sizeof(*out_w);
		out_w = WFR_NEWN(out_w_size, wchar_t);
		ZeroMemory(out_w, out_w_size);
		if (MultiByteToWideChar(CP_ACP, 0, in, in_size, out_w, out_w_size) == out_w_len) {
			*pout_w = out_w;
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else {
			status = WFR_STATUS_FROM_WINDOWS();
		}
	} else {
		status = WFR_STATUS_FROM_WINDOWS();
	}

	return status;
}

wchar_t *
WfrWcsNDup(
	const wchar_t *		in_w,
	size_t			in_w_len
	)
{
	wchar_t *	out_w;
	size_t		out_w_size;


	out_w_size = in_w_len + 1;
	if ((out_w = WFR_NEWN(out_w_size, wchar_t))) {
		memset(out_w, 0, out_w_size * sizeof(wchar_t));
		wcsncpy(out_w, in_w, in_w_len);
		out_w[in_w_len] = L'\0';
	}

	return out_w;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
