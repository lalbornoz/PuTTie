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

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

WfrStatus
WfrConvertUtf8ToUtf16String(
	const char *	in,
	size_t		in_len,
	wchar_t **	poutW
	)
{
	return WfrConvertUtf8ToUtf16String1(in, in_len, poutW, NULL);
}

WfrStatus
WfrConvertUtf8ToUtf16String1(
	const char *	in,
	size_t		in_len,
	wchar_t **	poutW,
	size_t *	poutW_size
	)
{
	wchar_t *	outW;
	int		outW_len, outW_nbytes;
	WfrStatus	status;


	outW_len = MultiByteToWideChar(CP_UTF8, 0, in, in_len, NULL, 0);
	outW_nbytes = (outW_len + 1) * sizeof(*outW);

	if (WFR_NEWN(status, outW, outW_nbytes, wchar_t)) {
		ZeroMemory(outW, outW_nbytes);
		if (WFR_SUCCESS_WINDOWS(status,
				(outW_len == MultiByteToWideChar(
					CP_UTF8, 0, in, in_len,
					outW, outW_nbytes))))
		{
			outW[outW_len] = L'\0';
			*poutW = outW;
			if (poutW_size) {
				*poutW_size = outW_len + 1;
			}
		} else {
			WFR_FREE(outW);
		}
	}

	return status;
}

WfrStatus
WfrConvertUtf16ToUtf8String(
	const wchar_t *		inW,
	size_t			inW_len,
	char **			pout
	)
{
	return WfrConvertUtf16ToUtf8String1(inW, inW_len, pout, NULL);
}

WfrStatus
WfrConvertUtf16ToUtf8String1(
	const wchar_t *		inW,
	size_t			inW_len,
	char **			pout,
	size_t *		pout_size
	)
{
	char *		out;
	int		out_len, out_nbytes;
	WfrStatus	status;


	out_len = WideCharToMultiByte(CP_UTF8, 0, inW, inW_len, NULL, 0, NULL, NULL);
	out_nbytes = (out_len + 1) * sizeof(*out);

	if (WFR_NEWN(status, out, out_nbytes, char)) {
		ZeroMemory(out, out_nbytes);
		if (WFR_SUCCESS_WINDOWS(status,
				(out_len == WideCharToMultiByte(
					CP_UTF8, 0, inW, inW_len,
					out, out_nbytes, NULL, NULL))))
		{
			out[out_len] = '\0';
			*pout = out;
			if (pout_size) {
				*pout_size = out_len + 1;
			}
		} else {
			WFR_FREE(out);
		}
	}

	return status;
}

int
WfrFPrintUtf8AsUtf16F(
	FILE *restrict		stream,
	const char *restrict	format,
				...
	)
{
	va_list		ap;
	int		nprinted;
	WfrStatus	status;
	char *		s = NULL;
	size_t		s_len;
	wchar_t *	sW = NULL;


	va_start(ap, format);
	status = WfrSnDuprintV(&s, &s_len, format, ap);
	va_end(ap);

	if (WFR_SUCCESS(status)
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(
			s, s_len, &sW)))
	{
		nprinted = fwprintf(stream, L"%S", sW);
	} else {
		nprinted = -1;
	};

	WFR_FREE_IF_NOTNULL(s);
	WFR_FREE_IF_NOTNULL(sW);

	if (nprinted == -1) {
		errno = EINVAL;
	}
	return nprinted;
}

bool
WfrIsNULTerminatedW(
	wchar_t *	ws,
	size_t		ws_size
	)
{
	for (size_t n = 0; n < ws_size; n++) {
		if (ws[n] == L'\0') {
			return true;
		}
	}

	return false;
}

WfrStatus
WfrSnDuprintF(
	char **		ps,
	size_t *	pn,
	const char *	format,
			...
	)
{
	va_list		ap;
	WfrStatus	status;


	va_start(ap, format);
	status = WfrSnDuprintV(ps, pn, format, ap);
	va_end(ap);

	return status;
}

WfrStatus
WfrSnDuprintV(
	char **		ps,
	size_t *	pn,
	const char *	format,
	va_list		ap
	)
{
	va_list		aq;
	WfrStatus	status;
	char *		s = NULL;
	int		s_len;
	size_t		s_size;


	if (WFR_NEWN(status, s, 1, char)) {
		va_copy(aq, ap);

		s_size = 1;
		s_len = vsnprintf(s, s_size, format, ap);

		if (s_len < 0) {
			status = WFR_STATUS_FROM_ERRNO();
		} else if (s_len == 0) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else if (WFR_RESIZE(status, s, s_size, (size_t)s_len + 1, char)) {
			s_len = vsnprintf(s, s_size, format, aq);
			s[((s_len < 0) ? 0 : s_len)] = '\0';
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		va_end(aq);
	}

	if (WFR_SUCCESS(status)) {
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
	static wchar_t		condition_msgW[128];
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
		condition_msgW[0] = L'\0';
		(void)pcre2_get_error_message(
			status.condition,
			condition_msgW, sizeof(condition_msgW));
		(void)WideCharToMultiByte(
			CP_UTF8, 0, condition_msgW,
			wcslen(condition_msgW) + 1,
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
WfrUpdateStringList(
	bool			addfl,
	bool			delfl,
	char **			plist,
	size_t *		plist_size,
	const char *const	trans_item
	)
{
	size_t		item_len;
	char *		list_new = NULL, *list_new_last;
	ptrdiff_t	list_new_delta;
	size_t		list_new_size = 0;
	WfrStatus	status;
	size_t		trans_item_len;


	if (addfl || delfl) {
		trans_item_len = strlen(trans_item);

		if ((*plist == NULL)
		&&  WFR_NEWN(status, *plist, 2, char))
		{
			(*plist)[0] = '\0'; (*plist)[1] = '\0';
			*plist_size = 1;
		}

		list_new_size = trans_item_len + 1 + *plist_size;
		if (WFR_NEWN(status, list_new, list_new_size, char)) {
			memset(list_new, '\0', list_new_size);
			list_new_last = list_new;

			if (addfl) {
				memcpy(list_new_last, trans_item, trans_item_len + 1);
				list_new_last += trans_item_len + 1;
			}

			for (char *item = *plist, *item_next = NULL;
			     item && *item; item = item_next)
			{
				if ((item_next = strchr(item, '\0'))) {
					item_len = item_next - item;
					item_next++;

					if ((trans_item_len != item_len)
					||  (strncmp(trans_item, item, item_len) != 0))
					{
						memcpy(list_new_last, item, item_len);
						list_new_last += item_len + 1;
					}
				}
			}

			if (&list_new_last[0] < &list_new[list_new_size - 1]) {
				list_new_delta = (&list_new[list_new_size - 1] - &list_new_last[0]);
				if ((list_new_size - list_new_delta) < 2) {
					if (WFR_RESIZE(status, list_new, list_new_size, 2, char)) {
						list_new[0] = '\0';
						list_new[1] = '\0';
					}
				} else {
					(void)WFR_RESIZE(status,
						list_new, list_new_size,
						list_new_size - list_new_delta, char);
				}
			} else {
				status = WFR_STATUS_CONDITION_SUCCESS;
			}

			if (WFR_SUCCESS(status)) {
				WFR_FREE(*plist);
				*plist = list_new;
				*plist_size = list_new_size;
			}
		}
	}

	if (WFR_FAILURE(status)) {
		WFR_FREE_IF_NOTNULL(list_new);
	}

	return status;
}

wchar_t *
WfrWcsNDup(
	const wchar_t *		inW,
	size_t			inW_len
	)
{
	wchar_t *	outW = NULL;
	size_t		outW_size;


	outW_size = inW_len + 1;
	if (WFR_NEWN1(outW, outW_size, wchar_t)) {
		memset(outW, 0, outW_size * sizeof(wchar_t));
		wcsncpy(outW, inW, inW_len);
		outW[inW_len] = L'\0';
	}

	return outW;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
