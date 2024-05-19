/*
 * winfrip_rtl_save.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <windows.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_file.h"
#include "PuTTie/winfrip_rtl_registry.h"
#include "PuTTie/winfrip_rtl_save.h"
#include "PuTTie/winfrip_rtl_pcre2.h"

/*
 * Private subroutine prototypes
 */

WfrStatus WfrpSaveGetCreateTempRootDirectory(wchar_t **pdname_tmpW);

/*
 * Private subroutines
 */

WfrStatus
WfrpSaveGetCreateTempRootDirectory(
	wchar_t **	pdname_tmpW
)
{
	wchar_t *	dname_tmpW;
	struct _stat64	statbuf;
	WfrStatus	status;


	if (!(dname_tmpW = _wgetenv(L"TEMP"))
	||  !(dname_tmpW = _wgetenv(L"TMP"))) {
		dname_tmpW = (wchar_t *)L".\\";
	}

	WFR_STATUS_BIND_POSIX(status, (_wstat64(dname_tmpW, &statbuf) == 0));
	if (WFR_SUCCESS(status))
	{
		*pdname_tmpW = dname_tmpW;
	} else if (WFR_FAILURE(status)
		&& WFR_STATUS_IS_NOT_FOUND(status)
		&& WFR_SUCCESS(status = WfrMakeDirectoryW(dname_tmpW, true)))
	{
		*pdname_tmpW = dname_tmpW;
	}

	return status;
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

WfrStatus
WfrSaveListToFile(
	const char *	fname,
	const char *	fname_tmp,
	const char *	list,
	size_t		list_size
	)
{
	char *		dname_tmp = NULL;
	wchar_t *	dname_tmpW;
	FILE *		file = NULL;
	wchar_t *	fnameW = NULL;
	ssize_t		nwritten;
	char		pname_tmp[MAX_PATH + 1];
	wchar_t		pname_tmpW[MAX_PATH + 1];
	wchar_t *	pname_tmpW_ = NULL;
	WfrStatus	status;


	if (WFR_FAILURE(status = WfrpSaveGetCreateTempRootDirectory(&dname_tmpW))) {
		return status;
	}
	if (WFR_SUCCESS(status = WfrConvertUtf16ToUtf8String(dname_tmpW, wcslen(dname_tmpW), &dname_tmp))) {
		WFR_SNPRINTF(
			pname_tmp, sizeof(pname_tmp),
			"%s\\%s", dname_tmp, fname_tmp);
	}

	if (WFR_SUCCESS(status)
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(fname, strlen(fname), &fnameW))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(pname_tmp, strlen(pname_tmp), &pname_tmpW_))
	&&  WFR_SUCCESS_POSIX(status, (WFR_WCSNCPY(pname_tmpW, pname_tmpW_, wcslen(pname_tmpW_) + 1)))
	&&  WFR_SUCCESS(status = WFR_WMKTEMP_S(pname_tmpW))
	&&  WFR_SUCCESS_POSIX(status, (file = _wfopen(pname_tmpW, L"wb")))
	&&  (list_size > 0)
	&&  WFR_SUCCESS_POSIX(status, (nwritten = fwrite(list, list_size, 1, file)) == 1))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	} else if (WFR_FAILURE(status) && file) {
		if (ferror(file) < 0) {
			status = WFR_STATUS_FROM_ERRNO();
		} else {
			status = WFR_STATUS_FROM_ERRNO1(EPIPE);
		}
	}

	if (file) {
		(void)fflush(file);
		(void)fclose(file);
	}

	if (WFR_SUCCESS(status)
	&&  WFR_FAILURE_WINDOWS(status, MoveFileExW(
			pname_tmpW, fnameW,
			MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)))
	{
		(void)_wunlink(pname_tmpW);
	} else if (WFR_FAILURE(status)) {
		(void)_wunlink(pname_tmpW);
	}

	WFR_FREE_IF_NOTNULL(dname_tmp);
	WFR_FREE_IF_NOTNULL(fnameW);
	WFR_FREE_IF_NOTNULL(pname_tmpW_);

	return status;
}

WfrStatus
WfrSaveRawFile(
	bool		escape_fnamefl,
	bool		recreate_dnamefl,
	char *		dname,
	const char *	ext,
	const char *	fname,
	const char *	data,
	size_t		data_size
	)
{
	char *		dname_tmp = NULL;
	wchar_t *	dname_tmpW;
	wchar_t *	dnameW = NULL;
	FILE *		file = NULL;
	size_t		nwritten;
	char		pname[MAX_PATH + 1];
	char		pname_tmp[MAX_PATH + 1];
	wchar_t		pname_tmpW[MAX_PATH + 1];
	wchar_t *	pname_tmpW_ = NULL;
	wchar_t *	pnameW = NULL;
	struct _stat64	statbuf;
	WfrStatus	status;


	if (WFR_FAILURE(status = WfrpSaveGetCreateTempRootDirectory(&dname_tmpW))) {
		return status;
	}

	if (WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(dname, strlen(dname), &dnameW))
	&&  WFR_FAILURE_POSIX(status, (_wstat64(dnameW, &statbuf) == 0)))
	{
		if (WFR_STATUS_IS_NOT_FOUND(status)) {
			if (recreate_dnamefl) {
				status = WfrMakeDirectory(dname, true);
			}
		}
		if (WFR_FAILURE(status)) {
			return status;
		}
	}

	if (WFR_SUCCESS(status = WfrConvertUtf16ToUtf8String(dname_tmpW, wcslen(dname_tmpW), &dname_tmp))) {
		if (escape_fnamefl) {
			if (WFR_SUCCESS(status = WfrEscapeFileName(
					dname, ext, fname, false,
					pname, sizeof(pname)))
			&&  WFR_SUCCESS(status = WfrEscapeFileName(
					dname_tmp, ext, fname, true,
					pname_tmp, sizeof(pname_tmp))))
			{
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		} else {
			WFR_SNPRINTF_PNAME(pname, sizeof(pname), dname, ext, fname);
			WFR_SNPRINTF_PNAME_TMP(pname_tmp, sizeof(pname_tmp), dname_tmp, ext, fname);
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	if (WFR_SUCCESS(status)) {
		if (WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(pname, strlen(pname), &pnameW))
		&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(pname_tmp, strlen(pname_tmp), &pname_tmpW_))
		&&  WFR_SUCCESS_POSIX(status, (WFR_WCSNCPY(pname_tmpW, pname_tmpW_, wcslen(pname_tmpW_) + 1)))
		&&  WFR_SUCCESS(status = WFR_WMKTEMP_S(pname_tmpW))
		&&  WFR_SUCCESS_POSIX(status, (file = _wfopen(pname_tmpW, L"wb")))
		&&  (data_size > 0)
		&&  WFR_SUCCESS_POSIX(status, ((nwritten = fwrite(data, data_size, 1, file)) == 1)))
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else if (WFR_FAILURE(status) && file)
		{
			if (ferror(file) < 0) {
				status = WFR_STATUS_FROM_ERRNO();
			} else {
				status = WFR_STATUS_FROM_ERRNO1(EPIPE);
			}
		}

		if (file) {
			(void)fflush(file);
			(void)fclose(file);
		}
	}

	if (WFR_SUCCESS(status)
	&&  WFR_FAILURE_WINDOWS(status, MoveFileExW(
			pname_tmpW, pnameW,
			MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)))
	{
		(void)_wunlink(pname_tmpW);
	} else if (WFR_FAILURE(status)) {
		(void)_wunlink(pname_tmpW);
	}

	WFR_FREE_IF_NOTNULL(dname_tmp);
	WFR_FREE_IF_NOTNULL(dnameW);
	WFR_FREE_IF_NOTNULL(pname_tmpW_);
	WFR_FREE_IF_NOTNULL(pnameW);

	return status;
}

WfrStatus
WfrSaveToFileV(
	bool		escape_fnamefl,
	char *		dname,
	const char *	ext,
	const char *	fname,
			...
	)
{
	va_list			ap;
	char *			dname_tmp = NULL;
	wchar_t *		dname_tmpW;
	wchar_t *		dnameW = NULL;
	FILE *			file = NULL;
	char			fname_full[MAX_PATH + 1];
	wchar_t *		fname_fullW = NULL;
	char			fname_tmp[MAX_PATH + 1];
	wchar_t			fname_tmpW[MAX_PATH + 1];
	wchar_t *		fname_tmpW_ = NULL;
	const char *		key;
	int			rc;
	struct _stat64		statbuf;
	WfrStatus		status;
	WfrTreeItemTypeBase	type;
	const char *		value;


	if (WFR_FAILURE(status = WfrpSaveGetCreateTempRootDirectory(&dname_tmpW))) {
		return status;
	}

	if (dname
	&&  (WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(dname, strlen(dname), &dnameW))
	&&   WFR_FAILURE_POSIX(status, (_wstat64(dnameW, &statbuf) == 0))))
	{
		if (WFR_STATUS_IS_NOT_FOUND(status)) {
			status = WfrMakeDirectory(dname, true);
		}
		if (WFR_FAILURE(status)) {
			return status;
		}
	}

	if (WFR_SUCCESS(status = WfrConvertUtf16ToUtf8String(dname_tmpW, wcslen(dname_tmpW), &dname_tmp))) {
		if (escape_fnamefl) {
			if (WFR_FAILURE(status = WfrEscapeFileName(
					dname, ext, fname, false, fname_full, sizeof(fname_full)))
			||  WFR_FAILURE(status = WfrEscapeFileName(
					dname_tmp, ext, fname, true, fname_tmp, sizeof(fname_tmp))))
			{
				return status;
			}
		} else {
			WFR_SNPRINTF_PNAME(fname_full, sizeof(fname_full), dname, ext, fname);
			WFR_SNPRINTF_PNAME_TMP(fname_tmp, sizeof(fname_tmp), dname_tmp, ext, fname);
		}
	}

	if (WFR_SUCCESS(status)) {
		if (WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(fname_full, strlen(fname_full), &fname_fullW))
		&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(fname_tmp, strlen(fname_tmp), &fname_tmpW_))
		&&  WFR_SUCCESS_POSIX(status, (WFR_WCSNCPY(fname_tmpW, fname_tmpW_, wcslen(fname_tmpW_) + 1)))
		&&  WFR_SUCCESS(status = WFR_WMKTEMP_S(fname_tmpW))
		&&  WFR_SUCCESS_POSIX(status, (file = _wfopen(fname_tmpW, L"wb"))))
		{
			va_start(ap, fname);
			while (WFR_SUCCESS(status)) {
				if (!(key = va_arg(ap, const char *))) {
					break;
				} else {
					type = va_arg(ap, int);
					value = va_arg(ap, void *);
				}

				switch (type) {
				default:
					status = WFR_STATUS_FROM_ERRNO1(EINVAL);
					break;

				case WFR_TREE_ITYPE_INT:
					WFR_STATUS_BIND_POSIX(status,
						(rc = fprintf(file, "%s=int:%d\r\n", key, *(int *)value)) >= 0);
					break;

				case WFR_TREE_ITYPE_STRING:
					WFR_STATUS_BIND_POSIX(status,
						(rc = fprintf(file, "%s=string:%s\r\n", key, (const char *)value)) >= 0);
					break;
				}
			}
			va_end(ap);
		}
	}

	if (file) {
		(void)fflush(file);
		(void)fclose(file);
	}

	if (WFR_SUCCESS(status)
	&&  WFR_FAILURE_WINDOWS(status, MoveFileExW(
			fname_tmpW, fname_fullW,
			MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)))
	{
		(void)_wunlink(fname_tmpW);
	} else if (WFR_FAILURE(status)) {
		(void)_wunlink(fname_tmpW);
	}

	WFR_FREE_IF_NOTNULL(dname_tmp);
	WFR_FREE_IF_NOTNULL(dnameW);
	WFR_FREE_IF_NOTNULL(fname_fullW);
	WFR_FREE_IF_NOTNULL(fname_tmpW_);

	return status;
}

WfrStatus
WfrSaveToRegSubKeyV(
	const char *	key_name,
	const char *	subkey,
			...
	)
{
	va_list		ap;
	HKEY		hKey;
	const char *	key;
	wchar_t *	keyW = NULL;
	WfrStatus	status;
	char *		subkey_escaped = NULL;
	wchar_t *	subkey_escapedW = NULL;
	int		type;
	const char *	value;
	size_t		value_len;
	wchar_t *	valueW = NULL;
	size_t		valueW_size;


	if (WFR_SUCCESS(status = WfrEscapeRegKey(subkey, &subkey_escaped))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(subkey_escaped, strlen(subkey_escaped), &subkey_escapedW))
	&&  WFR_SUCCESS(status = WfrOpenRegKeyRo(
			HKEY_CURRENT_USER, &hKey,
			key_name, subkey_escaped)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;

		va_start(ap, subkey);
		while (WFR_SUCCESS(status)) {
			if (!(key = va_arg(ap, const char *))) {
				break;
			} else {
				type = va_arg(ap, int);
				value = va_arg(ap, void *);
			}

			switch (type) {
			default:
				status = WFR_STATUS_FROM_ERRNO1(EINVAL);
				break;

			case WFR_TREE_ITYPE_INT:
				status = WfrConvertUtf8ToUtf16String(key, strlen(key), &keyW);
				if (WFR_SUCCESS(status)) {
					WFR_STATUS_BIND_LSTATUS(status, RegSetValueExW(
						hKey, keyW, 0, REG_DWORD,
						(const BYTE *)value, sizeof(int)));
				}
				WFR_FREE_IF_NOTNULL(keyW);
				break;

			case WFR_TREE_ITYPE_STRING:
				value_len = strlen(value);
				if (WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(key, strlen(key), &keyW))
				&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String1(value, value_len, &valueW, &valueW_size)))
				{
					WFR_STATUS_BIND_LSTATUS(status, RegSetValueExW(
						hKey, keyW, 0, REG_SZ,
						(const BYTE *)valueW,
						valueW_size * sizeof(wchar_t)));
				}
				WFR_FREE_IF_NOTNULL(keyW);
				WFR_FREE_IF_NOTNULL(valueW);
				break;
			}
		}
		va_end(ap);
	}

	if (hKey != NULL) {
		(void)RegCloseKey(hKey);
	}
	if (WFR_FAILURE(status) && subkey_escapedW) {
		(void)RegDeleteTreeW(hKey, subkey_escapedW);
	}

	WFR_FREE_IF_NOTNULL(subkey_escaped);
	WFR_FREE_IF_NOTNULL(subkey_escapedW);

	return status;
}

WfrStatus
WfrSaveTreeToFile(
	bool		escape_fnamefl,
	char *		dname,
	const char *	ext,
	const char *	fname,
	WfrTree	*	tree
	)
{
	char *		dname_tmp = NULL;
	wchar_t *	dname_tmpW;
	wchar_t *	dnameW = NULL;
	FILE *		file = NULL;
	char		fname_full[MAX_PATH + 1];
	wchar_t *	fname_fullW = NULL;
	char		fname_tmp[MAX_PATH + 1];
	wchar_t		fname_tmpW[MAX_PATH + 1];
	wchar_t *	fname_tmpW_ = NULL;
	WfrTreeItem *	item;
	int		rc;
	struct _stat64	statbuf;
	WfrStatus	status;


	if (WFR_FAILURE(status = WfrpSaveGetCreateTempRootDirectory(&dname_tmpW))) {
		return status;
	}

	if (dname
	&&  (WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(dname, strlen(dname), &dnameW))
	&&   WFR_FAILURE_POSIX(status, (_wstat64(dnameW, &statbuf) == 0))))
	{
		if (WFR_STATUS_IS_NOT_FOUND(status)) {
			status = WfrMakeDirectory(dname, true);
		}
		if (WFR_FAILURE(status)) {
			return status;
		}
	}

	if (WFR_SUCCESS(status = WfrConvertUtf16ToUtf8String(dname_tmpW, wcslen(dname_tmpW), &dname_tmp))) {
		if (escape_fnamefl) {
			if (WFR_FAILURE(status = WfrEscapeFileName(
					dname, ext, fname, false, fname_full, sizeof(fname_full)))
			||  WFR_FAILURE(status = WfrEscapeFileName(
					dname_tmp, ext, fname, true, fname_tmp, sizeof(fname_tmp))))
			{
				return status;
			}
		} else {
			WFR_SNPRINTF_PNAME(fname_full, sizeof(fname_full), dname, ext, fname);
			WFR_SNPRINTF_PNAME_TMP(fname_tmp, sizeof(fname_tmp), dname_tmp, ext, fname);
		}
	}

	if (WFR_SUCCESS(status)
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(fname_full, strlen(fname_full), &fname_fullW))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(fname_tmp, strlen(fname_tmp), &fname_tmpW_))
	&&  WFR_SUCCESS_POSIX(status, (WFR_WCSNCPY(fname_tmpW, fname_tmpW_, wcslen(fname_tmpW_) + 1)))
	&&  WFR_SUCCESS(status = WFR_WMKTEMP_S(fname_tmpW))
	&&  WFR_SUCCESS_POSIX(status, (file = _wfopen(fname_tmpW, L"wb"))))
	{
		WFR_TREE234_FOREACH(status, tree, idx, item) {
			switch (item->type) {
			default:
				rc = 0; break;

			case WFR_TREE_ITYPE_INT:
				rc = fprintf(
					file, "%s=int:%d\r\n",
					(char *)item->key, *((int *)item->value));
				break;

			case WFR_TREE_ITYPE_STRING:
				rc = fprintf(
					file, "%s=string:%s\r\n",
					(char *)item->key, (char *)item->value);
				break;
			}

			if (rc < 0) {
				status = WFR_STATUS_FROM_ERRNO1(rc);
				break;
			}
		}
	}

	if (file) {
		(void)fflush(file);
		(void)fclose(file);
	}

	if (WFR_SUCCESS(status)
	&&  WFR_FAILURE_WINDOWS(status, MoveFileExW(
			fname_tmpW, fname_fullW,
			MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)))
	{
		(void)_wunlink(fname_tmpW);
	} else if (WFR_FAILURE(status)) {
		(void)_wunlink(fname_tmpW);
	}

	WFR_FREE_IF_NOTNULL(dname_tmp);
	WFR_FREE_IF_NOTNULL(dnameW);
	WFR_FREE_IF_NOTNULL(fname_fullW);
	WFR_FREE_IF_NOTNULL(fname_tmpW_);

	return status;
}

WfrStatus
WfrSaveTreeToRegSubKey(
	const char *	key_name,
	const char *	subkey,
	WfrTree	*	tree
	)
{
	HKEY		hKey;
	WfrTreeItem *	item;
	wchar_t *	keyW = NULL;
	char *		subkey_escaped = NULL;
	wchar_t *	subkey_escapedW = NULL;
	WfrStatus	status;
	wchar_t *	valueW = NULL;
	size_t		valueW_size;


	if (WFR_SUCCESS(status = WfrEscapeRegKey(subkey, &subkey_escaped))
	&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(subkey_escaped, strlen(subkey_escaped), &subkey_escapedW)))
	{
		(void)RegDeleteTreeW(hKey, subkey_escapedW);
		if (WFR_SUCCESS(status = WfrCreateRegKey(
				HKEY_CURRENT_USER, &hKey,
				key_name, subkey_escaped)))
		{
			WFR_TREE234_FOREACH(status, tree, idx, item) {
				switch (item->type) {
				default:
					status = WFR_STATUS_FROM_ERRNO1(EINVAL);
					break;

				case WFR_TREE_ITYPE_INT:
					status = WfrConvertUtf8ToUtf16String(item->key, strlen(item->key), &keyW);
					if (WFR_SUCCESS(status)) {
						WFR_STATUS_BIND_LSTATUS(status, RegSetValueExW(
							hKey, keyW, 0, REG_DWORD,
							(const BYTE *)item->value, item->value_size));
					}
					WFR_FREE_IF_NOTNULL(keyW);
					break;

				case WFR_TREE_ITYPE_STRING:
					if (WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(item->key, strlen(item->key), &keyW))
					&&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String1(item->value, item->value_size, &valueW, &valueW_size)))
					{
						WFR_STATUS_BIND_LSTATUS(status, RegSetValueExW(
							hKey, keyW, 0, REG_SZ,
							(const BYTE *)valueW,
							valueW_size * sizeof(wchar_t)));
					}
					WFR_FREE_IF_NOTNULL(keyW);
					WFR_FREE_IF_NOTNULL(valueW);
					break;
				}
			}
		}

		if (WFR_FAILURE(status)) {
			(void)RegDeleteTreeW(hKey, subkey_escapedW);
		}
	}

	(void)RegCloseKey(hKey);
	WFR_FREE_IF_NOTNULL(keyW);
	WFR_FREE_IF_NOTNULL(subkey_escapedW);
	WFR_FREE_IF_NOTNULL(valueW);

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
