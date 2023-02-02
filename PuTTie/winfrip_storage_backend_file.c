/*
 * winfrip_storage_backend_file.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <windows.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_file.h"
#include "PuTTie/winfrip_rtl_pcre2.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_host_keys.h"
#include "PuTTie/winfrip_storage_jump_list.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"
#include "PuTTie/winfrip_storage_backend_file.h"

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
	WFSP_FILE_RMO_KEY		= 1,
	WFSP_FILE_RMO_VALUE_TYPE	= 2,
	WFSP_FILE_RMO_VALUE		= 3,
} WfspFileReMatchesOffset;

/*
 * Private variables
 */

/*
 * WfsppFileAppData: absolute pathnamne to the current user's application data directory (%APPDATA%)
 * WfsppFileDname: absolute pathname to the current user's PuTTie base directory
 * WfsppFileDnameHostKeys: absolute pathname to the current user's directory of PuTTie host key files
 * WfsppFileDnameSessions: absolute pathname to the current user's directory of PuTTie session files
 * WfsppFileExtHostKeys: file name extension of PuTTie host key files
 * WfsppFileExtSessions: file name extension of PuTTie session files
 * WfsppFileFnameJumpList: absolute pathname to the current user's PuTTie jump list file
 */

static char *		WfsppFileAppData = NULL;
static char		WfsppFileDname[MAX_PATH + 1] = "";
static char		WfsppFileDnameHostCAs[MAX_PATH + 1] = "";
static char		WfsppFileDnameHostKeys[MAX_PATH + 1] = "";
static char		WfsppFileDnameSessions[MAX_PATH + 1] = "";
static char		WfsppFileExtHostCAs[] = ".hostca";
static char		WfsppFileExtHostKeys[] = ".hostkey";
static char		WfsppFileExtSessions[] = ".ini";
static char		WfsppFileFnameJumpList[MAX_PATH + 1] = "";

static Wfp2Regex	WfsppFileRegex = {
	.ovecsize = 16,
	.spec_w = L"^([^=]+)=(int|string):(.*)$",
	.code = NULL,
	.error_message = {0}, .errorcode = 0, .erroroffset = 0,
	.md = NULL,
	.ovec = NULL,
};

/*
 * External subroutine prototypes
 */

/* [see windows/jump-list.c] */
void			update_jumplist(void);
void			clear_jumplist_PuTTY(void);

/*
 * Private subroutine prototypes
 */

static WfrStatus	WfsppFileGetJumpList(char **pjump_list, size_t *pjump_list_size);
static WfrStatus	WfsppFileInitAppDataSubdir(void);
static WfrStatus	WfsppFileInitRegex(void);
static WfrStatus	WfsppFileSetJumpList(const char *jump_list, size_t jump_list_size);
static WfrStatus	WfsppFileTransformJumpList(bool addfl, bool delfl, const char *const trans_item);

/*
 * Private subroutines
 */

static WfrStatus
WfsppFileGetJumpList(
	char **		pjump_list,
	size_t *	pjump_list_size
	)
{
	FILE *		file = NULL;
	char *		jump_list = NULL;
	size_t		jump_list_size = 0;
	struct stat	statbuf;
	WfrStatus	status;


	if (stat(WfsppFileFnameJumpList, &statbuf) < 0) {
		status = WFR_STATUS_FROM_ERRNO();
	} else if ((jump_list_size = statbuf.st_size) < 2) {
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);
	} else if (!(file = fopen(WfsppFileFnameJumpList, "rb"))
		|| !(jump_list = WFR_NEWN(jump_list_size = statbuf.st_size, char)))
	{
		status = WFR_STATUS_FROM_ERRNO();
	} else if (fread(jump_list, statbuf.st_size, 1, file) != 1) {
		status = WFR_STATUS_FROM_ERRNO();
		if (feof(file)) {
			status = WFR_STATUS_FROM_ERRNO1(ENODATA);
		}
	} else {
		jump_list[jump_list_size - 2] = '\0';
		jump_list[jump_list_size - 1] = '\0';

		*pjump_list = jump_list;
		if (pjump_list_size) {
			*pjump_list_size = jump_list_size;
		}

		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (file != NULL) {
		fclose(file);
	}

	if (WFR_STATUS_FAILURE(status)) {
		WFR_FREE_IF_NOTNULL(jump_list);
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
			WfsppFileDname, sizeof(WfsppFileDname),
			"%s/PuTTie", WfsppFileAppData);
		WFR_SNPRINTF(
			WfsppFileDnameHostCAs, sizeof(WfsppFileDnameHostCAs),
			"%s/hostcas", WfsppFileDname);
		WFR_SNPRINTF(
			WfsppFileDnameHostKeys, sizeof(WfsppFileDnameHostKeys),
			"%s/hostkeys", WfsppFileDname);
		WFR_SNPRINTF(
			WfsppFileDnameSessions, sizeof(WfsppFileDnameSessions),
			"%s/sessions", WfsppFileDname);
		WFR_SNPRINTF(
			WfsppFileFnameJumpList, sizeof(WfsppFileFnameJumpList),
			"%s/jump.list", WfsppFileDname);

		status = WFR_STATUS_CONDITION_SUCCESS;
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
WfsppFileSetJumpList(
	const char *	jump_list,
	size_t		jump_list_size
	)
{
	char *		dname_tmp;
	int		fd = -1;
	FILE *		file = NULL;
	char		fname_tmp[MAX_PATH + 1];
	ssize_t		nwritten;
	WfrStatus	status;


	if (!(dname_tmp = getenv("TEMP"))
	||  !(dname_tmp = getenv("TMP"))) {
		dname_tmp = "./";
	}
	WFR_SNPRINTF(fname_tmp, sizeof(fname_tmp), "%s/jump.list.XXXXXX", dname_tmp);

	if ((fd = mkstemp(fname_tmp)) < 0) {
		status = WFR_STATUS_FROM_ERRNO();
	} else if (!(file = fdopen(fd, "wb"))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else if (jump_list_size > 0) {
		if ((nwritten = fwrite(jump_list, jump_list_size, 1, file)) != 1) {
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

	if (WFR_STATUS_SUCCESS(status)
	&&  WFR_STATUS_FAILURE(status = WFR_STATUS_BIND_WINDOWS_BOOL(MoveFileEx(
			fname_tmp, WfsppFileFnameJumpList, MOVEFILE_REPLACE_EXISTING))))
	{
		(void)unlink(fname_tmp);
	} else if (WFR_STATUS_FAILURE(status)) {
		(void)unlink(fname_tmp);
	}

	return status;
}

static WfrStatus
WfsppFileTransformJumpList(
	bool			addfl,
	bool			delfl,
	const char *const	trans_item
	)
{
	char *		jump_list = NULL;
	size_t		jump_list_size;
	WfrStatus	status;


	if (addfl || delfl) {
		if (WFR_STATUS_FAILURE(status = WfsppFileGetJumpList(
				&jump_list, &jump_list_size)))
		{
			jump_list = NULL;
			jump_list_size = 0;
		}

		if (WFR_STATUS_SUCCESS(status = WfsTransformJumpList(
				addfl, delfl, &jump_list,
				&jump_list_size, trans_item)))
		{
			status = WfsppFileSetJumpList(jump_list, jump_list_size);
		}
	}

	WFR_FREE_IF_NOTNULL(jump_list);

	return status;
}

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfspFileCleanupHostCAs(
	WfsBackend	backend
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfspFileClearHostCAs(backend))) {
		status = WfrDeleteDirectory(WfsppFileDnameHostCAs, true, true);
	}

	return status;
}

WfrStatus
WfspFileClearHostCAs(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrDeleteFiles(WfsppFileDnameHostCAs, WfsppFileExtHostCAs);
}

WfrStatus
WfspFileCloseHostCA(
	WfsBackend	backend,
	WfsHostCA *	hca
	)
{
	(void)backend;
	(void)hca;

	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspFileDeleteHostCA(
	WfsBackend	backend,
	const char *	name
	)
{
	char		fname[MAX_PATH + 1];
	struct stat	statbuf;
	WfrStatus	status;


	(void)backend;

	if (WFR_STATUS_FAILURE(status = WfrEscapeFileName(
			WfsppFileDnameHostCAs,
			WfsppFileExtHostCAs, name,
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
WfspFileEnumerateHostCAs(
	WfsBackend	backend,
	bool		initfl,
	bool *		pdonefl,
	char **		pname,
	void *		state
	)
{
	WfrEnumerateFilesState *	enum_state;
	const char *			name;
	WfrStatus			status;


	(void)backend;

	if (initfl) {
		return WfrEnumerateFilesInit(
			WfsppFileDnameHostCAs,
			(WfrEnumerateFilesState **)state);
	}

	enum_state = (WfrEnumerateFilesState *)state;
	if (WFR_STATUS_SUCCESS(status = WfrEnumerateFiles(
			WfsppFileExtHostCAs, pdonefl, &name, &enum_state)))
	{
		if (!(*pdonefl)) {
			status = WfrUnescapeFileName((char *)name, (const char **)pname);
		}
	}

	return status;
}

WfrStatus
WfspFileLoadHostCA(
	WfsBackend	backend,
	const char *	name,
	WfsHostCA **	phca
	)
{
	enum WfspFileLHCABits {
		WFSP_FILE_LHCA_BIT_CA_PUBLIC_KEY	= 0,
		WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA1	= 1,
		WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA256	= 2,
		WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA512	= 3,
		WFSP_FILE_LHCA_BIT_VALIDITY_EXPRESSION	= 4,
	};

	enum WfspFileLHCABits	bits = 0;
	FILE *			file = NULL;
	char			fname[MAX_PATH];
	char *			fname_buf = NULL;
	WfsTreeItemType		item_type;
	char *			item_type_string;
	size_t			item_type_string_size;
	char *			key;
	size_t			key_size;
	char *			line, *line_sep;
	size_t			line_len;
	wchar_t *		line_w = NULL;
	int			nmatches;
	char *			p;
	WfsHostCA *		hca = NULL, hca_tmpl;
	struct stat		statbuf;
	WfrStatus		status;
	void *			value_new;
	size_t			value_new_size;


	if (WFR_STATUS_FAILURE(status = WfrEscapeFileName(
			WfsppFileDnameHostCAs,
			WfsppFileExtHostCAs, name,
			false, fname, sizeof(fname))))
	{
		return status;
	}

	WFS_HOST_CA_INIT(hca_tmpl);

	if ((stat(fname, &statbuf) < 0)
	||  (!(file = fopen(fname, "rb")))
	||  (!(fname_buf = WFR_NEWN(statbuf.st_size + 1, char))))
	{
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
		status = WfsGetHostCA(backend, true, name, &hca);
		if (WFR_STATUS_SUCCESS(status)
		&&  (hca->mtime == statbuf.st_mtime))
		{
			goto out;
		}

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

					if (WFR_STATUS_SUCCESS(status = Wfp2GetMatch(
							&WfsppFileRegex, true, WFSP_FILE_RMO_KEY,
							WFP2_RTYPE_STRING, line_w, &key,
							&key_size))
					&&  WFR_STATUS_SUCCESS(status = Wfp2GetMatch(
							&WfsppFileRegex, true, WFSP_FILE_RMO_VALUE_TYPE,
							WFP2_RTYPE_STRING, line_w, &item_type_string,
							&item_type_string_size)))
					{
						if (memcmp(item_type_string, "int", item_type_string_size) == 0) {
							item_type = WFS_TREE_ITYPE_INT;
							status = Wfp2GetMatch(
								&WfsppFileRegex, true, WFSP_FILE_RMO_VALUE,
								WFP2_RTYPE_INT, line_w,
								&value_new, &value_new_size);
						} else if (memcmp(item_type_string, "string", item_type_string_size) == 0) {
							item_type = WFS_TREE_ITYPE_STRING;
							status = Wfp2GetMatch(
								&WfsppFileRegex, true, WFSP_FILE_RMO_VALUE,
								WFP2_RTYPE_STRING, line_w,
								&value_new, &value_new_size);
						} else {
							status = WFR_STATUS_FROM_ERRNO1(EINVAL);
						}

						WFR_FREE(item_type_string);

						if (WFR_STATUS_SUCCESS(status)) {
							if ((strcmp(key, "PublicKey") == 0)
							&&  (item_type == WFS_TREE_ITYPE_STRING))
							{
								bits |= WFSP_FILE_LHCA_BIT_CA_PUBLIC_KEY;
								hca_tmpl.public_key = value_new;
							}
							else if ((strcmp(key, "PermitRSASHA1") == 0)
							      && (item_type == WFS_TREE_ITYPE_INT))
							{
								bits |= WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA1;
								hca_tmpl.permit_rsa_sha1 = *(int *)value_new;
								WFR_FREE(value_new);
							}
							else if ((strcmp(key, "PermitRSASHA256") == 0)
							      && (item_type == WFS_TREE_ITYPE_INT))
							{
								bits |= WFSP_FILE_LHCA_BIT_CA_PUBLIC_KEY;
								bits |= WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA256;
								hca_tmpl.permit_rsa_sha256 = *(int *)value_new;
								WFR_FREE(value_new);
							}
							else if ((strcmp(key, "PermitRSASHA512") == 0)
							      && (item_type == WFS_TREE_ITYPE_INT))
							{
								bits |= WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA512;
								hca_tmpl.permit_rsa_sha512 = *(int *)value_new;
								WFR_FREE(value_new);
							}
							else if ((strcmp(key, "Validity") == 0)
							      && (item_type == WFS_TREE_ITYPE_STRING))
							{
								bits |= WFSP_FILE_LHCA_BIT_VALIDITY_EXPRESSION;
								hca_tmpl.validity = value_new;
							}
							else {
								WFR_FREE(value_new);
								status = WFR_STATUS_FROM_ERRNO1(EINVAL);
							}
						} else {
							WFR_FREE(value_new);
						}

						WFR_FREE_IF_NOTNULL(key);
					}
				}

				WFR_FREE_IF_NOTNULL(line_w);
			}
		} while (WFR_STATUS_SUCCESS(status) && line_sep && (p < &fname_buf[statbuf.st_size]));

		if (WFR_STATUS_SUCCESS(status)) {
			if (bits !=
			    ( WFSP_FILE_LHCA_BIT_CA_PUBLIC_KEY
			    | WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA1
			    | WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA256
			    | WFSP_FILE_LHCA_BIT_PERMIT_RSA_SHA512
			    | WFSP_FILE_LHCA_BIT_VALIDITY_EXPRESSION))
			{
				status = WFR_STATUS_FROM_ERRNO1(EINVAL);
			} else if (hca) {
				if (hca->public_key) {
					WFR_FREE(hca_tmpl.public_key);
				}
				hca->public_key = hca_tmpl.public_key;
				hca->mtime = statbuf.st_mtime;
				hca->permit_rsa_sha1 = hca_tmpl.permit_rsa_sha1;
				hca->permit_rsa_sha256 = hca_tmpl.permit_rsa_sha256;
				hca->permit_rsa_sha512 = hca_tmpl.permit_rsa_sha512;
				if (hca->validity) {
					WFR_FREE(hca->validity);
				}
				hca->validity = hca_tmpl.validity;
			} else if (!hca) {
				status = WfsAddHostCA(
					backend, hca_tmpl.public_key,
					name, hca_tmpl.permit_rsa_sha1,
					hca_tmpl.permit_rsa_sha256,
					hca_tmpl.permit_rsa_sha512,
					hca_tmpl.validity,
					&hca);
			}
		}
	}

out:
	WFR_FREE_IF_NOTNULL(fname_buf);
	if (file) {
		(void)fclose(file);
	}

	if (WFR_STATUS_SUCCESS(status)) {
		if (phca) {
			*phca = hca;
		}
	} else {
		WFR_FREE_IF_NOTNULL(hca_tmpl.public_key);
		WFR_FREE_IF_NOTNULL(hca_tmpl.validity);
	}

	return status;
}

WfrStatus
WfspFileRenameHostCA(
	WfsBackend	backend,
	const char *	name,
	const char *	name_new
	)
{
	char		fname[MAX_PATH + 1], fname_new[MAX_PATH + 1];
	WfrStatus	status;


	(void)backend;

	if (WFR_STATUS_FAILURE(status = WfrEscapeFileName(
			WfsppFileDnameHostCAs,
			WfsppFileExtHostCAs, name,
			false, fname, sizeof(fname)))
	||  WFR_STATUS_FAILURE(status = WfrEscapeFileName(
			WfsppFileDnameHostCAs,
			WfsppFileExtHostCAs, name_new,
			false, fname_new, sizeof(fname_new))))
	{
		status = WFR_STATUS_BIND_WINDOWS_BOOL(MoveFileEx(
			fname, fname_new, MOVEFILE_REPLACE_EXISTING));
	}

	return status;
}

WfrStatus
WfspFileSaveHostCA(
	WfsBackend	backend,
	WfsHostCA *	hca
	)
{
	char *		dname_tmp;
	int		fd = -1;
	FILE *		file = NULL;
	char		fname[MAX_PATH], fname_tmp[MAX_PATH];
	int		rc;
	struct stat	statbuf;
	WfrStatus	status;


	(void)backend;

	if (stat(WfsppFileDnameHostCAs, &statbuf) < 0) {
		status = WFR_STATUS_FROM_ERRNO();
		if (WFR_STATUS_CONDITION(status) == ENOENT) {
			status = WfrMakeDirectory(WfsppFileDnameHostCAs, true);
		}
		if (WFR_STATUS_FAILURE(status)) {
			return status;
		}
	}

	if (!(dname_tmp = getenv("TEMP"))
	||  !(dname_tmp = getenv("TMP"))) {
		dname_tmp = "./";
	}

	if (WFR_STATUS_FAILURE(status = WfrEscapeFileName(
			WfsppFileDnameHostCAs,
			WfsppFileExtHostCAs, hca->name,
			false, fname, sizeof(fname)))
	||  WFR_STATUS_FAILURE(status = WfrEscapeFileName(
			dname_tmp, WfsppFileExtHostCAs, hca->name,
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

		if (WFR_STATUS_SUCCESS(status)) {
			rc = fprintf(
				file, "PublicKey=string:%s\r\n",
				hca->public_key);
			if (rc < 0) {
				status = WFR_STATUS_FROM_ERRNO1(rc);
			}
		}

		if (WFR_STATUS_SUCCESS(status)) {
			rc = fprintf(
				file, "PermitRSASHA1=int:%d\r\n",
				hca->permit_rsa_sha1);
			if (rc < 0) {
				status = WFR_STATUS_FROM_ERRNO1(rc);
			}
		}

		if (WFR_STATUS_SUCCESS(status)) {
			rc = fprintf(
				file, "PermitRSASHA256=int:%d\r\n",
				hca->permit_rsa_sha1);
			if (rc < 0) {
				status = WFR_STATUS_FROM_ERRNO1(rc);
			}
		}

		if (WFR_STATUS_SUCCESS(status)) {
			rc = fprintf(
				file, "PermitRSASHA512=int:%d\r\n",
				hca->permit_rsa_sha1);
			if (rc < 0) {
				status = WFR_STATUS_FROM_ERRNO1(rc);
			}
		}

		if (WFR_STATUS_SUCCESS(status)) {
			rc = fprintf(
				file, "Validity=string:%s\r\n",
				hca->validity);
			if (rc < 0) {
				status = WFR_STATUS_FROM_ERRNO1(rc);
			}
		}
	}

	if (file) {
		(void)fflush(file);
		(void)fclose(file);
	} else if ((fd >= 0)) {
		(void)close(fd);
	}

	if (WFR_STATUS_SUCCESS(status)
	&&  WFR_STATUS_FAILURE(status = WFR_STATUS_BIND_WINDOWS_BOOL(MoveFileEx(
			fname_tmp, fname, MOVEFILE_REPLACE_EXISTING))))
	{
		(void)unlink(fname_tmp);
	} else if (WFR_STATUS_FAILURE(status)) {
		(void)unlink(fname_tmp);
	}

	return status;
}


WfrStatus
WfspFileCleanupHostKeys(
	WfsBackend	backend
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfspFileClearHostKeys(backend))) {
		status = WfrDeleteDirectory(WfsppFileDnameHostKeys, true, true);
	}

	return status;
}

WfrStatus
WfspFileClearHostKeys(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrDeleteFiles(WfsppFileDnameHostKeys, WfsppFileExtHostKeys);
}

WfrStatus
WfspFileDeleteHostKey(
	WfsBackend	backend,
	const char *	key_name
	)
{
	char		fname[MAX_PATH];
	struct stat	statbuf;
	WfrStatus	status;


	(void)backend;

	if (WFR_STATUS_SUCCESS(status = WfrEscapeFileName(
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
	WfsBackend	backend,
	bool		initfl,
	bool *		pdonefl,
	char **		pkey_name,
	void *		state
	)
{
	WfrEnumerateFilesState *	enum_state;
	const char *			name;
	WfrStatus			status;


	(void)backend;

	if (initfl) {
		return WfrEnumerateFilesInit(
			WfsppFileDnameHostKeys,
			(WfrEnumerateFilesState **)state);
	}

	enum_state = (WfrEnumerateFilesState *)state;
	if (WFR_STATUS_SUCCESS(status = WfrEnumerateFiles(
			WfsppFileExtHostKeys, pdonefl, &name, &enum_state)))
	{
		if (!(*pdonefl)) {
			status = WfrUnescapeFileName((char *)name, (const char **)pkey_name);
		}
	}

	return status;
}

WfrStatus
WfspFileLoadHostKey(
	WfsBackend	backend,
	const char *	key_name,
	const char **	pkey
	)
{
	FILE *		file = NULL;
	char		fname[MAX_PATH + 1];
	char *		key = NULL;
	struct stat	statbuf;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfrEscapeFileName(
			WfsppFileDnameHostKeys,
			WfsppFileExtHostKeys, key_name,
			false, fname, sizeof(fname))))
	{
		if ((stat(fname, &statbuf) < 0)
		||  (!(file = fopen(fname, "rb")))
		||  (!(key = WFR_NEWN(statbuf.st_size + 1, char)))) {
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
		WFR_FREE(key);
	}

	return status;
}

WfrStatus
WfspFileRenameHostKey(
	WfsBackend	backend,
	const char *	key_name,
	const char *	key_name_new
	)
{
	char		fname[MAX_PATH + 1], fname_new[MAX_PATH + 1];
	WfrStatus	status;


	(void)backend;

	if (WFR_STATUS_SUCCESS(status = WfrEscapeFileName(
			WfsppFileDnameHostKeys,
			WfsppFileExtHostKeys, key_name,
			false, fname, sizeof(fname)))
	&&  WFR_STATUS_SUCCESS(status = WfrEscapeFileName(
			WfsppFileDnameHostKeys,
			WfsppFileExtHostKeys, key_name_new,
			false, fname_new, sizeof(fname_new))))
	{
		status = WFR_STATUS_BIND_WINDOWS_BOOL(MoveFileEx(
			fname, fname_new, MOVEFILE_REPLACE_EXISTING));
	}

	return status;
}

WfrStatus
WfspFileSaveHostKey(
	WfsBackend	backend,
	const char *	key_name,
	const char *	key
	)
{
	char *		dname_tmp;
	int		fd = -1;
	FILE *		file = NULL;
	char		fname[MAX_PATH + 1], fname_tmp[MAX_PATH + 1];
	size_t		key_len;
	size_t		nwritten;
	struct stat	statbuf;
	WfrStatus	status;


	(void)backend;

	if (stat(WfsppFileDnameHostKeys, &statbuf) < 0) {
		status = WFR_STATUS_FROM_ERRNO();
		if (WFR_STATUS_CONDITION(status) == ENOENT) {
			status = WfrMakeDirectory(WfsppFileDnameHostKeys, true);
		}
		if (WFR_STATUS_FAILURE(status)) {
			return status;
		}
	}

	if (!(dname_tmp = getenv("TEMP"))
	||  !(dname_tmp = getenv("TMP"))) {
		dname_tmp = "./";
	}

	if (WFR_STATUS_SUCCESS(status = WfrEscapeFileName(
			WfsppFileDnameHostKeys,
			WfsppFileExtHostKeys, key_name,
			false, fname, sizeof(fname)))
	&&  WFR_STATUS_SUCCESS(status = WfrEscapeFileName(
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

	if (WFR_STATUS_SUCCESS(status)
	&&  WFR_STATUS_FAILURE(status = WFR_STATUS_BIND_WINDOWS_BOOL(MoveFileEx(
			fname_tmp, fname, MOVEFILE_REPLACE_EXISTING))))
	{
		(void)unlink(fname_tmp);
	} else if (WFR_STATUS_FAILURE(status)) {
		(void)unlink(fname_tmp);
	}

	return status;
}


WfrStatus
WfspFileCleanupSessions(
	WfsBackend	backend
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfspFileClearSessions(backend))) {
		status = WfrDeleteDirectory(WfsppFileDnameSessions, true, true);
	}

	return status;
}

WfrStatus
WfspFileClearSessions(
	WfsBackend	backend
	)
{
	(void)backend;
	return WfrDeleteFiles(WfsppFileDnameSessions, WfsppFileExtSessions);
}

WfrStatus
WfspFileCloseSession(
	WfsBackend	backend,
	WfsSession *	session
	)
{
	(void)backend;
	(void)session;

	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspFileDeleteSession(
	WfsBackend	backend,
	const char *	sessionname
	)
{
	char		fname[MAX_PATH + 1];
	struct stat	statbuf;
	WfrStatus	status;


	(void)backend;

	if (WFR_STATUS_FAILURE(status = WfrEscapeFileName(
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
	WfrEnumerateFilesState *	enum_state;
	const char *			name;
	WfrStatus			status;


	(void)backend;

	if (initfl) {
		return WfrEnumerateFilesInit(
			WfsppFileDnameSessions,
			(WfrEnumerateFilesState **)state);
	}

	enum_state = (WfrEnumerateFilesState *)state;
	if (WFR_STATUS_SUCCESS(status = WfrEnumerateFiles(
			WfsppFileExtSessions, pdonefl, &name, &enum_state)))
	{
		if (!(*pdonefl)) {
			status = WfrUnescapeFileName((char *)name, (const char **)psessionname);
		}
	}

	return status;
}

WfrStatus
WfspFileLoadSession(
	WfsBackend	backend,
	const char *	sessionname,
	WfsSession **	psession
	)
{
	bool			addedfl = false;
	FILE *			file = NULL;
	char			fname[MAX_PATH];
	char *			fname_buf = NULL;
	WfsTreeItemType		item_type;
	char *			item_type_string;
	size_t			item_type_string_size;
	char *			key;
	size_t			key_size;
	char *			line, *line_sep;
	size_t			line_len;
	wchar_t *		line_w = NULL;
	int			nmatches;
	char *			p;
	WfsSession *		session;
	struct stat		statbuf;
	WfrStatus		status;
	void *			value_new;
	size_t			value_new_size;


	if (WFR_STATUS_FAILURE(status = WfrEscapeFileName(
			WfsppFileDnameSessions,
			WfsppFileExtSessions, sessionname,
			false, fname, sizeof(fname))))
	{
		return status;
	}

	if ((stat(fname, &statbuf) < 0)
	||  (!(file = fopen(fname, "rb")))
	||  (!(fname_buf = WFR_NEWN(statbuf.st_size + 1, char))))
	{
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

						if (WFR_STATUS_SUCCESS(status = Wfp2GetMatch(
								&WfsppFileRegex, true, WFSP_FILE_RMO_KEY,
								WFP2_RTYPE_STRING, line_w, &key,
								&key_size))
						&&  WFR_STATUS_SUCCESS(status = Wfp2GetMatch(
								&WfsppFileRegex, true, WFSP_FILE_RMO_VALUE_TYPE,
								WFP2_RTYPE_STRING, line_w, &item_type_string,
								&item_type_string_size)))
						{
							if (memcmp(item_type_string, "int", item_type_string_size) == 0) {
								item_type = WFS_TREE_ITYPE_INT;
								status = Wfp2GetMatch(
									&WfsppFileRegex, true, WFSP_FILE_RMO_VALUE,
									WFP2_RTYPE_INT, line_w,
									&value_new, &value_new_size);
							} else if (memcmp(item_type_string, "string", item_type_string_size) == 0) {
								item_type = WFS_TREE_ITYPE_STRING;
								status = Wfp2GetMatch(
									&WfsppFileRegex, true, WFSP_FILE_RMO_VALUE,
									WFP2_RTYPE_STRING, line_w,
									&value_new, &value_new_size);
							} else {
								status = WFR_STATUS_FROM_ERRNO1(EINVAL);
							}

							WFR_FREE(item_type_string);

							if (WFR_STATUS_SUCCESS(status)) {
								status = WfsSetSessionKey(
									session, key, value_new,
									value_new_size, item_type);
							} else {
								WFR_FREE(value_new);
							}

							WFR_FREE_IF_NOTNULL(key);
						}
					}

					WFR_FREE_IF_NOTNULL(line_w);
				}
			} while (WFR_STATUS_SUCCESS(status) && line_sep && (p < &fname_buf[statbuf.st_size]));
		}
	}

out:
	WFR_FREE_IF_NOTNULL(fname_buf);
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
	WfsBackend	backend,
	const char *	sessionname,
	const char *	sessionname_new
	)
{
	char		fname[MAX_PATH + 1], fname_new[MAX_PATH + 1];
	WfrStatus	status;


	(void)backend;

	if (WFR_STATUS_FAILURE(status = WfrEscapeFileName(
			WfsppFileDnameSessions,
			WfsppFileExtSessions, sessionname,
			false, fname, sizeof(fname)))
	||  WFR_STATUS_FAILURE(status = WfrEscapeFileName(
			WfsppFileDnameSessions,
			WfsppFileExtSessions, sessionname_new,
			false, fname_new, sizeof(fname_new))))
	{
		status = WFR_STATUS_BIND_WINDOWS_BOOL(MoveFileEx(
			fname, fname_new, MOVEFILE_REPLACE_EXISTING));
	}

	return status;
}

WfrStatus
WfspFileSaveSession(
	WfsBackend	backend,
	WfsSession *	session
	)
{
	char *		dname_tmp;
	int		fd = -1;
	FILE *		file = NULL;
	WfsTreeItem *	item;
	char		fname[MAX_PATH], fname_tmp[MAX_PATH];
	int		rc;
	struct stat	statbuf;
	WfrStatus	status;


	(void)backend;

	if (stat(WfsppFileDnameSessions, &statbuf) < 0) {
		status = WFR_STATUS_FROM_ERRNO();
		if (WFR_STATUS_CONDITION(status) == ENOENT) {
			status = WfrMakeDirectory(WfsppFileDnameSessions, true);
		}
		if (WFR_STATUS_FAILURE(status)) {
			return status;
		}
	}

	if (!(dname_tmp = getenv("TEMP"))
	||  !(dname_tmp = getenv("TMP"))) {
		dname_tmp = "./";
	}

	if (WFR_STATUS_FAILURE(status = WfrEscapeFileName(
			WfsppFileDnameSessions,
			WfsppFileExtSessions, session->name,
			false, fname, sizeof(fname)))
	||  WFR_STATUS_FAILURE(status = WfrEscapeFileName(
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
		WFS_TREE234_FOREACH(status, session->tree, idx, item) {
			switch (item->type) {
			default:
				rc = 0; break;

			case WFS_TREE_ITYPE_INT:
				rc = fprintf(
					file, "%s=int:%d\r\n",
					(char *)item->key, *((int *)item->value));
				break;

			case WFS_TREE_ITYPE_STRING:
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

	if (WFR_STATUS_SUCCESS(status)
	&&  WFR_STATUS_FAILURE(status = WFR_STATUS_BIND_WINDOWS_BOOL(MoveFileEx(
			fname_tmp, fname, MOVEFILE_REPLACE_EXISTING))))
	{
		(void)unlink(fname_tmp);
	} else if (WFR_STATUS_FAILURE(status)) {
		(void)unlink(fname_tmp);
	}

	return status;
}


void
WfspFileAddJumpList(
	const char *const	sessionname
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsppFileTransformJumpList(true, false, sessionname))) {
		update_jumplist();
	} else {
		/* Make sure we don't leave the jumplist dangling. */
		WfsClearJumpList(WfsGetBackend());
	}
}

WfrStatus
WfspFileCleanupJumpList(
	void
	)
{
	struct stat	statbuf;
	WfrStatus	status;


	if ((stat(WfsppFileFnameJumpList, &statbuf) < 0)) {
		status = WFR_STATUS_FROM_ERRNO();
		if (WFR_STATUS_CONDITION(status) == ENOENT) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	} else if (unlink(WfsppFileFnameJumpList) < 0) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

void
WfspFileClearJumpList(
	void
	)
{
	clear_jumplist_PuTTY();
}

WfrStatus
WfspFileGetEntriesJumpList(
	char **		pjump_list,
	size_t *	pjump_list_size
	)
{
	WfrStatus	status;


	status = WfsppFileGetJumpList(pjump_list, pjump_list_size);
	if (WFR_STATUS_FAILURE(status)) {
		if (WFR_STATUS_CONDITION(status) == ENOENT) {
			if (!((*pjump_list = WFR_NEWN(2, char)))) {
				status = WFR_STATUS_FROM_ERRNO();
			} else {
				(*pjump_list)[0] = '\0';
				(*pjump_list)[1] = '\0';
				*pjump_list_size = 2;
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		}
	}

	return status;
}

void
WfspFileRemoveJumpList(
	const char *const	sessionname
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsppFileTransformJumpList(false, true, sessionname))) {
		update_jumplist();
	} else {
		/* Make sure we don't leave the jumplist dangling. */
		WfsClearJumpList(WfsGetBackend());
	}
}

WfrStatus
WfspFileSetEntriesJumpList(
	const char *	jump_list,
	size_t		jump_list_size
	)
{
	return WfsppFileSetJumpList(jump_list, jump_list_size);
}


WfrStatus
WfspFileCleanupContainer(
	WfsBackend	backend
	)
{
	WfrStatus	status;


	(void)backend;
	status = WfrDeleteDirectory(WfsppFileDname, true, true);
	return status;
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
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
