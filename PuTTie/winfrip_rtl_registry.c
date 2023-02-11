/*
 * winfrip_rtl_registry.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdarg.h>

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
	char *		lpName = NULL;
	DWORD		lpName_size;
	WfrStatus	status;
	LONG		status_registry;


	if (WFR_NEWN(status, lpName, MAX_PATH + 1, char)) {
		donefl = false;
		while (!donefl) {
			switch (status_registry = RegEnumKey(hKey, dwIndex, lpName, lpName_size)) {
			case ERROR_SUCCESS:
				donefl = true;
				status = WFR_STATUS_CONDITION_SUCCESS;
				break;

			case ERROR_MORE_DATA:
				if (WFR_FAILURE(status = WFR_RESIZE(
						lpName, lpName_size, lpName_size + 64, char)))
				{
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

	if (WFR_SUCCESS(status)) {
		*plpName = lpName;
	} else {
		WFR_FREE_IF_NOTNULL(lpName);
	}

	return status;
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

WfrStatus
WfrCleanupRegSubKey(
	const char *	key_name,
	const char *	subkey
	)
{
	HKEY		hKey = NULL;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfrOpenRegKeyRw(HKEY_CURRENT_USER, &hKey, key_name))
	&&  WFR_SUCCESS_LSTATUS(status, RegDeleteTree(hKey, subkey)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_IS_NOT_FOUND(status)) {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}
	if (hKey) {
		(void)RegCloseKey(hKey);
	}

	return status;
}

WfrStatus
WfrClearRegSubKey(
	const char *	subkey
	)
{
	bool			donefl;
	WfrEnumerateRegState *	enum_state;
	HKEY			hKey = NULL;
	size_t			itemc = 0;
	char **			itemv = NULL;
	char *			key_name;
	char *			key_name_escaped;
	WfrStatus		status;


	if (WFR_SUCCESS(status = WfrEnumerateRegInit(&enum_state, subkey, NULL))
	&&  enum_state->hKey
	&&  WFR_SUCCESS_WINDOWS(status, DuplicateHandle(
			GetCurrentProcess(), enum_state->hKey,
			GetCurrentProcess(), (LPHANDLE)&hKey,
			0, FALSE, DUPLICATE_SAME_ACCESS)))
	{
		do {
			if (WFR_SUCCESS(status = WfrEnumerateRegValues(
					&donefl, NULL, NULL, &key_name, NULL, &enum_state))
			&&  !donefl)
			{
				if (WFR_FAILURE(status = WFR_RESIZE(
						itemv, itemc, itemc + 1, char *)))
				{
					WFR_FREE(key_name);
				} else if (WFR_SUCCESS(status = WfrEscapeRegKey(
						(char *)key_name, &key_name_escaped)))
				{
					WFR_FREE(key_name);
					itemv[itemc - 1] = key_name_escaped;
				} else {
					WFR_FREE(key_name);
					itemv[itemc - 1] = NULL;
				}
			}
		} while (WFR_SUCCESS(status) && !donefl);

		for (size_t nitem = 0; WFR_SUCCESS(status) && (nitem < itemc); nitem++) {
			WFR_SUCCESS_LSTATUS(status, RegDeleteValue(hKey, itemv[nitem]));
		}

		for (size_t nitem = 0; nitem < itemc; nitem++) {
			WFR_FREE_IF_NOTNULL(itemv[nitem]);
		}
		WFR_FREE_IF_NOTNULL(itemv);
	}

	if (hKey) {
		(void)RegCloseKey(hKey);
	}

	return status;
}

WfrStatus
WfrDeleteRegSubKey(
	const char *	key_name,
	const char *	subkey
	)
{
	HKEY		hKey;
	LPSTR		subkey_escaped = NULL;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfrEscapeRegKey(subkey, &subkey_escaped))
	&&  WFR_SUCCESS(status = WfrOpenRegKeyRw(HKEY_CURRENT_USER, &hKey, key_name))
	&&  WFR_SUCCESS_LSTATUS(status, RegDeleteTree(hKey, subkey_escaped)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_FREE_IF_NOTNULL(subkey_escaped);
	if (hKey) {
		(void)RegCloseKey(hKey);
	}

	return status;
}

WfrStatus
WfrDeleteRegValue(
	const char *	key_name,
	const char *	value_name
	)
{
	HKEY		hKey;
	LPSTR		value_name_escaped = NULL;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfrEscapeRegKey(value_name, &value_name_escaped))
	&&  WFR_SUCCESS(status = WfrOpenRegKeyRw(HKEY_CURRENT_USER, &hKey, key_name))
	&&  WFR_SUCCESS_LSTATUS(status, RegDeleteValue(hKey, value_name_escaped)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_FREE_IF_NOTNULL(value_name_escaped);
	if (hKey) {
		(void)RegCloseKey(hKey);
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
	DWORD			item_data_len, item_data_size = 0;
	static char		item_name[32767]; // [see https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regenumvaluea]
	char *			item_name_unescaped;
	DWORD			item_name_len;
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
			item_data_len = item_data_size; item_name_len = sizeof(item_name);
			switch ((status_registry = RegEnumValue(
					(*pstate)->hKey, (*pstate)->dwIndex, item_name,
					&item_name_len, NULL, &dwType, item_data,
					&item_data_len)))
			{
			default:
				donefl = true; *pdonefl = false;
				status = WFR_STATUS_FROM_WINDOWS1(status_registry);
				break;

			case ERROR_MORE_DATA:
				item_data_len++;
				status = WFR_RESIZE_IF_NEQ_SIZE(
					item_data, item_data_size,
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
					item_data_len++;
					break;
				}
				item_name[item_name_len] = '\0';

				*pdonefl = false;

				if (WFR_SUCCESS(status = WfrUnescapeRegKey(
						item_name, &item_name_unescaped)))
				{
					if (pitem_data) {
						*pitem_data = item_data;
					}
					if (pitem_data_len) {
						*pitem_data_len = item_data_len;
					}
					*pitem_name = item_name_unescaped;
					if (pitem_type) {
						*pitem_type = item_type;
					}
					(*pstate)->dwIndex++;
				}

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
			if ((*key == ' ') || (*key == '\\') || (*key == '*')
			||  (*key == '?') || (*key == '%') || (*key < ' ')
			||  (*key > '~') || ((*key == '.') && !dotfl))
			{
				if (WFR_SUCCESS(status = WFR_RESIZE(
						key_escaped, key_escaped_size,
						key_escaped_size + 2, char)))
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
WfrLoadRegValue(
	const char *	key_name,
	const char *	subkey,
	char **		pvalue,
	size_t *	pvalue_size
	)
{
	HKEY		hKey = NULL;
	WfrStatus	status;
	char *		subkey_escaped = NULL;
	char *		value = NULL;
	DWORD		value_size = 0;


	if (WFR_SUCCESS(status = WfrEscapeRegKey(subkey, &subkey_escaped))
	&&  WFR_SUCCESS(status = WfrOpenRegKeyRo(HKEY_CURRENT_USER, &hKey, key_name))
	&&  WFR_SUCCESS_LSTATUS(status, RegGetValue(
			hKey, NULL, subkey_escaped, RRF_RT_REG_SZ, NULL, NULL, &value_size))
	&&  WFR_NEWN(status, value, value_size, char)
	&&  WFR_SUCCESS_LSTATUS(status, RegGetValue(
			hKey, NULL, subkey_escaped, RRF_RT_REG_SZ, NULL, value, &value_size)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (hKey != NULL) {
		(void)RegCloseKey(hKey);
	}
	WFR_FREE_IF_NOTNULL(subkey_escaped);

	if (WFR_SUCCESS(status)) {
		*pvalue = value;
		if (pvalue_size) {
			*pvalue_size = value_size;
		}
	} else {
		WFR_FREE_IF_NOTNULL(value);
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
	REGSAM		samDesired = KEY_READ | (writefl ? KEY_WRITE : 0);
	WfrStatus	status;
	LONG		status_registry;


	for (closefl = false, path = va_arg(ap, const char *);
	     path; path = va_arg(ap, const char *))
	{
		if (createfl) {
			status_registry = RegCreateKeyEx(
				hKey, path, 0, NULL,
				REG_OPTION_NON_VOLATILE, samDesired,
				NULL, &hKey_new, NULL);
		} else {
			status_registry = RegOpenKeyEx(
				hKey, path, 0, samDesired, &hKey_new);
		}

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
	char *		subkey_escaped = NULL, *subkey_new_escaped = NULL;
	wchar_t *	subkey_escaped_w = NULL, *subkey_new_escaped_w = NULL;


	status = WFR_STATUS_CONDITION_SUCCESS;

	if ((strcmp(subkey, subkey_new) != 0)
	&&  WFR_SUCCESS(status = WfrEscapeRegKey(subkey, &subkey_escaped))
	&&  WFR_SUCCESS(status = WfrToWcsDup(subkey_escaped, strlen(subkey_escaped) + 1, &subkey_escaped_w))
	&&  WFR_SUCCESS(status = WfrEscapeRegKey(subkey_new, &subkey_new_escaped))
	&&  WFR_SUCCESS(status = WfrToWcsDup(subkey_new_escaped, strlen(subkey_new_escaped) + 1, &subkey_new_escaped_w))
	&&  WFR_SUCCESS(status = WfrOpenRegKeyRw(HKEY_CURRENT_USER, &hKey, key_name))
	&&  WFR_SUCCESS_LSTATUS(status, RegRenameKey(hKey, subkey_escaped_w, subkey_new_escaped_w)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (hKey != NULL) {
		(void)RegCloseKey(hKey);
	}
	WFR_FREE_IF_NOTNULL(subkey_escaped);
	WFR_FREE_IF_NOTNULL(subkey_new_escaped);
	WFR_FREE_IF_NOTNULL(subkey_escaped_w);
	WFR_FREE_IF_NOTNULL(subkey_new_escaped_w);

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
	char *		value_name_escaped = NULL, *value_name_new_escaped = NULL;
	WfrStatus	status;
	char *		value = NULL;
	size_t		value_size;


	status = WFR_STATUS_CONDITION_SUCCESS;

	if ((strcmp(value_name, value_name_new) != 0)
	&&  WFR_SUCCESS(status = WfrEscapeRegKey(value_name, &value_name_escaped))
	&&  WFR_SUCCESS(status = WfrEscapeRegKey(value_name_new, &value_name_new_escaped))
	&&  WFR_SUCCESS(status = WfrLoadRegValue(key_name, value_name, &value, &value_size))
	&&  WFR_SUCCESS(status = WfrOpenRegKeyRw(HKEY_CURRENT_USER, &hKey, key_name))
	&&  WFR_SUCCESS_LSTATUS(status, RegSetValueEx(
			hKey, value_name_new_escaped, 0, REG_SZ,
			(const BYTE *)value, value_size))
	&&  WFR_SUCCESS_LSTATUS(status, RegDeleteValue(hKey, value_name_escaped)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (hKey != NULL) {
		(void)RegCloseKey(hKey);
	}
	WFR_FREE_IF_NOTNULL(value);
	WFR_FREE_IF_NOTNULL(value_name_escaped);
	WFR_FREE_IF_NOTNULL(value_name_new_escaped);

	return status;
}

WfrStatus
WfrSetRegValue(
	const char *	key_name,
	const char *	value_name,
	const char *	value,
	size_t		value_size
	)
{
	HKEY		hKey;
	char *		value_name_escaped = NULL;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfrEscapeRegKey(value_name, &value_name_escaped))
	&&  WFR_SUCCESS(status = WfrCreateRegKey(HKEY_CURRENT_USER, &hKey, key_name))
	&&  WFR_SUCCESS_LSTATUS(status, RegSetValueEx(
			hKey, value_name_escaped, 0, REG_SZ,
			(const BYTE *)value, value_size)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (hKey != NULL) {
		(void)RegCloseKey(hKey);
	}
	WFR_FREE_IF_NOTNULL(value_name_escaped);

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
			if ((key_escaped[0] == '%')
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

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
