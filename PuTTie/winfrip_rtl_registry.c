/*
 * winfrip_rtl_registry.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#pragma GCC diagnostic pop

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


	if (!(lpName = WFR_NEWN(MAX_PATH + 1, char))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		donefl = false;
		while (!donefl) {
			switch (status_registry = RegEnumKey(hKey, dwIndex, lpName, lpName_size)) {
			case ERROR_SUCCESS:
				donefl = true;
				status = WFR_STATUS_CONDITION_SUCCESS;
				break;

			case ERROR_MORE_DATA:
				if (WFR_STATUS_FAILURE(status = WFR_RESIZE(
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

	if (WFR_STATUS_SUCCESS(status)) {
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
WfrEnumerateRegInit(
	WfrEnumerateRegState **		state,
					...
	)
{
	va_list		ap;
	WfrStatus	status;


	if (!(((*state) = WFR_NEW(WfrEnumerateRegState)))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		WFR_ENUMERATE_REG_STATE_INIT((**state));
		va_start(ap, state);
		if (WFR_STATUS_FAILURE(status = WfrOpenRegKeyRoV(
				HKEY_CURRENT_USER, &(*state)->hKey, ap)))
		{
			if ((WFR_STATUS_CONDITION(status) == ERROR_FILE_NOT_FOUND)
			||  (WFR_STATUS_CONDITION(status) == ERROR_PATH_NOT_FOUND))
			{
				status = WFR_STATUS_CONDITION_SUCCESS;
			} else {
				WFR_FREE((*state));
			}
		}
		va_end(ap);
	}

	return status;
}

WfrStatus
WfrEnumerateRegKeys(
	bool *			pdonefl,
	char **			pitem_name,
	WfrEnumerateRegState *	state
	)
{
	char *		item_name = NULL, *item_name_escaped;
	WfrStatus	status;


	if (!state->hKey) {
		*pdonefl = state->donefl = true;
		*pitem_name = NULL;
		return WFR_STATUS_CONDITION_SUCCESS;

	}

	if (state->donefl) {
		*pdonefl = state->donefl = true;
		*pitem_name = NULL;
		status = WFR_STATUS_CONDITION_SUCCESS;
	} else if (WFR_STATUS_FAILURE(status = WfrpEnumerateRegKey(
			state->hKey, state->dwIndex, &item_name)))
	{
		if (WFR_STATUS_CONDITION(status) == ERROR_NO_MORE_ITEMS) {
			*pdonefl = true;
			*pitem_name = NULL;
			(void)RegCloseKey(state->hKey);
			WFR_FREE(state);
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	} else if (WFR_STATUS_SUCCESS(status = WfrUnescapeRegKey(
			item_name, &item_name_escaped)))
	{
		*pdonefl = false;
		*pitem_name = item_name_escaped;
		state->dwIndex++;
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_FREE_IF_NOTNULL(item_name);

	return status;
}

WfrStatus
WfrEnumerateRegValues(
	bool *			pdonefl,
	void **			pitem_data,
	size_t *		pitem_data_len,
	char **			pitem_name,
	DWORD *			pitem_type,
	WfrEnumerateRegState *	state
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

	if (!state->hKey) {
		donefl = true; *pdonefl = true;
		*pitem_name = NULL;
		WFR_ENUMERATE_REG_STATE_INIT(*state);
		return WFR_STATUS_CONDITION_SUCCESS;
	}

	if (!(item_data = WFR_NEWN(1, char))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		item_data_size = 1;

		do {
			item_data_len = item_data_size; item_name_len = sizeof(item_name);
			switch ((status_registry = RegEnumValue(
					state->hKey, state->dwIndex, item_name,
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
				(void)RegCloseKey(state->hKey);
				WFR_FREE(state);
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

				if (WFR_STATUS_SUCCESS(status = WfrUnescapeRegKey(
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
					state->dwIndex++;
				}

			out:
				break;
			}
		} while (!(*pdonefl) && !donefl);
	}

	if (WFR_STATUS_FAILURE(status)) {
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
	if (!(key_escaped = WFR_NEWN(key_escaped_size, char))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		npos = 0;
		status = WFR_STATUS_CONDITION_SUCCESS;

		while (*key) {
			if ((*key == ' ') || (*key == '\\') || (*key == '*')
			||  (*key == '?') || (*key == '%') || (*key < ' ')
			||  (*key > '~') || ((*key == '.') && !dotfl))
			{
				if (WFR_STATUS_SUCCESS(status = WFR_RESIZE(
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

		if (WFR_STATUS_SUCCESS(status)) {
			key_escaped[npos] = '\0';
			*pkey_escaped = key_escaped;
		} else {
			WFR_FREE(key_escaped);
		}
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
	if (!(key = WFR_NEWN(key_escaped_len + 1, char))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		npos = 0;
		status = WFR_STATUS_CONDITION_SUCCESS;

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
