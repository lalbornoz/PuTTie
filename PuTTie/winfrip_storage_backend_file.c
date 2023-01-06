/*
 * winfrip_storage_backend_file.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include "PuTTie/winfrip_rtl_status.h"
#include "PuTTie/winfrip_storage_jumplist_wrap.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_pcre2.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_backend_file.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/*
 * Private types
 */

/*
 * WfspFileReMatchesOffset: offsets of regular expression capture groups used to
 * match fields of interest in PuTTie session files during parsing thereof
 *
 * WfsppFileRegex: regular expression including ancillary data used to parse PuTTie
 * session files
 */

typedef enum WfspFileReMatchesOffset {
	WFSP_FILE_RMO_KEY				= 1,
	WFSP_FILE_RMO_VALUE_TYPE		= 2,
	WFSP_FILE_RMO_VALUE				= 3,
} WfspFileReMatchesOffset;

/*
 * Private variables
 */

/*
 * WfsppFileAppData: absolute pathnamne to the current user's application data directory (%APPDATA%)
 * WfsppFileDnameHostKeys: absolute pathname to the current user's directory of PuTTie host key files
 * WfsppFileDnameSessions: absolute pathname to the current user's directory of PuTTie session files
 * WfsppFileExtHostKeys: file name extension of PuTTie host key files
 * WfsppFileExtSessions: file name extension of PuTTie session files
 */

static char *			WfsppFileAppData = NULL;
static char				WfsppFileDnameHostKeys[MAX_PATH + 1] = "";
static char				WfsppFileDnameSessions[MAX_PATH + 1] = "";
static char				WfsppFileExtHostKeys[] = ".hostkey";
static char				WfsppFileExtSessions[] = ".ini";

static WinfrippP2Regex	WfsppFileRegex = {
	.ovecsize = 16,
	.spec_w = L"^([^=]+)=(int|string):(.*)$",
	.code = NULL,
	.error_message = {0}, .errorcode = 0, .erroroffset = 0,
	.md = NULL,
	.ovec = NULL,
};

/*
 * Private subroutine prototypes
 */

static WfrStatus		WfsppFileClear(const char *dname, const char *ext);
static WfrStatus		WfsppFileEnumerateInit(const char *dname, WfspFileEnumerateState **enum_state);
static WfrStatus		WfsppFileInitAppDataSubdir(void);
static WfrStatus		WfsppFileInitRegex(void);
static WfrStatus		WfsppFileNameEscape(const char *dname, const char *ext, const char *name, bool tmpfl, char *fname, size_t fname_size);
static WfrStatus		WfsppFileNameUnescape(char *fname, const char **pname);

/*
 * Private subroutines
 */

static WfrStatus
WfsppFileClear(
	const char *	dname,
	const char *	ext
	)
{
	struct dirent *		dire;
	DIR *				dirp = NULL;
	size_t				ext_len;
	char				fname[MAX_PATH + 1];
	char *				pext;
	struct stat			statbuf;
	WfrStatus			status;


	ext_len = strlen(ext);

	if ((stat(dname, &statbuf) < 0)
	||  (!(dirp = opendir(dname)))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;

		while ((dire = readdir(dirp))) {
			if ((dire->d_name[0] == '.')
			&&  (dire->d_name[1] == '\0')) {
				continue;
			} else if ((dire->d_name[0] == '.')
				   &&  (dire->d_name[1] == '.')
				   &&  (dire->d_name[2] == '\0')) {
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

static WfrStatus
WfsppFileEnumerateInit(
	const char *				dname,
	WfspFileEnumerateState **	enum_state
	)
{
	struct stat		statbuf;
	WfrStatus		status;


	if (!(((*enum_state) = snew(WfspFileEnumerateState)))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		WFSP_FILE_ENUMERATE_STATE_INIT((**enum_state));
		if ((stat(dname, &statbuf) < 0)
		|| (!((*enum_state)->dirp = opendir(dname))))
		{
			sfree((*enum_state)); *enum_state = NULL;
			status = WFR_STATUS_FROM_ERRNO();
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	return status;
}

static WfrStatus
WfsppFileInitAppDataSubdir(
	void
	)
{
	char *		appdata;
	WfrStatus	status;


	if (!(appdata = getenv("APPDATA"))) {
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);
	} else {
		WfsppFileAppData = appdata;
		WFR_SNPRINTF(
				WfsppFileDnameHostKeys, sizeof(WfsppFileDnameHostKeys),
				"%s/PuTTie/hostkeys", WfsppFileAppData);
		WFR_SNPRINTF(
				WfsppFileDnameSessions, sizeof(WfsppFileDnameSessions),
				"%s/PuTTie/sessions", WfsppFileAppData);

		if ((mkdir(WfsppFileDnameHostKeys) < 0) && (errno != EEXIST)) {
			status = WFR_STATUS_FROM_ERRNO();
		} else if ((mkdir(WfsppFileDnameSessions) < 0) && (errno != EEXIST)) {
			status = WFR_STATUS_FROM_ERRNO();
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	return status;
}

static WfrStatus
WfsppFileInitRegex(
	void
	)
{
	WfrStatus	status;


	if (WfsppFileRegex.code) {
		pcre2_code_free(WfsppFileRegex.code); WfsppFileRegex.code = NULL;
	}

	WfsppFileRegex.code = pcre2_compile(
		WfsppFileRegex.spec_w, PCRE2_ANCHORED | PCRE2_ZERO_TERMINATED,
		0, &WfsppFileRegex.errorcode, &WfsppFileRegex.erroroffset, NULL);

	if (!WfsppFileRegex.code) {
		pcre2_get_error_message(
			WfsppFileRegex.errorcode,
			WfsppFileRegex.error_message,
			sizeof(WfsppFileRegex.error_message) / sizeof(WfsppFileRegex.error_message[0]));
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
	} else {
		WfsppFileRegex.md = pcre2_match_data_create(WfsppFileRegex.ovecsize, NULL);
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

static WfrStatus
WfsppFileNameEscape(
	const char *	dname,
	const char *	ext,
	const char *	name,
	bool			tmpfl,
	char *			fname,
	size_t			fname_size
	)
{
	bool		catfl;
	char *		name_escaped = NULL;
	size_t		name_escaped_len, name_escaped_size;
	char		*p;
	WfrStatus	status;


	name_escaped_size = strlen(name) + 1;
	if (!(name_escaped = snewn(name_escaped_size, char))) {
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
					if (WFR_STATUS_SUCCESS(status = WFR_SRESIZE_IF_NEQ_SIZE(
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

	WFR_SFREE_IF_NOTNULL(name_escaped);

	return status;
}

static WfrStatus
WfsppFileNameUnescape(
	char *			fname,
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
	if (!(name = snewn(name_size, char))) {
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

					if (WFR_STATUS_SUCCESS(status = WFR_SRESIZE_IF_NEQ_SIZE(
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
			sfree(name);
		}
	}

	return status;
}

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfspFileClearHostKeys(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfsppFileClear(WfsppFileDnameHostKeys, ".hostkey");
}

WfrStatus
WfspFileDeleteHostKey(
	WfsBackend		backend,
	const char *	key_name
	)
{
	char			fname[MAX_PATH];
	struct stat		statbuf;
	WfrStatus		status;


	(void)backend;

	if (WFR_STATUS_SUCCESS(status = WfsppFileNameEscape(
						WfsppFileDnameHostKeys,
						WfsppFileExtHostKeys, key_name,
						false, fname, sizeof(fname))))
	{
		if ((stat(fname, &statbuf) < 0)) {
			status = WFR_STATUS_FROM_ERRNO();
		} else if (unlink(fname) < 0) {
			status = WFR_STATUS_FROM_ERRNO();
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	return status;
}

WfrStatus
WfspFileEnumerateHostKeys(
	WfsBackend		backend,
	bool			initfl,
	bool *			pdonefl,
	const char **	pkey_name,
	void *			state
	)
{
	WfspFileEnumerateState *	enum_state;
	char *						fname, *fname_ext;
	WfrStatus					status;


	(void)backend;

	if (initfl) {
		return WfsppFileEnumerateInit(
				WfsppFileDnameHostKeys,
				(WfspFileEnumerateState **)state);
	}

	enum_state = (WfspFileEnumerateState *)state; errno = 0;
	while ((enum_state->dire = readdir(enum_state->dirp))) {
		if ((enum_state->dire->d_name[0] == '.')
		&&  (enum_state->dire->d_name[1] == '\0')) {
			continue;
		} else if ((enum_state->dire->d_name[0] == '.')
			   &&  (enum_state->dire->d_name[1] == '.')
			   &&  (enum_state->dire->d_name[2] == '\0')) {
			continue;
		} else {
			*pdonefl = false;
			fname = enum_state->dire->d_name;

			if ((fname_ext = strstr(fname, ".hostkey"))
			&&  (fname_ext[sizeof(".hostkey") - 1] == '\0'))
			{
				*fname_ext = '\0';
				status = WfsppFileNameUnescape(fname, pkey_name);
				goto out;
			}
		}
	}

	if (errno == 0) {
		*pdonefl = true;
		*pkey_name = NULL;
		enum_state->donefl = true;
		(void)closedir(enum_state->dirp);
		WFSP_FILE_ENUMERATE_STATE_INIT(*enum_state);
		status = WFR_STATUS_CONDITION_SUCCESS;
	} else {
		status = WFR_STATUS_FROM_ERRNO();
	}

out:
	return status;
}

WfrStatus
WfspFileLoadHostKey(
	WfsBackend		backend,
	const char *	key_name,
	const char **	pkey
	)
{
	FILE *			file = NULL;
	char			fname[MAX_PATH + 1];
	char *			key = NULL;
	struct stat		statbuf;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WfsppFileNameEscape(
						WfsppFileDnameHostKeys,
						WfsppFileExtHostKeys, key_name,
						false, fname, sizeof(fname))))
	{
		if ((stat(fname, &statbuf) < 0)
		||  (!(file = fopen(fname, "rb")))
		||	(!(key = snewn(statbuf.st_size + 1, char)))) {
			status = WFR_STATUS_FROM_ERRNO();
		} else if (fread(key, statbuf.st_size, 1, file) != 1) {
			if (statbuf.st_size == 0) {
				status = WFR_STATUS_FROM_ERRNO1(ENODATA);
			} else {
				status = WFR_STATUS_FROM_ERRNO();
				if (feof(file)) {
					status = WFR_STATUS_FROM_ERRNO1(ENODATA);
				}
			}
		} else {
			key[statbuf.st_size] = '\0';
			status = WfsSetHostKey(backend, key_name, key);
		}

		if (file) {
			(void)fclose(file);
		}
	}

	if (WFR_STATUS_SUCCESS(status)) {
		*pkey = key;
	} else {
		sfree(key);
	}

	return status;
}

WfrStatus
WfspFileRenameHostKey(
	WfsBackend		backend,
	const char *	key_name,
	const char *	key_name_new
	)
{
	char		fname[MAX_PATH + 1], fname_new[MAX_PATH + 1];
	WfrStatus	status;


	(void)backend;

	if (WFR_STATUS_SUCCESS(status = WfsppFileNameEscape(
						WfsppFileDnameHostKeys,
						WfsppFileExtHostKeys, key_name,
						false, fname, sizeof(fname)))
	&&  WFR_STATUS_SUCCESS(status = WfsppFileNameEscape(
						WfsppFileDnameHostKeys,
						WfsppFileExtHostKeys, key_name_new,
						false, fname_new, sizeof(fname_new))))
	{
		if (MoveFileEx(fname, fname_new, MOVEFILE_REPLACE_EXISTING) < 0) {
			status = WFR_STATUS_FROM_WINDOWS();
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	return status;
}

WfrStatus
WfspFileSaveHostKey(
	WfsBackend		backend,
	const char *	key_name,
	const char *	key
	)
{
	char *		dname_tmp;
	int			fd = -1;
	FILE *		file = NULL;
	char		fname[MAX_PATH + 1], fname_tmp[MAX_PATH + 1];
	size_t		key_len;
	size_t		nwritten;
	WfrStatus	status;


	(void)backend;

	if (!(dname_tmp = getenv("TEMP"))
	||  !(dname_tmp = getenv("TMP"))) {
		dname_tmp = "./";
	}

	if (WFR_STATUS_SUCCESS(status = WfsppFileNameEscape(
						WfsppFileDnameHostKeys,
						WfsppFileExtHostKeys, key_name,
						false, fname, sizeof(fname)))
	&&  WFR_STATUS_SUCCESS(status = WfsppFileNameEscape(
						dname_tmp, WfsppFileExtHostKeys, key_name,
						true, fname_tmp, sizeof(fname_tmp))))
	{
		key_len = strlen(key);

		if ((fd = mkstemp(fname_tmp)) < 0) {
			status = WFR_STATUS_FROM_ERRNO();
		} else if (!(file = fdopen(fd, "wb"))) {
			status = WFR_STATUS_FROM_ERRNO();
		} else if (key_len > 0) {
			if ((nwritten = fwrite(key, key_len, 1, file)) != 1) {
				if (ferror(file) < 0) {
					status = WFR_STATUS_FROM_ERRNO();
				} else {
					status = WFR_STATUS_FROM_ERRNO1(EPIPE);
				}
			} else {
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		if (file) {
			(void)fflush(file);
			(void)fclose(file);
		} else if ((fd >= 0)) {
			(void)close(fd);
		}
	}

	if (WFR_STATUS_SUCCESS(status)) {
		if (MoveFileEx(fname_tmp, fname, MOVEFILE_REPLACE_EXISTING) < 0) {
			status = WFR_STATUS_FROM_WINDOWS();
			(void)unlink(fname_tmp);
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	} else {
		(void)unlink(fname_tmp);
	}

	return status;
}


WfrStatus
WfspFileClearSessions(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfsppFileClear(WfsppFileDnameSessions, ".ini");
}

WfrStatus
WfspFileCloseSession(
	WfsBackend		backend,
	WfspSession *	session
	)
{
	(void)backend;
	(void)session;

	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspFileDeleteSession(
	WfsBackend		backend,
	const char *	sessionname
	)
{
	char			fname[MAX_PATH + 1];
	struct stat		statbuf;
	WfrStatus		status;


	(void)backend;

	if (WFR_STATUS_FAILURE(status = WfsppFileNameEscape(
							WfsppFileDnameSessions,
							WfsppFileExtSessions, sessionname,
							false, fname, sizeof(fname))))
	{
		return status;
	}

	if ((stat(fname, &statbuf) < 0)) {
		status = WFR_STATUS_FROM_ERRNO();
	} else if (unlink(fname) < 0) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfspFileEnumerateSessions(
	WfsBackend	backend,
	bool		initfl,
	bool *		pdonefl,
	char **		psessionname,
	void *		state
	)
{
	WfspFileEnumerateState *	enum_state;
	char *						fname, *pext;
	char *						sessionname;
	size_t						sessionname_len;
	WfrStatus					status;


	(void)backend;

	if (initfl) {
		return WfsppFileEnumerateInit(
				WfsppFileDnameSessions,
				(WfspFileEnumerateState **)state);
	}

	enum_state = (WfspFileEnumerateState *)state; errno = 0;
	while ((enum_state->dire = readdir(enum_state->dirp))) {
		if ((enum_state->dire->d_name[0] == '.')
		&&  (enum_state->dire->d_name[1] == '\0')) {
			continue;
		} else if ((enum_state->dire->d_name[0] == '.')
			   &&  (enum_state->dire->d_name[1] == '.')
			   &&  (enum_state->dire->d_name[2] == '\0')) {
			continue;
		} else {
			fname = enum_state->dire->d_name;
			if ((pext = strstr(fname, ".ini"))) {
				sessionname_len = pext - fname;
			} else {
				sessionname_len = strlen(fname);
			}

			if (!(sessionname = strdup(fname))) {
				status = WFR_STATUS_FROM_ERRNO();
			} else {
				*pdonefl = false;
				sessionname[sessionname_len] = '\0';
				status = WfsppFileNameUnescape(sessionname, (const char **)psessionname);
			}
			goto out;
		}
	}

	if (errno == 0) {
		*pdonefl = true;
		*psessionname = NULL;
		enum_state->donefl = true;
		(void)closedir(enum_state->dirp);
		WFSP_FILE_ENUMERATE_STATE_INIT(*enum_state);
		status = WFR_STATUS_CONDITION_SUCCESS;
	} else {
		status = WFR_STATUS_FROM_ERRNO();
	}

out:
	return status;
}

WfrStatus
WfspFileLoadSession(
	WfsBackend		backend,
	const char *	sessionname,
	WfspSession **	psession
	)
{
	bool				addedfl = false;
	FILE *				file = NULL;
	char				fname[MAX_PATH];
	char *				fname_buf = NULL;
	WfspTreeItemType	item_type;
	char *				item_type_string;
	size_t				item_type_string_size;
	char *				key;
	size_t				key_size;
	char *				line, *line_sep;
	size_t				line_len;
	wchar_t *			line_w = NULL;
	int					nmatches;
	char *				p;
	WfspSession *		session;
	struct stat			statbuf;
	WfrStatus			status;
	void *				value_new;
	size_t				value_new_size;


	if (WFR_STATUS_FAILURE(status = WfsppFileNameEscape(
							WfsppFileDnameSessions,
							WfsppFileExtSessions, sessionname,
							false, fname, sizeof(fname))))
	{
		return status;
	}

	if ((stat(fname, &statbuf) < 0)
	||  (!(file = fopen(fname, "rb")))
	||	(!(fname_buf = snewn(statbuf.st_size + 1, char)))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else if (fread(fname_buf, statbuf.st_size, 1, file) != 1) {
		if (statbuf.st_size == 0) {
			status = WFR_STATUS_FROM_ERRNO1(ENODATA);
		} else {
			status = WFR_STATUS_FROM_ERRNO();
			if (feof(file)) {
				status = WFR_STATUS_FROM_ERRNO1(ENODATA);
			}
		}
	} else {
		status = WfsGetSession(backend, true, sessionname, &session);
		if (WFR_STATUS_SUCCESS(status)) {
			if (session->mtime == statbuf.st_mtime) {
				goto out;
			} else {
				session->mtime = statbuf.st_mtime;
				status = WfsClearSession(backend, session, sessionname);
				addedfl = false;
			}
		} else if (WFR_STATUS_CONDITION(status) == ENOENT) {
			status = WfsAddSession(backend, sessionname, &session);
			addedfl = WFR_STATUS_SUCCESS(status);
		}

		if (WFR_STATUS_SUCCESS(status)) {
			WfsppFileRegex.ovec = pcre2_get_ovector_pointer(WfsppFileRegex.md);
			memset(WfsppFileRegex.ovec, 0, pcre2_get_ovector_count(WfsppFileRegex.md) * 2 * sizeof(*WfsppFileRegex.ovec));

			fname_buf[statbuf.st_size] = '\0';
			p = &fname_buf[0];
			do {
				line = p; line_sep = strchr(line, '\n');
				if (line_sep) {
					*line_sep = L'\0', p = line_sep + 1;
					if ((line_sep[-1] == L'\r')
					&&  (&line_sep[-1] >= line)) {
						line_sep[-1] = L'\0';
					}
				}

				line_len = strlen(line);
				if ((line[0] != L'#') && (strlen(line) > 0)) {
					if (WFR_STATUS_SUCCESS(status = WfrToWcsDup(line, line_len + 1, &line_w))
					&&  ((nmatches = pcre2_match(
								WfsppFileRegex.code, line_w, line_len,
								0, 0, WfsppFileRegex.md, NULL)) <= (WfsppFileRegex.ovecsize / 2)))
					{
						key = NULL; key_size = 0;
						item_type_string = NULL; item_type_string_size = 0;
						value_new = NULL; value_new_size = 0;

						if (WFR_STATUS_SUCCESS(status = WinfrippPcre2GetMatch(
									&WfsppFileRegex, true, WFSP_FILE_RMO_KEY,
									WINFRIPP_P2RTYPE_STRING, line_w, &key,
									&key_size))
						&&  WFR_STATUS_SUCCESS(status = WinfrippPcre2GetMatch(
									&WfsppFileRegex, true, WFSP_FILE_RMO_VALUE_TYPE,
									WINFRIPP_P2RTYPE_STRING, line_w, &item_type_string,
									&item_type_string_size)))
						{
							if (memcmp(item_type_string, "int", item_type_string_size) == 0) {
								item_type = WFSP_TREE_ITYPE_INT;
								status = WinfrippPcre2GetMatch(
										&WfsppFileRegex, true, WFSP_FILE_RMO_VALUE,
										WINFRIPP_P2RTYPE_INT, line_w,
										&value_new, &value_new_size);
							} else if (memcmp(item_type_string, "string", item_type_string_size) == 0) {
								item_type = WFSP_TREE_ITYPE_STRING;
								status = WinfrippPcre2GetMatch(
										&WfsppFileRegex, true, WFSP_FILE_RMO_VALUE,
										WINFRIPP_P2RTYPE_STRING, line_w,
										&value_new, &value_new_size);
							} else {
								status = WFR_STATUS_FROM_ERRNO1(EINVAL);
							}

							sfree(item_type_string);

							if (WFR_STATUS_SUCCESS(status)) {
								status = WfsSetSessionKey(
										session, key, value_new,
										value_new_size, item_type);
							} else {
								sfree(value_new);
							}

							WFR_SFREE_IF_NOTNULL(key);
						}
					}

					WFR_SFREE_IF_NOTNULL(line_w); line_w = NULL;
				}
			} while (WFR_STATUS_SUCCESS(status) && line_sep && (p < &fname_buf[statbuf.st_size]));
		}
	}

out:
	WFR_SFREE_IF_NOTNULL(fname_buf);
	if (file) {
		(void)fclose(file);
	}

	if (WFR_STATUS_SUCCESS(status)) {
		if (psession) {
			*psession = session;
		}
	} else {
		if (addedfl && session) {
			(void)WfsDeleteSession(backend, false, sessionname);
		}
	}

	return status;
}

WfrStatus
WfspFileRenameSession(
	WfsBackend		backend,
	const char *	sessionname,
	const char *	sessionname_new
	)
{
	char		fname[MAX_PATH + 1], fname_new[MAX_PATH + 1];
	WfrStatus	status;


	(void)backend;

	if (WFR_STATUS_FAILURE(status = WfsppFileNameEscape(
							WfsppFileDnameSessions,
							WfsppFileExtSessions, sessionname,
							false, fname, sizeof(fname)))
	||  WFR_STATUS_FAILURE(status = WfsppFileNameEscape(
							WfsppFileDnameSessions,
							WfsppFileExtSessions, sessionname_new,
							false, fname_new, sizeof(fname_new))))
	{
		return status;
	}

	if (MoveFileEx(fname, fname_new, MOVEFILE_REPLACE_EXISTING) < 0) {
		status = WFR_STATUS_FROM_WINDOWS();
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfspFileSaveSession(
	WfsBackend		backend,
	WfspSession *	session
	)
{
	char *			dname_tmp;
	int				fd = -1;
	FILE *			file = NULL;
	WfspTreeItem *	item;
	char			fname[MAX_PATH], fname_tmp[MAX_PATH];
	int				rc;
	WfrStatus		status;


	(void)backend;

	if (!(dname_tmp = getenv("TEMP"))
	||  !(dname_tmp = getenv("TMP"))) {
		dname_tmp = "./";
	}

	if (WFR_STATUS_FAILURE(status = WfsppFileNameEscape(
							WfsppFileDnameSessions,
							WfsppFileExtSessions, session->name,
							false, fname, sizeof(fname)))
	||  WFR_STATUS_FAILURE(status = WfsppFileNameEscape(
							dname_tmp, WfsppFileExtSessions, session->name,
							true, fname_tmp, sizeof(fname_tmp))))
	{
		return status;
	}

	if ((fd = mkstemp(fname_tmp)) < 0) {
		status = WFR_STATUS_FROM_ERRNO();
	} else if (!(file = fdopen(fd, "wb"))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
		WFSP_TREE234_FOREACH(status, session->tree, idx, item) {
			switch (item->type) {
			default:
				rc = 0; break;

			case WFSP_TREE_ITYPE_INT:
				rc = fprintf(file, "%s=int:%d\r\n",
						(char *)item->key, *((int *)item->value));
				break;

			case WFSP_TREE_ITYPE_STRING:
				rc = fprintf(file, "%s=string:%s\r\n",
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

	if (WFR_STATUS_SUCCESS(status)) {
		if (MoveFileEx(fname_tmp, fname, MOVEFILE_REPLACE_EXISTING) < 0) {
			status = WFR_STATUS_FROM_WINDOWS();
			(void)unlink(fname_tmp);
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	} else {
		(void)unlink(fname_tmp);
	}

	return status;
}


void
WfspFileJumpListAdd(
	const char *const	sessionname
	)
{
	add_session_to_jumplist_PuTTY(sessionname);
}

void
WfspFileJumpListClear(
	void
	)
{
	clear_jumplist_PuTTY();
}

void
WfspFileJumpListRemove(
	const char *const	sessionname
	)
{
	remove_session_from_jumplist_PuTTY(sessionname);
}


WfrStatus
WfspFileInit(
	void
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsppFileInitAppDataSubdir())
	&&  WFR_STATUS_SUCCESS(status = WfsppFileInitRegex()))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfspFileSetBackend(
	WfsBackend	backend_new
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsClearHostKeys(backend_new, false))
	&&  WFR_STATUS_SUCCESS(status = WfsClearSessions(backend_new, false)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
