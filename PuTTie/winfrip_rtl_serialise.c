/*
 * winfrip_rtl_serialise.c - pointless frippery & tremendous amounts of bloat
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
#include "PuTTie/winfrip_rtl_serialise.h"
#include "PuTTie/winfrip_rtl_pcre2.h"

/*
 * Private types
 */

/*
 * Offsets of regular expression capture groups used to match fields
 * of interest in files during parsing thereof
 */

typedef enum WfrpReMatchesOffset {
	WFRP_RMO_KEY		= 1,
	WFRP_RMO_VALUE_TYPE	= 2,
	WFRP_RMO_VALUE		= 3,
} WfrpReMatchesOffset;

/*
 * Private variables
 */

/*
 * Regular expression including ancillary data used to parse files
 */

static Wfp2Regex	WfrpRegex = {
	.ovecsize	= 8,
	.spec_w		= L"^([^=]+)=(int|string):(.*)$",
	.code		= NULL,
	.error_message	= {0},
	.errorcode	= 0,
	.erroroffset	= 0,
	.md		= NULL,
	.ovec		= NULL,
};
static bool		WfrpRegexInitialised = false;

/*
 * Private subroutine prototypes
 */

static WfrStatus	WfrpInitRegex(void);

/*
 * Private subroutines
 */

static WfrStatus
WfrpInitRegex(
	void
	)
{
	WfrStatus	status;


	if (!WfrpRegexInitialised) {
		if (WfrpRegex.code) {
			pcre2_code_free(WfrpRegex.code); WfrpRegex.code = NULL;
		}
		WfrpRegex.code = pcre2_compile(
			WfrpRegex.spec_w, PCRE2_ANCHORED | PCRE2_ZERO_TERMINATED,
			0, &WfrpRegex.errorcode, &WfrpRegex.erroroffset, NULL);

		if (!WfrpRegex.code) {
			(void)pcre2_get_error_message(
				WfrpRegex.errorcode,
				WfrpRegex.error_message,
				WFR_SIZEOF_WSTRING(WfrpRegex.error_message));
			status = WFR_STATUS_FROM_PCRE2(WfrpRegex.errorcode, WFR_STATUS_SEVERITY_ERROR);
		} else if (!(WfrpRegex.md = pcre2_match_data_create(WfrpRegex.ovecsize, NULL))) {
			pcre2_code_free(WfrpRegex.code); WfrpRegex.code = NULL;
			status = WFR_STATUS_FROM_ERRNO1(ENOMEM);
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
			WfrpRegexInitialised = true;
		}
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

WfrStatus
WfrLoadParse(
	char *			data,
	size_t			data_size,
	WfrLoadParseItemFn	item_fn,
	void *			param1,
	void *			param2
	)
{
	char *			key;
	size_t			key_size;
	char *			line;
	size_t			line_len;
	char *			line_sep;
	wchar_t *		line_w = NULL;
	int			nmatches;
	int			nmatches_count;
	char *			p;
	WfrStatus		status;
	Wfp2RType		type;
	char *			type_string;
	size_t			type_string_size;
	void *			value;
	size_t			value_size;


	if (data_size == 0) {
		return WFR_STATUS_FROM_ERRNO1(EINVAL);
	} else if (WFR_STATUS_FAILURE(status = WfrpInitRegex())) {
		return status;
	}

	nmatches_count = WfrpRegex.ovecsize / 2;
	p = &data[0];
	WfrpRegex.ovec = pcre2_get_ovector_pointer(WfrpRegex.md);
	memset(
		WfrpRegex.ovec, 0,
		pcre2_get_ovector_count(WfrpRegex.md) * 2 * sizeof(*(WfrpRegex.ovec)));

	do {
		line = p; line_sep = strchr(line, '\n');
		if (line_sep) {
			*line_sep = L'\0', p = line_sep + 1;
			if ((line_sep[-1] == L'\r')
			&&  (&line_sep[-1] >= line)) {
				line_sep[-1] = L'\0';
			}
		}

		if ((line[0] != L'#')
		&&  ((line_len = strlen(line)) > 0))
		{
			nmatches = 0;

			if (WFR_STATUS_SUCCESS(status = WfrToWcsDup(line, line_len + 1, &line_w))
			&&  ((nmatches = pcre2_match(
					WfrpRegex.code, line_w, line_len,
					0, 0, WfrpRegex.md, NULL)) == nmatches_count))
			{
				key = NULL; key_size = 0;
				type_string = NULL; type_string_size = 0;
				value = NULL; value_size = 0;

				if (WFR_STATUS_SUCCESS(status = Wfp2GetMatch(
						&WfrpRegex, true, WFRP_RMO_KEY,
						WFP2_RTYPE_STRING, line_w, &key,
						&key_size))
				&&  WFR_STATUS_SUCCESS(status = Wfp2GetMatch(
						&WfrpRegex, true, WFRP_RMO_VALUE_TYPE,
						WFP2_RTYPE_STRING, line_w, &type_string,
						&type_string_size)))
				{
					if (memcmp(type_string, "int", type_string_size) == 0) {
						type = WFP2_RTYPE_INT;
						status = Wfp2GetMatch(
							&WfrpRegex, true, WFRP_RMO_VALUE, type,
							line_w, &value, &value_size);
					} else if (memcmp(type_string, "string", type_string_size) == 0) {
						type = WFP2_RTYPE_STRING;
						status = Wfp2GetMatch(
							&WfrpRegex, true, WFRP_RMO_VALUE, type,
							line_w, &value, &value_size);
					} else {
						status = WFR_STATUS_FROM_ERRNO1(EINVAL);
					}

					WFR_FREE(type_string);

					if (WFR_STATUS_SUCCESS(status)) {
						status = item_fn(param1, param2, key, type, value, value_size);
					}

					WFR_FREE_IF_NOTNULL(key);
				}
			}

			if (nmatches < 0) {
				status = WFR_STATUS_FROM_PCRE2(nmatches, WFR_STATUS_SEVERITY_ERROR);
			} else if (nmatches != nmatches_count) {
				status = WFR_STATUS_FROM_ERRNO1(EINVAL);
			}

			WFR_FREE_IF_NOTNULL(line_w);
		}
	} while (WFR_STATUS_SUCCESS(status)
	      && line_sep
	      && (p < &data[data_size]));

	return status;
}

WfrStatus
WfrMapPcre2TypeToTreeType(
	Wfp2RType		pcre2_type,
	WfrTreeItemTypeBase *	ptree_item_type
	)
{
	switch (pcre2_type) {
	default:
		return WFR_STATUS_FROM_ERRNO1(EINVAL);
	case WFP2_RTYPE_INT:
		*ptree_item_type = WFR_TREE_ITYPE_INT;
		return WFR_STATUS_CONDITION_SUCCESS;
	case WFP2_RTYPE_STRING:
		*ptree_item_type = WFR_TREE_ITYPE_STRING;
		return WFR_STATUS_CONDITION_SUCCESS;
	}
}

WfrStatus
WfrSaveToFileV(
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


	if (stat(dname, &statbuf) < 0) {
		status = WFR_STATUS_FROM_ERRNO();
		if (WFR_STATUS_CONDITION(status) == ENOENT) {
			status = WfrMakeDirectory(dname, true);
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
			dname, ext, fname, false, fname_full, sizeof(fname_full)))
	||  WFR_STATUS_FAILURE(status = WfrEscapeFileName(
			dname_tmp, ext, fname, true, fname_tmp, sizeof(fname_tmp))))
	{
		return status;
	}

	if ((fd = mkstemp(fname_tmp)) < 0) {
		status = WFR_STATUS_FROM_ERRNO();
	} else if (!(file = fdopen(fd, "wb"))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;

		va_start(ap, fname);
		while (WFR_STATUS_SUCCESS(status)) {
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
				rc = fprintf(file, "%s=int:%d\r\n", key, *(int *)value);
				if (rc < 0) {
					status = WFR_STATUS_FROM_ERRNO1(rc);
				}
				break;

			case WFR_TREE_ITYPE_STRING:
				rc = fprintf(file, "%s=string:%s\r\n", key, (const char *)value);
				if (rc < 0) {
					status = WFR_STATUS_FROM_ERRNO1(rc);
				}
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

	if (WFR_STATUS_SUCCESS(status)
	&&  WFR_STATUS_FAILURE(status = WFR_STATUS_BIND_WINDOWS_BOOL(MoveFileEx(
			fname_tmp, fname_full, MOVEFILE_REPLACE_EXISTING))))
	{
		(void)unlink(fname_tmp);
	} else if (WFR_STATUS_FAILURE(status)) {
		(void)unlink(fname_tmp);
	}

	return status;
}

WfrStatus
WfrSaveTreeToFile(
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


	if (stat(dname, &statbuf) < 0) {
		status = WFR_STATUS_FROM_ERRNO();
		if (WFR_STATUS_CONDITION(status) == ENOENT) {
			status = WfrMakeDirectory(dname, true);
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
			dname, ext, fname, false, fname_full, sizeof(fname_full)))
	||  WFR_STATUS_FAILURE(status = WfrEscapeFileName(
			dname_tmp, ext, fname, true, fname_tmp, sizeof(fname_tmp))))
	{
		return status;
	}

	if ((fd = mkstemp(fname_tmp)) < 0) {
		status = WFR_STATUS_FROM_ERRNO();
	} else if (!(file = fdopen(fd, "wb"))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
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

	if (WFR_STATUS_SUCCESS(status)
	&&  WFR_STATUS_FAILURE(status = WFR_STATUS_BIND_WINDOWS_BOOL(MoveFileEx(
			fname_tmp, fname_full, MOVEFILE_REPLACE_EXISTING))))
	{
		(void)unlink(fname_tmp);
	} else if (WFR_STATUS_FAILURE(status)) {
		(void)unlink(fname_tmp);
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
