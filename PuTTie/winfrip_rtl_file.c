/*
 * winfrip_rtl_file.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <windows.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_file.h"

/*
 * Private types
 */

typedef struct WfrWatchDirectoryContext {
	bool			display_errorsfl;
	char **			pdname;
	CRITICAL_SECTION *	dname_cs;
	HANDLE			dname_event;
	HWND			hwnd;
	UINT			window_msg;
} WfrWatchDirectoryContext;
#define WFR_WATCH_DIRECTORY_CONTEXT_EMPTY {					\
	.display_errorsfl = false,						\
	.pdname = NULL,								\
	.dname_cs = NULL,							\
	.dname_event = NULL,							\
	.hwnd = NULL,								\
	.window_msg = 0,							\
}
#define WFR_WATCH_DIRECTORY_CONTEXT_INIT(ctx) ({				\
	(ctx) = (WfrWatchDirectoryContext)WFR_WATCH_DIRECTORY_CONTEXT_EMPTY;	\
	WFR_STATUS_CONDITION_SUCCESS;						\
})
#define WFR_WATCH_DIRECTORY_CONTEXT_INIT1(ctx, display_errorsfl_, pdname_,	\
					  dname_cs_, dname_event_, hwnd_,	\
					  window_msg_) ({			\
	(ctx) = (WfrWatchDirectoryContext)WFR_WATCH_DIRECTORY_CONTEXT_EMPTY;	\
	(ctx).display_errorsfl = (display_errorsfl_);				\
	(ctx).pdname = (pdname_);						\
	(ctx).dname_cs = (dname_cs_);						\
	(ctx).dname_event = (dname_event_);					\
	(ctx).hwnd = (hwnd_);							\
	(ctx).window_msg = (window_msg_);					\
	WFR_STATUS_CONDITION_SUCCESS;						\
})

/*
 * Private subroutines
 */

static DWORD WINAPI
WfrpWatchDirectoryThreadProc(
	LPVOID	lpParameter
	)
{
	HANDLE				dwChangeHandle = NULL;
	HANDLE				dwHandles[2];
	HANDLE				hDummyEvent;
	WfrStatus			status;
	WfrWatchDirectoryContext *	ctx;


	ctx = (WfrWatchDirectoryContext *)lpParameter;

	if (!(hDummyEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
		status = WFR_STATUS_FROM_WINDOWS();
		if (ctx->display_errorsfl) {
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "creating event");
		}
		return FALSE;
	} else {
		dwHandles[0] = ctx->dname_event;
		dwHandles[1] = hDummyEvent;
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	do {
		switch (WaitForMultipleObjects(2, dwHandles, FALSE, INFINITE)) {
		case WAIT_ABANDONED_0:
		case WAIT_ABANDONED_0 + 1:
		case WAIT_TIMEOUT:
			break;

		case WAIT_FAILED:
		default:
			status = WFR_STATUS_FROM_WINDOWS();
			break;

		case WAIT_OBJECT_0:
			EnterCriticalSection(ctx->dname_cs);

			if (dwChangeHandle != NULL) {
				(void)FindCloseChangeNotification(dwChangeHandle);
				dwChangeHandle = NULL;
			}

			if (*(ctx->pdname)) {
				dwChangeHandle = FindFirstChangeNotification(
					*(ctx->pdname), FALSE, FILE_NOTIFY_CHANGE_FILE_NAME);
				if ((dwChangeHandle == INVALID_HANDLE_VALUE)
				||  (dwChangeHandle == NULL))
				{
					if (ctx->display_errorsfl) {
						status = WFR_STATUS_FROM_WINDOWS();
						WFR_IF_STATUS_FAILURE_MESSAGEBOX(
							status, "finding first change notification for %s", *(ctx->pdname));
					}
					status = WFR_STATUS_CONDITION_SUCCESS;
				} else {
					dwHandles[1] = dwChangeHandle;
					status = WFR_STATUS_CONDITION_SUCCESS;
				}
			}

			LeaveCriticalSection(ctx->dname_cs);
			break;

		case WAIT_OBJECT_0 + 1:
			if (dwHandles[1] != hDummyEvent) {
				EnterCriticalSection(ctx->dname_cs);
				if (FindNextChangeNotification(dwChangeHandle) == FALSE) {
					if (ctx->display_errorsfl) {
						status = WFR_STATUS_FROM_WINDOWS();
						WFR_IF_STATUS_FAILURE_MESSAGEBOX(status,
							"finding next change notification", *(ctx->pdname));
						status = WFR_STATUS_CONDITION_SUCCESS;
					}

					(void)FindCloseChangeNotification(dwChangeHandle);
					dwChangeHandle = NULL;
					dwHandles[1] = hDummyEvent;
				}
				LeaveCriticalSection(ctx->dname_cs);

				(void)SendMessage(ctx->hwnd, ctx->window_msg, 0, 0);
			}
			break;
		}
	} while (WFR_SUCCESS(status));

	return (WFR_SUCCESS(status) ? TRUE : FALSE);
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

WfrStatus
WfrDeleteDirectory(
	const char *	path,
	bool		continue_on_errorfl,
	bool		noentfl,
	bool		recursefl
	)
{
	struct dirent *		dire;
	DIR *			dirp = NULL;
	char			path_cwd[PATH_MAX + 1];
	struct stat		statbuf;
	WfrStatus		status;


	if (!WFR_SUCCESS_POSIX(status, (rmdir(path) == 0))
	&&  WFR_STATUS_IS_NOT_FOUND(status))
	{
		if (noentfl) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	} else if (WFR_FAILURE(status)
		&& (((WFR_STATUS_CONDITION(status) == EEXIST)
		||   (WFR_STATUS_CONDITION(status) == ENOTEMPTY))
		&&   recursefl))
	{
		if (WFR_SUCCESS_POSIX(status, (dirp = opendir(path)))
		&&  WFR_SUCCESS_POSIX(status, (getcwd(path_cwd, sizeof(path_cwd))))
		&&  WFR_SUCCESS_POSIX(status, (chdir(path) == 0)))
		{
			errno = 0;
			while (WFR_SUCCESS(status) && (dire = readdir(dirp))) {
				if ((dire->d_name[0] == '.')
				&&  (dire->d_name[1] == '\0'))
				{
					goto next;
				} else if ((dire->d_name[0] == '.')
					&& (dire->d_name[1] == '.')
					&& (dire->d_name[2] == '\0'))
				{
					goto next;
				} else if (!WFR_SUCCESS_POSIX(status,
					(stat(dire->d_name, &statbuf) == 0)))
				{
					if (continue_on_errorfl) {
						status = WFR_STATUS_CONDITION_SUCCESS;
					}
				} else if (statbuf.st_mode & S_IFDIR) {
					status = WfrDeleteDirectory(
						dire->d_name, continue_on_errorfl,
						noentfl, recursefl);
				} else if (!WFR_SUCCESS_POSIX(status,
					(unlink(dire->d_name) == 0)))
				{
					if (continue_on_errorfl) {
						status = WFR_STATUS_CONDITION_SUCCESS;
					}
				}

			next:
				errno = 0;
			}

			if (WFR_SUCCESS(status)) {
				status = ((errno == 0) ? status : WFR_STATUS_FROM_ERRNO());
			}
		}

		(void)closedir(dirp);
		WFR_SUCCESS_POSIX(status, (chdir(path_cwd) == 0));
		status = WfrDeleteDirectory(
			path, continue_on_errorfl,
			noentfl, false);
	}

	return status;
}

WfrStatus
WfrDeleteFile(
	bool		escape_fnamefl,
	const char *	dname,
	const char *	ext,
	const char *	fname
	)
{
	char		pname[MAX_PATH + 1];
	struct stat	statbuf;
	WfrStatus	status;


	if (escape_fnamefl) {
		if (WFR_FAILURE(status = WfrEscapeFileName(
				dname, ext, fname, false,
				pname, sizeof(pname))))
		{
			return status;
		}
	} else {
		WFR_SNPRINTF_PNAME(pname, sizeof(pname), dname, ext, fname);
	}

	if (WFR_SUCCESS_POSIX(status, (stat(pname, &statbuf) == 0))
	&&  WFR_SUCCESS_POSIX(status, (unlink(pname) == 0)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfrDeleteFileW(
	const wchar_t *		dname,
	const wchar_t *		fname
	)
{
	wchar_t		pname[MAX_PATH + 1];
	struct _stat64	statbuf;
	WfrStatus	status;


	WFR_SNPRINTF_PNAMEW(pname, WFR_SIZEOF_WSTRING(pname), dname, fname);

	if (WFR_SUCCESS_POSIX(status, (_wstat64(pname, &statbuf) == 0))
	&&  WFR_SUCCESS_POSIX(status, (_wunlink(pname) == 0)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfrDeleteFiles(
	const char *	dname,
	const char *	ext
	)
{
	struct dirent *		dire;
	DIR *			dirp = NULL;
	size_t			ext_len;
	char			fname[PATH_MAX + 1];
	char			path_cwd[PATH_MAX + 1];
	char *			pext;
	struct stat		statbuf;
	WfrStatus		status;


	ext_len = strlen(ext);

	if (!WFR_SUCCESS_POSIX(status, (stat(dname, &statbuf) == 0))) {
		if (WFR_STATUS_IS_NOT_FOUND(status)) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
		return status;
	}

	if (WFR_SUCCESS_POSIX(status, (dirp = opendir(dname)))
	&&  WFR_SUCCESS_POSIX(status, (getcwd(path_cwd, sizeof(path_cwd)))))
	{
		errno = 0;
		while (WFR_SUCCESS(status) && (dire = readdir(dirp))) {
			if (!WFR_SUCCESS_POSIX(status, (chdir(dname) == 0))
			||  !WFR_SUCCESS_POSIX(status, (stat(dire->d_name, &statbuf) == 0)))
			{
				goto next;
			} else if (!(statbuf.st_mode & S_IFREG)) {
				goto next;
			} else if ((dire->d_name[0] == '.')
				&& (dire->d_name[1] == '\0'))
			{
				goto next;
			} else if ((dire->d_name[0] == '.')
				&& (dire->d_name[1] == '.')
				&& (dire->d_name[2] == '\0'))
			{
				goto next;
			} else if ((pext = strstr(dire->d_name, ext))
				&& (pext[ext_len] == '\0'))
			{
				WFR_SNPRINTF(fname, sizeof(fname), "%s/%s", dname, dire->d_name);
				WFR_SUCCESS_POSIX(status, (unlink(fname) == 0));
			}

		next:
			(void)chdir(path_cwd);
			errno = 0;
		}

		if (WFR_SUCCESS(status)) {
			status = ((errno == 0) ? status : WFR_STATUS_FROM_ERRNO());
		}
	}

	if (dirp) {
		(void)closedir(dirp);
	}

	return status;
}

WfrStatus
WfrEnumerateFiles(
	const char *			ext,
	bool *				pdonefl,
	const char **			pfname,
	WfrEnumerateFilesState **	pstate
	)
{
	size_t		ext_len = 0;
	char *		fname, *fname_ext;
	char		path_cwd[PATH_MAX + 1];
	struct stat	statbuf;
	WfrStatus	status;


	if (!(*pstate)->dirp) {
		*pdonefl = true;
		*pfname = NULL;
		WfrEnumerateFilesCancel(pstate);
		return WFR_STATUS_CONDITION_SUCCESS;
	}

	if (!WFR_SUCCESS_POSIX(status,
		(getcwd(path_cwd, sizeof(path_cwd)))))
	{
		return status;
	}

	if (ext) {
		ext_len = strlen(ext);
	}

	errno = 0;
	status = WFR_STATUS_CONDITION_SUCCESS;

	while (WFR_SUCCESS(status)
	&&     ((*pstate)->dire = readdir((*pstate)->dirp)))
	{
		if (!WFR_SUCCESS_POSIX(status, (chdir((*pstate)->path) == 0))
		||  !WFR_SUCCESS_POSIX(status, (stat((*pstate)->dire->d_name, &statbuf) == 0)))
		{
			goto next;
		} else if (!(statbuf.st_mode & S_IFREG)) {
			goto next;
		} else if (((*pstate)->dire->d_name[0] == '.')
			&& ((*pstate)->dire->d_name[1] == '\0'))
		{
			goto next;
		} else if (((*pstate)->dire->d_name[0] == '.')
			&& ((*pstate)->dire->d_name[1] == '.')
			&& ((*pstate)->dire->d_name[2] == '\0'))
		{
			goto next;
		} else {
			fname = (*pstate)->dire->d_name;
			fname_ext = NULL;

			if (!ext
			||  (ext
			&&   ((fname_ext = strstr(fname, ext))
			&&    (fname_ext[ext_len] == '\0'))))
			{
				if (fname_ext) {
					*fname_ext = '\0';
				}
				*pdonefl = false;
				*pfname = fname;
				(*pstate)->donefl = false;
				(void)chdir(path_cwd);
				return WFR_STATUS_CONDITION_SUCCESS;
			}
		}

	next:
		(void)chdir(path_cwd);
		errno = 0;
	}

	if (WFR_SUCCESS(status)) {
		status = ((errno == 0) ? status : WFR_STATUS_FROM_ERRNO());
		*pdonefl = true;
		*pfname = NULL;
		WfrEnumerateFilesCancel(pstate);
	}

	return status;
}

void
WfrEnumerateFilesCancel(
	WfrEnumerateFilesState **	pstate
	)
{
	if (*pstate) {
		if ((*pstate)->dirp) {
			(void)closedir((*pstate)->dirp);
		}
		WFR_ENUMERATE_FILES_STATE_INIT(**pstate);
		WFR_FREE(*pstate); *pstate = NULL;
	}
}

WfrStatus
WfrEnumerateFilesInit(
	const char *			dname,
	WfrEnumerateFilesState **	pstate
	)
{
	struct stat	statbuf;
	WfrStatus	status;


	if (WFR_SUCCESS_POSIX(status, ((*pstate) = WFR_NEW(WfrEnumerateFilesState)))
	&&  WFR_SUCCESS(status = WFR_ENUMERATE_FILES_STATE_INIT(**pstate))
	&&  WFR_SUCCESS_POSIX(status, ((*pstate)->path = strdup(dname)))
	&&  (WFR_SUCCESS_POSIX(status, (stat(dname, &statbuf) == 0))
	||   WFR_STATUS_IS_NOT_FOUND(status))
	&&  (WFR_STATUS_IS_NOT_FOUND(status) ? true
	    : WFR_SUCCESS_POSIX(status, ((*pstate)->dirp = opendir(dname)))))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_FAILURE(status)) {
		WFR_FREE_IF_NOTNULL((*pstate)->path);
		WFR_FREE_IF_NOTNULL(*pstate);
	}

	return status;
}

WfrStatus
WfrEnumerateFilesV(
	const char *	dname,
	const char *	ext,
	size_t *	pfilec,
	char ***	pfilev
	)
{
	bool				donefl;
	size_t				filec_new = 0;
	char **				filev_new = NULL;
	const char *			fname;
	char *				fname_new;
	WfrEnumerateFilesState *	state;
	WfrStatus			status;


	if (WFR_SUCCESS(status = WfrEnumerateFilesInit(dname, &state))) {
		while (WFR_SUCCESS(status = WfrEnumerateFiles(
				ext, &donefl, &fname, &state)) && !donefl)
		{
			if (WFR_FAILURE(status = WFR_RESIZE(
					filev_new, filec_new,
					filec_new + 1, char *))
			||  !WFR_SUCCESS_POSIX(status, (fname_new = strdup(fname))))
			{
				(void)WfrEnumerateFilesCancel(&state);
				break;
			} else {
				filev_new[filec_new - 1] = fname_new;
			}
		}
	}

	if (WFR_SUCCESS(status)) {
		*pfilec = filec_new;
		*pfilev = filev_new;
	} else {
		WFR_FREE_VECTOR_IF_NOTNULL(filec_new, filev_new);
	}

	return status;
}

WfrStatus
WfrEscapeFileName(
	const char *	dname,
	const char *	ext,
	const char *	name,
	bool		tmpfl,
	char *		fname,
	size_t		fname_size
	)
{
	bool		catfl;
	char *		name_escaped = NULL;
	size_t		name_escaped_len, name_escaped_size;
	char		*p;
	WfrStatus	status;


	name_escaped_size = strlen(name) + 1;
	if (WFR_SUCCESS_POSIX(status, (name_escaped = WFR_NEWN(name_escaped_size, char)))) {
		memset(name_escaped, 0, name_escaped_size);

		do {
			for (catfl = false, p = (char *)name; *p; p++) {
				if ((*p == '"') || (*p == '*') || (*p == ':')
				||  (*p == '<') || (*p == '>') || (*p == '?')
				||  (*p == '|') || (*p == '/') || (*p == '\\'))
				{
					if (WFR_SUCCESS(status = WFR_RESIZE(
							name_escaped, name_escaped_size,
							name_escaped_size + (sizeof("%00") - 1),
							char)))
					{
						strncat(name_escaped, name, p - name);
						name_escaped_len = strlen(name_escaped);
						WFR_SNPRINTF(
							&name_escaped[name_escaped_len],
							name_escaped_size - name_escaped_len,
							"%%%02x", (int)*p);
						name = ++p; catfl = true;
						break;
					}
				}
			}

			if (!catfl) {
				strcat(name_escaped, name);
				break;
			}
		} while (WFR_SUCCESS(status));

		if (WFR_SUCCESS(status)) {
			WFR_SNPRINTF(
				fname, fname_size,
				"%s%s%s%s%s",
				(dname ? dname : ""),
				(dname ? "/" : ""),
				name_escaped,
				(ext ? ext : ""),
				(tmpfl ? ".XXXXXX" : ""));
		}
	}

	WFR_FREE_IF_NOTNULL(name_escaped);

	return status;
}

WfrStatus
WfrMakeDirectory(
	char *	path,
	bool	existsfl
	)
{
	bool		lastfl;
	char *		p, *path_sub, sep = '\0';
	char *		path_absdrive;
	char		path_cwd[PATH_MAX + 1];
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	if (!WFR_SUCCESS_POSIX(status, (getcwd(path_cwd, sizeof(path_cwd))))) {
		return status;
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
	     WFR_SUCCESS(status) && *p; p++)
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

			if (!WFR_SUCCESS_POSIX(status, (mkdir(path_absdrive ? path_absdrive : path_sub) == 0))
			&&  ((WFR_STATUS_CONDITION(status) == EEXIST) && existsfl))
			{
				status = WFR_STATUS_CONDITION_SUCCESS;
			}

		change_dir:
			if (!lastfl && WFR_SUCCESS(status)) {
				WFR_SUCCESS_POSIX(status, chdir(path_absdrive ? path_absdrive : path_sub) == 0);
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

WfrStatus
WfrPathNameToAbsoluteW(
	const wchar_t *		pname,
	wchar_t **		ppname_abs
	)
{
	LPWSTR		path_abs;
	DWORD		path_abs_size;
	WfrStatus	status;


	if (WFR_IS_ABSOLUTE_PATHW(pname)) {
		WFR_SUCCESS_POSIX(status, (*ppname_abs = wcsdup(pname)));
	} else if (WFR_SUCCESS_WINDOWS(status, ((path_abs_size = GetFullPathNameW(pname, 0, NULL, NULL)) > 0))
		&& WFR_SUCCESS_POSIX(status, (path_abs = WFR_NEWN(path_abs_size * sizeof(path_abs[0]), WCHAR)))
		&& WFR_SUCCESS_WINDOWS(status, (GetFullPathNameW(pname, path_abs_size, path_abs, NULL) > 0)))
	{
		*ppname_abs = path_abs;
	}

	return status;
}

WfrStatus
WfrPathNameToDirectory(
	char *		pname,
	char **		pdname
	)
{
	char *		dname;
	size_t		dname_len;
	char *		dname_end;
	size_t		pname_len;
	struct stat	statbuf;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS_ERRNO1(status, EINVAL,
		(pname_len = strlen(pname))))
	{
		if ((stat(pname, &statbuf) == 0)
		&&  (statbuf.st_mode & S_IFDIR))
		{
			dname_len = pname_len;
		} else {
			for (dname_end = &pname[pname_len - 1];
			     (dname_end > pname) && (*dname_end != '/') && (*dname_end != '\\');
			     dname_end--);

			dname_len = (dname_end > pname) ? (dname_end - pname) : 1;
		}

		if (WFR_SUCCESS_POSIX(status,
			(dname = WFR_NEWN(dname_len + 1, char))))
		{
			memcpy(dname, pname, dname_len);
			dname[dname_len] = '\0';
			*pdname = dname;
		}
	}

	return status;
}

WfrStatus
WfrPathNameToDirectoryW(
	wchar_t *	pname,
	wchar_t **	pdname
	)
{
	wchar_t *	dname;
	size_t		dname_len;
	wchar_t *	dname_end;
	size_t		pname_len;
	struct _stat64	statbuf;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS_ERRNO1(status, EINVAL,
		(pname_len = wcslen(pname))))
	{
		if ((_wstat64(pname, &statbuf) == 0)
		&&  (statbuf.st_mode & S_IFDIR))
		{
			dname_len = pname_len;
		} else {
			for (dname_end = &pname[pname_len - 1];
			     (dname_end > pname) && (*dname_end != '/') && (*dname_end != '\\');
			     dname_end--);

			dname_len = (dname_end > pname) ? (dname_end - pname) : 1;
		}

		for (dname_end = &pname[pname_len - 1];
		     (dname_end > pname) && (*dname_end != L'/') && (*dname_end != L'\\');
		     dname_end--);

		dname_len = (dname_end > pname) ? (dname_end - pname) : 1;
		if (WFR_SUCCESS_POSIX(status,
			(dname = WFR_NEWN((dname_len + 1) * sizeof(dname[0]), wchar_t))))
		{
			memcpy(dname, pname, dname_len * sizeof(dname[0]));
			dname[dname_len] = L'\0';
			*pdname = dname;
		}
	}

	return status;
}

WfrStatus
WfrRenameFile(
	bool		escape_fnamefl,
	const char *	dname,
	const char *	ext,
	const char *	fname,
	const char *	fname_new
	)
{
	char		pname[MAX_PATH + 1], pname_new[MAX_PATH + 1];
	WfrStatus	status;


	if (escape_fnamefl) {
		if (WFR_SUCCESS(status = WfrEscapeFileName(
				dname, ext, fname, false,
				pname, sizeof(pname)))
		&&  WFR_SUCCESS(status = WfrEscapeFileName(
				dname, ext, fname_new, false,
				pname_new, sizeof(pname_new))))
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	} else {
		WFR_SNPRINTF_PNAME(pname, sizeof(pname), dname, ext, fname);
		WFR_SNPRINTF_PNAME(pname_new, sizeof(pname_new), dname, ext, fname_new);
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_SUCCESS(status)) {
		WFR_SUCCESS_WINDOWS(status,
			MoveFileEx(
				pname, pname_new, MOVEFILE_REPLACE_EXISTING));
	}

	return status;
}

WfrStatus
WfrUnescapeFileName(
	char *		fname,
	const char **	pname
	)
{
	bool		catfl;
	char		ch;
	char *		name = NULL;
	size_t		name_len, name_size;
	char		*p;
	char		seq[3];
	WfrStatus	status;


	name_size = strlen(fname) + 1;
	if (WFR_SUCCESS_POSIX(status, (name = WFR_NEWN(name_size, char)))) {
		memset(name, 0, name_size);

		do {
			for (catfl = false, p = fname; *p; p++) {
				if ( (p[0] == '%')
				&& (((p[1] >= '0') && (p[1] <= '9')) || ((p[1] >= 'a') && (p[1] <= 'f')))
				&& (((p[2] >= '0') && (p[2] <= '9')) || ((p[2] >= 'a') && (p[2] <= 'f'))))
				{
					seq[0] = p[1]; seq[1] = p[2]; seq[2] = '\0';
					ch = (char)strtoul(seq, NULL, 16);

					if (WFR_SUCCESS(status = WFR_RESIZE(
							name, name_size, name_size + 1, char)))
					{
						strncat(name, fname, p - fname);
						name_len = strlen(name);
						WFR_SNPRINTF(
								&name[name_len],
								name_size - name_len,
								"%c", ch);
						fname = (p += 3); catfl = true;
						break;
					}
				}
			}

			if (!catfl) {
				strcat(name, fname);
				break;
			}
		} while (WFR_SUCCESS(status));

		if (WFR_SUCCESS(status)) {
			*pname = name;
		} else {
			WFR_FREE(name);
		}
	}

	return status;
}

WfrStatus
WfrWatchDirectory(
	bool			display_errorsfl,
	char **			pdname,
	CRITICAL_SECTION *	dname_cs,
	HWND			hwnd,
	UINT			window_msg,
	HANDLE *		phEvent,
	HANDLE *		phThread
	)
{
	WfrWatchDirectoryContext *	ctx = NULL;
	HANDLE				hEvent = NULL;
	HANDLE				hThread;
	WfrStatus			status;


	InitializeCriticalSection(dname_cs);

	if (WFR_SUCCESS_WINDOWS(status, (hEvent = CreateEvent(NULL, FALSE, FALSE, NULL)))
	&&  WFR_SUCCESS_POSIX(status, (ctx = WFR_NEW(WfrWatchDirectoryContext)))
	&&  WFR_SUCCESS(status = WFR_WATCH_DIRECTORY_CONTEXT_INIT1(
		*ctx, display_errorsfl, pdname, dname_cs, hEvent, hwnd, window_msg))
	&&  WFR_SUCCESS_WINDOWS(status,
		(hThread = CreateThread(NULL, 0, WfrpWatchDirectoryThreadProc, ctx, 0, NULL))))
	{
		*phEvent = hEvent;
		*phThread = hThread;
	}

	if (WFR_FAILURE(status)) {
		WFR_FREE_IF_NOTNULL(ctx);
		if (hEvent) {
			(void)CloseHandle(hEvent);
		}
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
