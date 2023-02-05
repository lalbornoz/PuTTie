/*
 * winfrip_rtl_load.c - pointless frippery & tremendous amounts of bloat
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
#include "PuTTie/winfrip_rtl_load.h"
#include "PuTTie/winfrip_rtl_pcre2.h"
#include "PuTTie/winfrip_rtl_tree.h"

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
static WfrStatus	WfrpMapPcre2TypeToTreeType(Wfp2RType pcre2_type, WfrTreeItemTypeBase *ptree_item_type);

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

static WfrStatus
WfrpMapPcre2TypeToTreeType(
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

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

WfrStatus
WfrLoadListFromFile(
	const char *	fname,
	char **		plist,
	size_t *	plist_size
	)
{
	FILE *		file = NULL;
	char *		list = NULL;
	size_t		list_size = 0;
	struct stat	statbuf;
	WfrStatus	status;


	if (stat(fname, &statbuf) < 0) {
		status = WFR_STATUS_FROM_ERRNO();
	} else if ((list_size = statbuf.st_size) < 2) {
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);
	} else if (!(file = fopen(fname, "rb"))
		|| !(list = WFR_NEWN(list_size = statbuf.st_size, char)))
	{
		status = WFR_STATUS_FROM_ERRNO();
	} else if (fread(list, statbuf.st_size, 1, file) != 1) {
		status = WFR_STATUS_FROM_ERRNO();
		if (feof(file)) {
			status = WFR_STATUS_FROM_ERRNO1(ENODATA);
		}
	} else {
		list[list_size - 2] = '\0';
		list[list_size - 1] = '\0';

		*plist = list;
		if (plist_size) {
			*plist_size = list_size;
		}

		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (file != NULL) {
		fclose(file);
	}

	if (WFR_STATUS_FAILURE(status)) {
		WFR_FREE_IF_NOTNULL(list);
	}

	return status;
}

WfrStatus
WfrLoadParse(
	char *			data,
	size_t			data_size,
	void *			param1,
	void *			param2,
	WfrLoadParseItemFn	item_fn
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
	WfrTreeItemTypeBase	tree_item_type;
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

					if (WFR_STATUS_SUCCESS(status)
					&&  WFR_STATUS_SUCCESS(WfrpMapPcre2TypeToTreeType(type, &tree_item_type)))
					{
						status = item_fn(param1, param2, key, tree_item_type, value, value_size);
					}

					WFR_FREE(key);
					if (WFR_STATUS_FAILURE(status)) {
						WFR_FREE_IF_NOTNULL(value);
					}
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
WfrLoadRawFile(
	bool		escape_fnamefl,
	const char *	dname,
	const char *	ext,
	const char *	fname,
	char **		pdata,
	size_t *	pdata_size,
	time_t *	pmtime
	)
{
	FILE *		file = NULL;
	char *		data = NULL;
	size_t		data_size;
	char		pname[MAX_PATH + 1];
	struct stat	statbuf;
	WfrStatus	status;


	if (escape_fnamefl) {
		status = WfrEscapeFileName(
			dname, ext, fname,
			false, pname, sizeof(pname));
	} else {
		WFR_SNPRINTF_PNAME(pname, sizeof(pname), dname, ext, fname);
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_SUCCESS(status)) {
		if ((stat(pname, &statbuf) < 0)
		||  (!(file = fopen(pname, "rb")))
		||  (!(data = WFR_NEWN(data_size = (statbuf.st_size + 1), char)))) {
			status = WFR_STATUS_FROM_ERRNO();
		} else if (fread(data, statbuf.st_size, 1, file) != 1) {
			if (statbuf.st_size == 0) {
				status = WFR_STATUS_FROM_ERRNO1(ENODATA);
			} else {
				status = WFR_STATUS_FROM_ERRNO();
				if (feof(file)) {
					status = WFR_STATUS_FROM_ERRNO1(ENODATA);
				}
			}
		} else {
			data[statbuf.st_size] = '\0';
		}

		if (file) {
			(void)fclose(file);
		}
	}

	if (WFR_STATUS_SUCCESS(status)) {
		*pdata = data;
		if (pdata_size) {
			*pdata_size = data_size;
		}
		if (pmtime) {
			*pmtime = statbuf.st_mtime;
		}
	} else {
		WFR_FREE(data);
	}

	return status;
}

WfrStatus
WfrLoadRegSubKey(
	const char *		key_name,
	const char *		subkey,
	void *			param1,
	void *			param2,
	WfrLoadRegSubKeyItemFn	item_fn
	)
{
	bool			donefl;
	WfrEnumerateRegState *	enum_state;
	char *			name;
	WfrStatus		status;
	char *			subkey_escaped = NULL;
	WfrTreeItemTypeBase	type;
	DWORD			type_registry;
	void *			value = NULL;
	size_t			value_len;


	if (WFR_STATUS_SUCCESS(status = WfrEscapeRegKey(subkey, &subkey_escaped))
	&&  WFR_STATUS_SUCCESS(status = WfrEnumerateRegInit(
			&enum_state, key_name, subkey_escaped, NULL)))
	{
		donefl = false;

		while (WFR_STATUS_SUCCESS(status) && !donefl) {
			if (WFR_STATUS_SUCCESS(status = WfrEnumerateRegValues(
					&donefl, &value, &value_len,
					&name, &type_registry, &enum_state)) && !donefl)
			{
				switch (type_registry) {
				default:
					status = WFR_STATUS_FROM_ERRNO1(EINVAL); break;
				case REG_DWORD:
					type = WFR_TREE_ITYPE_INT; break;
				case REG_SZ:
					type = WFR_TREE_ITYPE_STRING; break;
				}

				if (WFR_STATUS_SUCCESS(status)) {
					status = item_fn(param1, param2, name, type, value, value_len);
				}

				WFR_FREE(name);
				if (WFR_STATUS_FAILURE(status)) {
					WFR_FREE(value);
				}
			}
		}
	}

	WFR_FREE_IF_NOTNULL(subkey_escaped);

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
