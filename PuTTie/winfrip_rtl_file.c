/*
 * winfrip_rtl_file.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_file.h"

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
WfrDeleteFiles(
	const char *	dname,
	const char *	ext
	)
{
	struct dirent *		dire;
	DIR *			dirp = NULL;
	size_t			ext_len;
	char			fname[MAX_PATH + 1];
	char			path_cwd[MAX_PATH + 1];
	char *			pext;
	struct stat		statbuf;
	WfrStatus		status;


	ext_len = strlen(ext);

	if (stat(dname, &statbuf) < 0) {
		status = WFR_STATUS_FROM_ERRNO();
		if (WFR_STATUS_CONDITION(status) == ENOENT) {
			return WFR_STATUS_CONDITION_SUCCESS;
		} else {
			return status;
		}
	}

	if (!(dirp = opendir(dname))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else if (!getcwd(path_cwd, sizeof(path_cwd))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
		while (WFR_STATUS_SUCCESS(status) && (dire = readdir(dirp))) {
			if (chdir(dname) < 0) {
				status = WFR_STATUS_FROM_ERRNO();
			} else if (stat(dire->d_name, &statbuf) < 0) {
				status = WFR_STATUS_FROM_ERRNO();
				(void)chdir(path_cwd);
			} else if (chdir(path_cwd) < 0) {
				status = WFR_STATUS_FROM_ERRNO();
			} else if (!(statbuf.st_mode & S_IFREG)) {
				continue;
			} else if ((dire->d_name[0] == '.')
				&& (dire->d_name[1] == '\0'))
			{
				continue;
			} else if ((dire->d_name[0] == '.')
				&& (dire->d_name[1] == '.')
				&& (dire->d_name[2] == '\0'))
			{
				continue;
			} else if ((pext = strstr(dire->d_name, ext))
				&& (pext[ext_len] == '\0'))
			{
				WFR_SNPRINTF(fname, sizeof(fname), "%s/%s", dname, dire->d_name);
				if (unlink(fname) < 0) {
					status = WFR_STATUS_FROM_ERRNO();
				}
			}
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
	WfrEnumerateFilesState *	state
	)
{
	size_t		ext_len;
	char *		fname, *fname_ext;
	char		path_cwd[MAX_PATH + 1];
	struct stat	statbuf;
	WfrStatus	status;


	if (!state->dirp) {
		*pdonefl = true;
		*pfname = NULL;
		state->donefl = true;
		WFR_ENUMERATE_FILES_STATE_INIT(*state);
		return WFR_STATUS_CONDITION_SUCCESS;
	}

	if (!getcwd(path_cwd, sizeof(path_cwd))) {
		return WFR_STATUS_FROM_ERRNO();
	}

	errno = 0;
	ext_len = strlen(ext);
	status = WFR_STATUS_CONDITION_SUCCESS;

	while (WFR_STATUS_SUCCESS(status)
	&&     (state->dire = readdir(state->dirp)))
	{
		if (chdir(state->path) < 0) {
			status = WFR_STATUS_FROM_ERRNO();
		} else if (stat(state->dire->d_name, &statbuf) < 0) {
			status = WFR_STATUS_FROM_ERRNO();
			(void)chdir(path_cwd);
		} else if (chdir(path_cwd) < 0) {
			status = WFR_STATUS_FROM_ERRNO();
		} else if (!(statbuf.st_mode & S_IFREG)) {
			continue;
		} else if ((state->dire->d_name[0] == '.')
			&& (state->dire->d_name[1] == '\0'))
		{
			continue;
		} else if ((state->dire->d_name[0] == '.')
			&& (state->dire->d_name[1] == '.')
			&& (state->dire->d_name[2] == '\0'))
		{
			continue;
		} else {
			fname = state->dire->d_name;
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
				state->donefl = false;
				return WFR_STATUS_CONDITION_SUCCESS;
			}
		}
	}

	if (errno == 0) {
		*pdonefl = true;
		*pfname = NULL;
		state->donefl = true;
		(void)closedir(state->dirp);
		WFR_FREE(state);
		status = WFR_STATUS_CONDITION_SUCCESS;
	} else {
		status = WFR_STATUS_FROM_ERRNO();
	}

	return status;
}

WfrStatus
WfrEnumerateFilesInit(
	const char *			dname,
	WfrEnumerateFilesState **	state
	)
{
	struct stat	statbuf;
	WfrStatus	status;


	if (!((*state) = WFR_NEW(WfrEnumerateFilesState))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		WFR_ENUMERATE_FILES_STATE_INIT(**state);
		if (!((*state)->path = strdup(dname))) {
			status = WFR_STATUS_FROM_ERRNO();
		} else if (stat(dname, &statbuf) < 0) {
			status = WFR_STATUS_FROM_ERRNO();
			if (WFR_STATUS_CONDITION(status) == ENOENT) {
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		} else if (!((*state)->dirp = opendir(dname))) {
			status = WFR_STATUS_FROM_ERRNO();
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	if (WFR_STATUS_FAILURE(status)) {
		WFR_FREE_IF_NOTNULL((*state)->path);
		WFR_FREE_IF_NOTNULL(*state);
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
	if (!(name_escaped = WFR_NEWN(name_escaped_size, char))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
		memset(name_escaped, 0, name_escaped_size);

		do {
			for (catfl = false, p = (char *)name; *p; p++) {
				if ((*p == '"') || (*p == '*') || (*p == ':')
				||  (*p == '<') || (*p == '>') || (*p == '?')
				||  (*p == '|') || (*p == '/') || (*p == '\\'))
				{
					if (WFR_STATUS_SUCCESS(status = WFR_RESIZE(
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
		} while (WFR_STATUS_SUCCESS(status));

		if (WFR_STATUS_SUCCESS(status)) {
			WFR_SNPRINTF(
				fname, fname_size, "%s/%s%s%s",
				dname, name_escaped, ext,
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
	if (!(name = WFR_NEWN(name_size, char))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
		memset(name, 0, name_size);

		do {
			for (catfl = false, p = fname; *p; p++) {
				if ( (p[0] == '%')
				&& (((p[1] >= '0') && (p[1] <= '9')) || ((p[1] >= 'a') && (p[1] <= 'f')))
				&& (((p[2] >= '0') && (p[2] <= '9')) || ((p[2] >= 'a') && (p[2] <= 'f'))))
				{
					seq[0] = p[1]; seq[1] = p[2]; seq[2] = '\0';
					ch = (char)strtoul(seq, NULL, 16);

					if (WFR_STATUS_SUCCESS(status = WFR_RESIZE(
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
		} while (WFR_STATUS_SUCCESS(status));

		if (WFR_STATUS_SUCCESS(status)) {
			*pname = name;
		} else {
			WFR_FREE(name);
		}
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
