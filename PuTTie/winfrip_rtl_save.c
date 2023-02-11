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
	char *		dname_tmp;
	int		fd = -1;
	FILE *		file = NULL;
	ssize_t		nwritten;
	char		pname_tmp[MAX_PATH + 1];
	WfrStatus	status;


	if (!(dname_tmp = getenv("TEMP"))
	||  !(dname_tmp = getenv("TMP"))) {
		dname_tmp = (char *)"./";
	}
	WFR_SNPRINTF(
		pname_tmp, sizeof(pname_tmp),
		"%s/%s", dname_tmp, fname_tmp);

	if (WFR_SUCCESS_POSIX(status, (fd = mkstemp(pname_tmp)) > 0)
	&&  WFR_SUCCESS_POSIX(status, (file = fdopen(fd, "wb")))
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
	} else if ((fd >= 0)) {
		(void)close(fd);
	}

	if (WFR_SUCCESS(status)
	&&  WFR_FAILURE_WINDOWS(status, MoveFileEx(
			pname_tmp, fname, MOVEFILE_REPLACE_EXISTING)))
	{
		(void)unlink(pname_tmp);
	} else if (WFR_FAILURE(status)) {
		(void)unlink(pname_tmp);
	}

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
	char *		dname_tmp;
	int		fd = -1;
	FILE *		file = NULL;
	size_t		nwritten;
	char		pname[MAX_PATH + 1], pname_tmp[MAX_PATH + 1];
	struct stat	statbuf;
	WfrStatus	status;


	if (WFR_FAILURE_POSIX(status, (stat(dname, &statbuf) == 0))) {
		if (WFR_STATUS_IS_NOT_FOUND(status)) {
			if (recreate_dnamefl) {
				status = WfrMakeDirectory(dname, true);
			}
		}
		if (WFR_FAILURE(status)) {
			return status;
		}
	}

	if (!(dname_tmp = getenv("TEMP"))
	||  !(dname_tmp = getenv("TMP"))) {
		dname_tmp = (char *)"./";
	}

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

	if (WFR_SUCCESS(status)) {
		if (WFR_SUCCESS_POSIX(status, (fd = mkstemp(pname_tmp)) > 0)
		&&  WFR_SUCCESS_POSIX(status, (file = fdopen(fd, "wb")))
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
		} else if ((fd >= 0)) {
			(void)close(fd);
		}
	}

	if (WFR_SUCCESS(status)
	&&  WFR_FAILURE_WINDOWS(status, MoveFileEx(
			pname_tmp, pname, MOVEFILE_REPLACE_EXISTING)))
	{
		(void)unlink(pname_tmp);
	} else if (WFR_FAILURE(status)) {
		(void)unlink(pname_tmp);
	}

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
	char *			dname_tmp;
	int			fd = -1;
	FILE *			file = NULL;
	char			fname_full[MAX_PATH];
	char			fname_tmp[MAX_PATH];
	const char *		key;
	int			rc;
	struct stat		statbuf;
	WfrStatus		status;
	WfrTreeItemTypeBase	type;
	const char *		value;


	if (dname && WFR_FAILURE_POSIX(status, (stat(dname, &statbuf) == 0))) {
		if (WFR_STATUS_IS_NOT_FOUND(status)) {
			status = WfrMakeDirectory(dname, true);
		}
		if (WFR_FAILURE(status)) {
			return status;
		}
	}

	if (!(dname_tmp = getenv("TEMP"))
	||  !(dname_tmp = getenv("TMP"))) {
		dname_tmp = "./";
	}

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

	if (WFR_SUCCESS_POSIX(status, (fd = mkstemp(fname_tmp)) > 0)
	&&  WFR_SUCCESS_POSIX(status, (file = fdopen(fd, "wb"))))
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

	if (file) {
		(void)fflush(file);
		(void)fclose(file);
	} else if ((fd >= 0)) {
		(void)close(fd);
	}

	if (WFR_SUCCESS(status)
	&&  WFR_FAILURE_WINDOWS(status, MoveFileEx(
			fname_tmp, fname_full, MOVEFILE_REPLACE_EXISTING)))
	{
		(void)unlink(fname_tmp);
	} else if (WFR_FAILURE(status)) {
		(void)unlink(fname_tmp);
	}

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
	WfrStatus	status;
	char *		subkey_escaped = NULL;
	int		type;
	const char *	value;


	if (WFR_SUCCESS(status = WfrEscapeRegKey(subkey, &subkey_escaped))
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
				WFR_SUCCESS_LSTATUS(status, RegSetValueEx(
					hKey, key, 0, REG_DWORD,
					(const BYTE *)value, sizeof(int)));
				break;

			case WFR_TREE_ITYPE_STRING:
				WFR_SUCCESS_LSTATUS(status, RegSetValueEx(
					hKey, key, 0, REG_SZ,
					(const BYTE *)value, strlen(value)));
				break;
			}
		}
		va_end(ap);
	}

	if (hKey != NULL) {
		(void)RegCloseKey(hKey);
	}
	if (WFR_FAILURE(status)) {
		(void)RegDeleteTree(hKey, subkey_escaped);
	}

	WFR_FREE_IF_NOTNULL(subkey_escaped);

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
	char *		dname_tmp;
	int		fd = -1;
	FILE *		file = NULL;
	char		fname_full[MAX_PATH];
	char		fname_tmp[MAX_PATH];
	WfrTreeItem *	item;
	int		rc;
	struct stat	statbuf;
	WfrStatus	status;


	if (dname && WFR_FAILURE_POSIX(status, (stat(dname, &statbuf) == 0))) {
		if (WFR_STATUS_IS_NOT_FOUND(status)) {
			status = WfrMakeDirectory(dname, true);
		}
		if (WFR_FAILURE(status)) {
			return status;
		}
	}

	if (!(dname_tmp = getenv("TEMP"))
	||  !(dname_tmp = getenv("TMP"))) {
		dname_tmp = "./";
	}

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

	if (WFR_SUCCESS_POSIX(status, ((fd = mkstemp(fname_tmp)) > 0))
	&&  WFR_SUCCESS_POSIX(status, (file = fdopen(fd, "wb"))))
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
	} else if ((fd >= 0)) {
		(void)close(fd);
	}

	if (WFR_SUCCESS(status)
	&&  WFR_FAILURE_WINDOWS(status, MoveFileEx(
			fname_tmp, fname_full, MOVEFILE_REPLACE_EXISTING)))
	{
		(void)unlink(fname_tmp);
	} else if (WFR_FAILURE(status)) {
		(void)unlink(fname_tmp);
	}

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
	char *		subkey_escaped = NULL;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfrEscapeRegKey(subkey, &subkey_escaped))) {
		(void)RegDeleteTree(hKey, subkey_escaped);
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
					WFR_SUCCESS_LSTATUS(status, RegSetValueEx(
						hKey, item->key, 0, REG_DWORD,
						(const BYTE *)item->value, item->value_size));
					break;

				case WFR_TREE_ITYPE_STRING:
					WFR_SUCCESS_LSTATUS(status, RegSetValueEx(
						hKey, item->key, 0, REG_SZ,
						(const BYTE *)item->value, item->value_size));
					break;
				}
			}
		}

		if (WFR_FAILURE(status)) {
			(void)RegDeleteTree(hKey, subkey_escaped);
		}
	}

	(void)RegCloseKey(hKey);
	WFR_FREE_IF_NOTNULL(subkey_escaped);

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
