/*
 * winfrip_rtl.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdarg.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#pragma GCC diagnostic pop

#include <windows.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_pcre2.h"

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

WfrStatus
WfrDeleteDirectory(
	const char *	path,
	bool		noentfl,
	bool		recursefl
	)
{
	struct dirent *		dire;
	DIR *			dirp = NULL;
	char			path_cwd[MAX_PATH + 1];
	struct stat		statbuf;
	WfrStatus		status;


	if (rmdir(path) < 0) {
		status = WFR_STATUS_FROM_ERRNO();
		if (WFR_STATUS_CONDITION(status) == ENOENT) {
			if (noentfl) {
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		} else if (((WFR_STATUS_CONDITION(status) == EEXIST)
			||  (WFR_STATUS_CONDITION(status) == ENOTEMPTY))
			&&  recursefl)
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
			if (!(dirp = opendir(path))) {
				status = WFR_STATUS_FROM_ERRNO();
			} else if (!getcwd(path_cwd, sizeof(path_cwd))) {
				status = WFR_STATUS_FROM_ERRNO();
			} else if (chdir(path) < 0) {
				status = WFR_STATUS_FROM_ERRNO();
			} else {
				while (WFR_STATUS_SUCCESS(status) && (dire = readdir(dirp))) {
					if ((dire->d_name[0] == '.')
					&&  (dire->d_name[1] == '\0'))
					{
						continue;
					} else if ((dire->d_name[0] == '.')
						&& (dire->d_name[1] == '.')
						&& (dire->d_name[2] == '\0'))
					{
						continue;
					} else if (stat(dire->d_name, &statbuf) < 0) {
						status = WFR_STATUS_FROM_ERRNO();
					} else if (statbuf.st_mode & S_IFDIR) {
						status = WfrDeleteDirectory(dire->d_name, noentfl, recursefl);
					} else if (unlink(dire->d_name) < 0) {
						status = WFR_STATUS_FROM_ERRNO();
					}
				}
			}

			(void)closedir(dirp);
			if (chdir(path_cwd) < 0) {
				status = WFR_STATUS_FROM_ERRNO();
			}

			status = WfrDeleteDirectory(path, noentfl, false);
		}
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfrEnumRegKey(
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

WfrStatus
WfrMakeDirectory(
	char *	path,
	bool	existsfl
	)
{
	bool		lastfl;
	char *		p, *path_sub, sep;
	char *		path_absdrive;
	char		path_cwd[MAX_PATH + 1];
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	if (!getcwd(path_cwd, sizeof(path_cwd))) {
		status = WFR_STATUS_FROM_ERRNO();
	}

	if ((((path[0] >= 'a') && (path[0] <= 'z'))
	||   ((path[0] >= 'A') && (path[0] <= 'Z')))
	&&    (path[1] == ':')
	&&   ((path[2] == '/') || (path[2] == '\\')))
	{
		path_absdrive = path; path += 3;
	} else {
		path_absdrive = NULL;
	}

	for (p = path, path_sub = path;
	     WFR_STATUS_SUCCESS(status) && *p; p++)
	{
		if ((p[0] == '/')
		||  (p[0] == '\\')
		||  (p[1] == '\0'))
		{
			if ((p[0] == '/') || (p[0] == '\\')) {
				lastfl = false; sep = *p; *p = '\0';
			} else {
				lastfl = true;
			}

			while ((*path_sub == '/')
			    || (*path_sub == '\\'))
			{
				path_sub++;
			}
			if (path_sub[0] == '\0') {
				goto next;
			} else if ((path_sub[0] == '.') && (path_sub[1] == '\0') && !path_absdrive) {
				goto next;
			} else if ((path_sub[0] == '.') && (path_sub[1] == '.') && (path_sub[2] == '\0')) {
				goto change_dir;
			}

			if (mkdir(path_absdrive ? path_absdrive : path_sub) < 0) {
				status = WFR_STATUS_FROM_ERRNO();
				if ((WFR_STATUS_CONDITION(status) == EEXIST)
				&&  existsfl)
				{
					status = WFR_STATUS_CONDITION_SUCCESS;
				}
			}

		change_dir:
			if (!lastfl && WFR_STATUS_SUCCESS(status)) {
				if (chdir(path_absdrive ? path_absdrive : path_sub) < 0) {
					status = WFR_STATUS_FROM_ERRNO();
				}
			}

		next:
			if (path_absdrive) {
				path_absdrive = NULL;
			}

			if (!lastfl) {
				*p = sep; path_sub = ++p;
			}
		}
	}

	(void)chdir(path_cwd);

	return status;
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

	return MessageBox(NULL, msg_buf, lpCaption, uType);
}

WfrStatus
WfrOpenRegKey(
	HKEY		hKey,
	bool		createfl,
	bool		writefl,
	HKEY *		phKey,
	const char *	path,
			...
	)
{
	va_list		ap;
	bool		closefl;
	HKEY		hKey_new;
	REGSAM		samDesired = KEY_READ | (writefl ? KEY_WRITE : 0);
	WfrStatus	status;
	LONG		status_registry;


	va_start(ap, path);
	for (closefl = false; path; path = va_arg(ap, const char *)) {
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
	va_end(ap);

	if (status_registry == ERROR_SUCCESS) {
		*phKey = hKey;
		status = WFR_STATUS_CONDITION_SUCCESS;
	} else {
		status = WFR_STATUS_FROM_WINDOWS1(status_registry);
	}

	return status;
}

WfrStatus
WfrSnDuprintf(
	char **restrict		ps,
	size_t *		pn,
	const char *restrict	format,
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
	static char	condition_msg[128];
	static wchar_t	condition_msg_w[128];
	static char	error_msg[256];


	switch (WFR_STATUS_FACILITY(status)) {
	default:
		strncpy(condition_msg, "(unknown facility)", sizeof(condition_msg));
		break;

	case WFR_STATUS_FACILITY_POSIX:
		strncpy(condition_msg, strerror(status.condition), sizeof(condition_msg) - 1);
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
	}

	WFR_SNPRINTF(
		error_msg, sizeof(error_msg),
		"Error in %s:%d: %s",
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


	out_w_size = (in_w_len + 1) * sizeof(*out_w);
	out_w = WFR_NEWN(out_w_size, wchar_t);
	ZeroMemory(out_w, out_w_size);
	wcsncpy(out_w, in_w, in_w_len);
	return out_w;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
