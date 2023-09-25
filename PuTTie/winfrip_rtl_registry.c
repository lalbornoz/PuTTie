/*
 * winfrip_rtl_registry.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdarg.h>
#include <stdio.h>

#include <windows.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_registry.h"

/*
 * Private subroutine prototypes
 */

static WfrStatus	WfrpEnumerateRegKey(HKEY hKey, DWORD dwIndex, char **plpName);

/*
 * Private subroutines
 */

static WfrStatus
WfrpEnumerateRegKey(
	HKEY		hKey,
	DWORD		dwIndex,
	char **		plpName
	)
{
	bool		donefl;
	char *		lpName;
	wchar_t *	lpNameW = NULL;
	DWORD		lpNameW_size;
	WfrStatus	status;
	LONG		status_registry;


	lpNameW_size = MAX_PATH + 1;
	if (WFR_NEWN(status, lpNameW, lpNameW_size, wchar_t)) {
		donefl = false;
		while (!donefl) {
			switch (status_registry = RegEnumKeyW(hKey, dwIndex, lpNameW, lpNameW_size)) {
			case ERROR_SUCCESS:
				donefl = true;
				status = WFR_STATUS_CONDITION_SUCCESS;
				break;

			case ERROR_MORE_DATA:
				if (!WFR_RESIZE(status, lpNameW, lpNameW_size, lpNameW_size + 64, wchar_t)) {
					donefl = true;
				}
				break;

			default:
				donefl = true;
				status = WFR_STATUS_FROM_WINDOWS1(status_registry);
				break;
			}
		}
	}

	if (WFR_SUCCESS(status)
	&&  WFR_SUCCESS(status = WfrConvertUtf16ToUtf8String(
			lpNameW, (lpNameW_size ? (lpNameW_size - 1) : lpNameW_size),
			&lpName)))
	{
		*plpName = lpName;
	} else {
		WFR_FREE_IF_NOTNULL(lpNameW);
	}

	return status;
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

WfrStatus
WfrClearRegSubKey(
	bool		noentfl,
	const char *	subkey
	)
{
	HKEY		hKey = NULL;
	LPSTR		subkey_escaped = NULL;
	wchar_t *	subkey_escapedW = NULL;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfrEscapeRegKey(subkey, &subkey_escaped))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(subkey_escaped, strlen(subkey_escaped), &subkey_escapedW))
	&&  WFR_SUCCESS(status = WfrOpenRegKeyRw(HKEY_CURRENT_USER, &hKey, subkey))
	&&  WFR_SUCCESS_LSTATUS(status, RegDeleteTreeW(hKey, subkey_escapedW)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_FREE_IF_NOTNULL(subkey_escaped);
	WFR_FREE_IF_NOTNULL(subkey_escapedW);
	if (hKey) {
		(void)RegCloseKey(hKey);
	}

	if (WFR_STATUS_IS_NOT_FOUND(status) && noentfl) {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfrDeleteRegSubKey(
	bool		noentfl,
	const char *	key_name,
	const char *	subkey
	)
{
	HKEY		hKey = NULL;
	LPSTR		subkey_escaped = NULL;
	wchar_t *	subkey_escapedW = NULL;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfrEscapeRegKey(subkey, &subkey_escaped))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(subkey_escaped, strlen(subkey_escaped), &subkey_escapedW))
	&&  WFR_SUCCESS(status = WfrOpenRegKeyRw(HKEY_CURRENT_USER, &hKey, key_name))
	&&  WFR_SUCCESS_LSTATUS(status, RegDeleteTreeW(hKey, subkey_escapedW))
	&&  WFR_SUCCESS_LSTATUS(status, RegDeleteKeyW(hKey, subkey_escapedW)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_FREE_IF_NOTNULL(subkey_escaped);
	WFR_FREE_IF_NOTNULL(subkey_escapedW);
	if (hKey) {
		(void)RegCloseKey(hKey);
	}

	if (WFR_STATUS_IS_NOT_FOUND(status) && noentfl) {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfrDeleteRegValue(
	bool		noentfl,
	const char *	key_name,
	const char *	value_name
	)
{
	HKEY		hKey = NULL;
	LPSTR		value_name_escaped = NULL;
	wchar_t *	value_name_escapedW = NULL;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfrEscapeRegKey(value_name, &value_name_escaped))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(value_name_escaped, strlen(value_name_escaped), &value_name_escapedW))
	&&  WFR_SUCCESS(status = WfrOpenRegKeyRw(HKEY_CURRENT_USER, &hKey, key_name))
	&&  WFR_SUCCESS_LSTATUS(status, RegDeleteValueW(hKey, value_name_escapedW)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_FREE_IF_NOTNULL(value_name_escaped);
	WFR_FREE_IF_NOTNULL(value_name_escapedW);
	if (hKey) {
		(void)RegCloseKey(hKey);
	}

	if (WFR_STATUS_IS_NOT_FOUND(status) && noentfl) {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

void
WfrEnumerateRegCancel(
	WfrEnumerateRegState **		pstate
	)
{
	if (*pstate) {
		if ((*pstate)->hKey) {
			(void)RegCloseKey((*pstate)->hKey);
		}
		WFR_FREE(*pstate); *pstate = NULL;
	}
}

WfrStatus
WfrEnumerateRegInit(
	WfrEnumerateRegState **		pstate,
					...
	)
{
	va_list		ap;
	WfrStatus	status;


	if (WFR_NEW(status, (*pstate), WfrEnumerateRegState)) {
		WFR_ENUMERATE_REG_STATE_INIT((**pstate));
		va_start(ap, pstate);
		if (WFR_FAILURE(status = WfrOpenRegKeyRoV(
				HKEY_CURRENT_USER, &(*pstate)->hKey, ap)))
		{
			if (WFR_STATUS_IS_NOT_FOUND(status)) {
				status = WFR_STATUS_CONDITION_SUCCESS;
			} else {
				WFR_FREE((*pstate));
			}
		}
		va_end(ap);
	}

	return status;
}

WfrStatus
WfrEnumerateRegKeys(
	bool *				pdonefl,
	char **				pitem_name,
	WfrEnumerateRegState **		pstate
	)
{
	char *		item_name = NULL, *item_name_escaped;
	WfrStatus	status;


	if (!(*pstate)->hKey) {
		*pdonefl = (*pstate)->donefl = true;
		*pitem_name = NULL;
		return WFR_STATUS_CONDITION_SUCCESS;

	}

	if ((*pstate)->donefl) {
		*pdonefl = (*pstate)->donefl = true;
		*pitem_name = NULL;
		status = WFR_STATUS_CONDITION_SUCCESS;
	} else if (WFR_FAILURE(status = WfrpEnumerateRegKey(
			(*pstate)->hKey, (*pstate)->dwIndex, &item_name)))
	{
		if (WFR_STATUS_CONDITION(status) == ERROR_NO_MORE_ITEMS) {
			*pdonefl = true;
			*pitem_name = NULL;
			WfrEnumerateRegCancel(pstate);
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	} else if (WFR_SUCCESS(status = WfrUnescapeRegKey(
			item_name, &item_name_escaped)))
	{
		*pdonefl = false;
		*pitem_name = item_name_escaped;
		(*pstate)->dwIndex++;
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_FREE_IF_NOTNULL(item_name);

	return status;
}

WfrStatus
WfrEnumerateRegValues(
	bool *				pdonefl,
	void **				pitem_data,
	size_t *			pitem_data_len,
	char **				pitem_name,
	DWORD *				pitem_type,
	WfrEnumerateRegState **		pstate
	)
{
	bool			donefl;
	DWORD			dwType;
	void *			item_data = NULL;
	char *			item_data_;
	DWORD			item_data_len;
	DWORD			item_data_size = 0;
	size_t			item_data_size_;
	char *			item_name = NULL;
	static wchar_t		item_nameW[32767]; // [see https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regenumvaluea]
	char *			item_name_unescaped;
	DWORD			item_nameW_len;
	DWORD			item_type;
	WfrStatus		status;
	LSTATUS			status_registry;


	donefl = false;
	status = WFR_STATUS_CONDITION_SUCCESS;
	if (*pdonefl) {
		return status;
	}

	if (!(*pstate)->hKey) {
		donefl = true; *pdonefl = true;
		*pitem_name = NULL;
		WFR_ENUMERATE_REG_STATE_INIT(**pstate);
		return WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_NEWN(status, item_data, 1, char)) {
		item_data_size = 1;

		do {
			item_data_len = item_data_size;
			item_nameW_len = sizeof(item_nameW);

			switch ((status_registry = RegEnumValueW(
					(*pstate)->hKey, (*pstate)->dwIndex, item_nameW,
					&item_nameW_len, NULL, &dwType, item_data,
					&item_data_len)))
			{
			default:
				donefl = true; *pdonefl = false;
				status = WFR_STATUS_FROM_WINDOWS1(status_registry);
				break;

			case ERROR_MORE_DATA:
				item_data_len += sizeof(wchar_t);
				(void)WFR_RESIZE_IF_NEQ_SIZE(
					status, item_data, item_data_size,
					item_data_len, char);
				break;

			case ERROR_NO_MORE_ITEMS:
				WFR_FREE(item_data);
				donefl = true;

				*pdonefl = true;
				*pitem_name = NULL;
				WfrEnumerateRegCancel(pstate);
				status = WFR_STATUS_CONDITION_SUCCESS;
				break;

			case ERROR_SUCCESS:
				donefl = true;

				item_data_ = NULL;

				switch (dwType) {
				default:
					donefl = true; *pdonefl = false;
					status = WFR_STATUS_FROM_ERRNO1(EINVAL);
					goto out;

				case REG_DWORD:
					item_type = REG_DWORD;
					item_data_len = sizeof(int);
					break;
				case REG_SZ:
					item_type = REG_SZ;
					item_data_len += sizeof(wchar_t);
					status = WfrConvertUtf16ToUtf8String1(
							item_data,
							WFR_DIV_ROUNDUP(item_data_len, sizeof(wchar_t)),
							&item_data_, &item_data_size_);
					if (WFR_SUCCESS(status)) {
						WFR_FREE(item_data);
					}
					break;
				}
				item_nameW[item_nameW_len] = L'\0';

				*pdonefl = false;

				if (WFR_SUCCESS(status = WfrConvertUtf16ToUtf8String(
						item_nameW, item_nameW_len,
						&item_name))
				&&  WFR_SUCCESS(status = WfrUnescapeRegKey(
						item_name, &item_name_unescaped)))
				{
					if (pitem_data) {
						if (item_data_) {
							*pitem_data = item_data_;
						} else {
							*pitem_data = item_data;
						}
					}
					if (pitem_data_len) {
						*pitem_data_len = item_data_
							? item_data_size_
							: item_data_len;
					}
					*pitem_name = item_name_unescaped;
					if (pitem_type) {
						*pitem_type = item_type;
					}
					(*pstate)->dwIndex++;
				} else {
					WFR_FREE_IF_NOTNULL(item_data_);
				}

				WFR_FREE_IF_NOTNULL(item_name);

			out:
				break;
			}
		} while (!(*pdonefl) && !donefl);
	}

	if (WFR_FAILURE(status)) {
		WFR_FREE_IF_NOTNULL(item_data);
	}

	return status;
}

WfrStatus
WfrEscapeRegKey(
	const char *	key,
	char **		pkey_escaped
	)
{
	char *			key_escaped = NULL;
	size_t			key_escaped_size;
	size_t			key_len;
	bool			dotfl = false;
	size_t			npos;
	static const char	hex_digits[16] = "0123456789ABCDEF";
	WfrStatus		status;


	key_len = strlen(key);
	key_escaped_size = key_len + 1;
	npos = 0;

	if (WFR_NEWN(status, key_escaped, key_escaped_size, char)) {
		while (*key) {
			if (((key[0] & 0xC0) == 0xC0)
			 && ((key[1] & 0x80) == 0x80))
			{
				key_escaped[npos++] = *key++;
				key_escaped[npos++] = *key;
			} else if (((key[0] & 0xE0) == 0xE0)
				&& ((key[1] & 0x80) == 0x80)
				&& ((key[2] & 0x80) == 0x80))
			{
				key_escaped[npos++] = *key++;
				key_escaped[npos++] = *key++;
				key_escaped[npos++] = *key;
			} else if (((key[0] & 0xF0) == 0xF0)
				&& ((key[1] & 0x80) == 0x80)
				&& ((key[2] & 0x80) == 0x80)
				&& ((key[3] & 0x80) == 0x80))
			{
				key_escaped[npos++] = *key++;
				key_escaped[npos++] = *key++;
				key_escaped[npos++] = *key++;
				key_escaped[npos++] = *key;
			} else if ((*key == ' ') || (*key == '\\') || (*key == '*')
				|| (*key == '?') || (*key == '%')  || ((*key == '.') && !dotfl)
				|| (*key < ' ')  ||  (*key > '~'))
			{
				if (WFR_RESIZE(status,
						key_escaped, key_escaped_size,
						key_escaped_size + 2, char))
				{
					key_escaped[npos++] = '%';
					key_escaped[npos++] = hex_digits[((unsigned char)*key) >> 4];
					key_escaped[npos++] = hex_digits[((unsigned char)*key)  & 15];
				} else {
					break;
				}
			} else {
				key_escaped[npos++] = *key;
			}

			dotfl = true;
			key++;
		}

		if (WFR_SUCCESS(status)) {
			key_escaped[npos] = '\0';
			*pkey_escaped = key_escaped;
		} else {
			WFR_FREE(key_escaped);
		}
	}

	return status;
}

WfrStatus
WfrGetRegStringListValue(
	const char *	subkey,
	const char *	value_name,
	HKEY *		phKey,
	char **		plist,
	DWORD *		plist_size
	)
{
	HKEY		hKey = NULL;
	char *		list;
	DWORD		list_size;
	size_t		list_size_;
	wchar_t *	listW = NULL;
	size_t		listW_len;
	DWORD		listW_size = 0;
	WfrStatus	status;
	char *		value_name_escaped = NULL;
	wchar_t *	value_name_escapedW = NULL;


	if (WFR_SUCCESS(status = WfrOpenRegKeyRo(
			HKEY_CURRENT_USER, &hKey, subkey))
	&&  WFR_SUCCESS(status = WfrEscapeRegKey(value_name, &value_name_escaped))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(
			value_name_escaped, strlen(value_name_escaped),
			&value_name_escapedW))
	&&  WFR_SUCCESS_LSTATUS(status, RegGetValueW(
			hKey, NULL, value_name_escapedW,
			RRF_RT_REG_MULTI_SZ, NULL, NULL,
			&listW_size))
	&&  WFR_SUCCESS_ERRNO1(status, ENOENT, (listW_size >= (2 * 2)))
	&&  WFR_NEWN_CAST(status, listW, listW_size, char, wchar_t)
	&&  WFR_SUCCESS_LSTATUS(status, RegGetValueW(
			hKey, NULL, value_name_escapedW,
			RRF_RT_REG_MULTI_SZ, NULL, listW,
			&listW_size)))
	{
		listW_len = WFR_DIV_ROUNDDOWN(listW_size, sizeof(wchar_t));
		listW[listW_len - 2] = L'\0';
		listW[listW_len - 1] = L'\0';

		if (WFR_SUCCESS(status = WfrConvertUtf16ToUtf8String1(
				listW, listW_len - 1,
				&list, &list_size_)))
		{
			*plist = list;
			if (plist_size) {
				list_size = list_size_;
				*plist_size = list_size;
			}
		}
	}

	if (WFR_FAILURE(status)) {
		WFR_FREE_IF_NOTNULL(listW);
		WFR_FREE_IF_NOTNULL(value_name_escaped);
		WFR_FREE_IF_NOTNULL(value_name_escapedW);
	}

	if (hKey) {
		if (phKey) {
			*phKey = hKey;
		} else {
			(void)RegCloseKey(hKey);
		}
	}

	return status;
}

WfrStatus
WfrLoadRegStringValue(
	const char *	key_name,
	const char *	subkey,
	char **		pvalue,
	size_t *	pvalue_size
	)
{
	HKEY		hKey = NULL;
	WfrStatus	status;
	char *		subkey_escaped = NULL;
	wchar_t *	subkey_escapedW = NULL;
	char *		value = NULL;
	size_t		value_size;
	wchar_t *	valueW = NULL;
	DWORD		valueW_size = 0;


	if (WFR_SUCCESS(status = WfrEscapeRegKey(subkey, &subkey_escaped))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(subkey_escaped, strlen(subkey_escaped), &subkey_escapedW))
	&&  WFR_SUCCESS(status = WfrOpenRegKeyRo(HKEY_CURRENT_USER, &hKey, key_name))
	&&  WFR_SUCCESS_LSTATUS(status, RegGetValueW(
			hKey, NULL, subkey_escapedW,
			RRF_RT_REG_SZ, NULL, NULL, &valueW_size))
	&&  WFR_NEWN_CAST(status, valueW, valueW_size, char, wchar_t)
	&&  WFR_SUCCESS_LSTATUS(status, RegGetValueW(
			hKey, NULL, subkey_escapedW,
			RRF_RT_REG_SZ, NULL, valueW, &valueW_size))
	&&  WFR_SUCCESS(status = WfrConvertUtf16ToUtf8String1(
			valueW, (valueW_size / sizeof(valueW[0])) - 1,
			&value, &value_size)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (hKey != NULL) {
		(void)RegCloseKey(hKey);
	}
	WFR_FREE_IF_NOTNULL(subkey_escaped);
	WFR_FREE_IF_NOTNULL(subkey_escapedW);

	if (WFR_SUCCESS(status)) {
		*pvalue = value;
		if (pvalue_size) {
			*pvalue_size = value_size;
		}
	} else {
		WFR_FREE_IF_NOTNULL(value);
		WFR_FREE_IF_NOTNULL(valueW);
	}

	return status;
}

WfrStatus
WfrOpenRegKey(
	HKEY		hKey,
	bool		createfl,
	bool		writefl,
	HKEY *		phKey,
			...
	)
{
	va_list		ap;
	WfrStatus	status;


	va_start(ap, phKey);
	status = WfrOpenRegKeyV(hKey, createfl, writefl, phKey, ap);
	va_end(ap);

	return status;
}

WfrStatus
WfrOpenRegKeyV(
	HKEY		hKey,
	bool		createfl,
	bool		writefl,
	HKEY *		phKey,
	va_list		ap
	)
{
	bool		closefl;
	HKEY		hKey_new;
	const char *	path;
	wchar_t *	pathW;
	REGSAM		samDesired = KEY_READ | (writefl ? KEY_WRITE : 0);
	WfrStatus	status;
	LONG		status_registry;


	for (closefl = false, path = va_arg(ap, const char *);
	     path; path = va_arg(ap, const char *))
	{
		if (WFR_FAILURE(status = WfrConvertUtf8ToUtf16String(
				path, strlen(path), &pathW)))
		{
			break;
		} else if (createfl) {
			status_registry = RegCreateKeyExW(
				hKey, pathW, 0, NULL,
				REG_OPTION_NON_VOLATILE, samDesired,
				NULL, &hKey_new, NULL);
		} else {
			status_registry = RegOpenKeyExW(
				hKey, pathW, 0, samDesired, &hKey_new);
		}
		WFR_FREE(pathW);

		if (status_registry == ERROR_SUCCESS) {
			if (closefl) {
				(void)RegCloseKey(hKey);
			}
			hKey = hKey_new; closefl = true;
		} else {
			if (closefl) {
				(void)RegCloseKey(hKey);
			}
			break;
		}
	}

	if (status_registry == ERROR_SUCCESS) {
		*phKey = hKey;
		status = WFR_STATUS_CONDITION_SUCCESS;
	} else {
		status = WFR_STATUS_FROM_WINDOWS1(status_registry);
	}

	return status;
}

WfrStatus
WfrRenameRegSubKey(
	const char *	key_name,
	const char *	subkey,
	const char *	subkey_new
	)
{
	HKEY		hKey = NULL;
	WfrStatus	status;
	char *		subkey_escaped = NULL;
	wchar_t *	subkey_escapedW = NULL;
	char *		subkey_new_escaped = NULL;
	wchar_t *	subkey_new_escapedW = NULL;


	status = WFR_STATUS_CONDITION_SUCCESS;

	if ((strcmp(subkey, subkey_new) != 0)
	&&  WFR_SUCCESS(status = WfrEscapeRegKey(subkey, &subkey_escaped))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(subkey_escaped, strlen(subkey_escaped), &subkey_escapedW))
	&&  WFR_SUCCESS(status = WfrEscapeRegKey(subkey_new, &subkey_new_escaped))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(subkey_new_escaped, strlen(subkey_new_escaped), &subkey_new_escapedW))
	&&  WFR_SUCCESS(status = WfrOpenRegKeyRw(HKEY_CURRENT_USER, &hKey, key_name))
	&&  WFR_SUCCESS_LSTATUS(status, RegRenameKey(hKey, subkey_escapedW, subkey_new_escapedW)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (hKey != NULL) {
		(void)RegCloseKey(hKey);
	}
	WFR_FREE_IF_NOTNULL(subkey_escaped);
	WFR_FREE_IF_NOTNULL(subkey_escapedW);
	WFR_FREE_IF_NOTNULL(subkey_new_escaped);
	WFR_FREE_IF_NOTNULL(subkey_new_escapedW);

	return status;
}

WfrStatus
WfrRenameRegValue(
	const char *	key_name,
	const char *	value_name,
	const char *	value_name_new
	)
{
	HKEY		hKey = NULL;
	WfrStatus	status;
	char *		value = NULL;
	wchar_t *	valueW = NULL;
	char *		value_name_escaped = NULL;
	wchar_t *	value_name_escapedW = NULL;
	char *		value_name_new_escaped = NULL;
	wchar_t *	value_name_new_escapedW = NULL;
	size_t		value_size;
	size_t		valueW_size;


	status = WFR_STATUS_CONDITION_SUCCESS;

	if ((strcmp(value_name, value_name_new) != 0)
	&&  WFR_SUCCESS(status = WfrEscapeRegKey(value_name, &value_name_escaped))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(value_name_escaped, strlen(value_name_escaped), &value_name_escapedW))
	&&  WFR_SUCCESS(status = WfrEscapeRegKey(value_name_new, &value_name_new_escaped))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(value_name_new_escaped, strlen(value_name_new_escaped), &value_name_new_escapedW))
	&&  WFR_SUCCESS(status = WfrLoadRegStringValue(key_name, value_name, &value, &value_size))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String1(value, value_size, &valueW, &valueW_size))
	&&  WFR_SUCCESS(status = WfrOpenRegKeyRw(HKEY_CURRENT_USER, &hKey, key_name))
	&&  WFR_SUCCESS_LSTATUS(status, RegSetValueExW(
			hKey, value_name_new_escapedW, 0, REG_SZ,
			(const BYTE *)valueW, valueW_size * sizeof(valueW[0])))
	&&  WFR_SUCCESS_LSTATUS(status, RegDeleteValueW(hKey, value_name_escapedW)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (hKey != NULL) {
		(void)RegCloseKey(hKey);
	}

	WFR_FREE_IF_NOTNULL(value);
	WFR_FREE_IF_NOTNULL(valueW);
	WFR_FREE_IF_NOTNULL(value_name_escaped);
	WFR_FREE_IF_NOTNULL(value_name_escapedW);
	WFR_FREE_IF_NOTNULL(value_name_new_escaped);
	WFR_FREE_IF_NOTNULL(value_name_new_escapedW);

	return status;
}

WfrStatus
WfrSetRegValue(
	const char *	key_name,
	const char *	value_name,
	const char *	value,
	size_t		value_size,
	DWORD		dwType
	)
{
	bool		convertfl;
	HKEY		hKey = NULL;
	char *		value_name_escaped = NULL;
	wchar_t *	value_name_escapedW = NULL;
	wchar_t *	valueW = NULL;
	size_t		valueW_size;
	WfrStatus	status;


	if (dwType == REG_MULTI_SZ) {
		convertfl = true;
		if (!((value_size >= 2)
		&&    (value[value_size - 2] == '\0')
		&&    (value[value_size - 1] == '\0')))
		{
			status = WFR_STATUS_FROM_ERRNO1(EINVAL);
		} else {
			value_size -= 1;
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	} else if (dwType == REG_SZ) {
		convertfl = true;
		value_size += 1;
		status = WFR_STATUS_CONDITION_SUCCESS;
	} else {
		convertfl = false;
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_SUCCESS(status = WfrEscapeRegKey(
			value_name, &value_name_escaped))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(
			value_name_escaped, strlen(value_name_escaped),
			&value_name_escapedW))
	&&  (convertfl
	    ? WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String1(
			value, value_size, &valueW, &valueW_size))
	    : true)
	&&  WFR_SUCCESS(status = WfrCreateRegKey(
			HKEY_CURRENT_USER, &hKey, key_name))
	&&  WFR_SUCCESS_LSTATUS(status, RegSetValueExW(
			hKey, value_name_escapedW, 0, dwType,
			(convertfl ? (const BYTE *)valueW : (const BYTE *)value),
			(convertfl ? (valueW_size * sizeof(valueW[0])) : value_size))))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (hKey != NULL) {
		(void)RegCloseKey(hKey);
	}

	WFR_FREE_IF_NOTNULL(valueW);
	WFR_FREE_IF_NOTNULL(value_name_escaped);
	WFR_FREE_IF_NOTNULL(value_name_escapedW);

	return status;
}

WfrStatus
WfrUnescapeRegKey(
	const char *	key_escaped,
	char **		pkey
	)
{
	unsigned	digits[2];
	char *		key = NULL;
	size_t		key_escaped_len;
	size_t		npos;
	WfrStatus	status;


	key_escaped_len = strlen(key_escaped);
	npos = 0;

	if (WFR_NEWN(status, key, key_escaped_len + 1, char)) {
		while (*key_escaped) {
			if (  (key_escaped[0] == '%')
			&&  (((key_escaped[1] >= '0') && (key_escaped[1] <= '9'))
			||   ((key_escaped[1] >= 'a') && (key_escaped[1] <= 'f'))
			||   ((key_escaped[1] >= 'A') && (key_escaped[1] <= 'F')))
			&&  (((key_escaped[2] >= '0') && (key_escaped[2] <= '9'))
			||   ((key_escaped[2] >= 'a') && (key_escaped[2] <= 'f'))
			||   ((key_escaped[2] >= 'A') && (key_escaped[2] <= 'F'))))
			{
				digits[0] = key_escaped[1] - '0';
				digits[0] -= ((digits[0] > 9) ? 7 : 0);
				digits[1] = key_escaped[2] - '0';
				digits[1] -= ((digits[1] > 9) ? 7 : 0);
				key[npos++] = (digits[0] << 4) + digits[1];
				key_escaped += 3;
			} else {
				key[npos++] = *key_escaped;
				key_escaped += 1;
			}
		}

		key[npos] = '\0';
		*pkey = key;
	}

	return status;
}

WfrStatus
WfrUpdateRegStringList(
	bool			addfl,
	bool			delfl,
	const char *		subkey,
	const char *const	trans_item,
	const char *		value_name
	)
{
	HKEY		hKey = NULL;
	char *		list = NULL;
	DWORD		list_size;
	size_t		list_size_;
	WfrStatus	status;


	if (addfl || delfl) {
		if (WFR_FAILURE(status = WfrGetRegStringListValue(
				subkey, value_name, &hKey, &list, &list_size)))
		{
			list = NULL;
			list_size_ = 0;
		} else {
			list_size_ = list_size;
		}

		if (WFR_SUCCESS(status = WfrUpdateStringList(
				addfl, delfl, &list,
				&list_size_, trans_item))
		&&  WFR_SUCCESS(status = WfrSetRegValue(
				subkey, value_name, list,
				list_size_, REG_MULTI_SZ)))
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	if (hKey != NULL) {
		(void)RegCloseKey(hKey);
	}
	WFR_FREE_IF_NOTNULL(list);

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
