/*
 * winfrip_pcre2.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_PCRE2_H
#define PUTTY_WINFRIP_PCRE2_H

#define PCRE2_CODE_UNIT_WIDTH	16
#define PCRE2_STATIC			1
#include "../PuTTie/pcre2@master/pcre2.h"

/*
 * Public status condition constants and struct types private to PuTTie/winfrip*.c
 */

#define WFR_STATUS_CONDITION_PCRE2_ERROR		0
#define WFR_STATUS_CONDITION_PCRE2_NO_MATCH	1
#define WFR_STATUS_CONDITION_PCRE2_CONTINUE	2
#define WFR_STATUS_CONDITION_PCRE2_DONE		3

#define WFR_STATUS_PCRE2_ERROR					WFR_STATUS_FROM_PCRE2(WFR_STATUS_CONDITION_PCRE2_ERROR, WFR_STATUS_SEVERITY_ERROR)
#define WFR_STATUS_PCRE2_NO_MATCH				WFR_STATUS_FROM_PCRE2(WFR_STATUS_CONDITION_PCRE2_NO_MATCH, WFR_STATUS_SEVERITY_ERROR)
#define WFR_STATUS_PCRE2_CONTINUE				WFR_STATUS_FROM_PCRE2(WFR_STATUS_CONDITION_PCRE2_CONTINUE, WFR_STATUS_SEVERITY_SUCCESS)
#define WFR_STATUS_PCRE2_DONE					WFR_STATUS_FROM_PCRE2(WFR_STATUS_CONDITION_PCRE2_DONE, WFR_STATUS_SEVERITY_SUCCESS)

typedef struct WinFrippP2MGState {
	pcre2_code *		code;
	bool				crlf_is_newline;
	bool				first_match;
	int					last_error;
	size_t				length;
	pcre2_match_data *	md;
	PCRE2_SIZE *		ovector;
	PCRE2_SIZE			startoffset;
	wchar_t *			subject;
} WinFrippP2MGState;

typedef struct WinfrippP2Regex {
	int					ovecsize;
	const wchar_t *		spec_w;

	pcre2_code *		code;
	wchar_t				error_message[256];
	int					errorcode;
	PCRE2_SIZE			erroroffset;
	pcre2_match_data *	md;
	size_t *			ovec;
} WinfrippP2Regex;

typedef enum WinfrippP2RType {
	WINFRIPP_P2RTYPE_BOOL	= 0,
	WINFRIPP_P2RTYPE_INT	= 1,
	WINFRIPP_P2RTYPE_SINT	= WINFRIPP_P2RTYPE_INT,
	WINFRIPP_P2RTYPE_STRING	= 2,
	WINFRIPP_P2RTYPE_UINT	= 3,
} WinfrippP2RType;

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

void winfripp_pcre2_init(WinFrippP2MGState *state, pcre2_code *code, size_t length, pcre2_match_data *md, wchar_t *subject);

WfrStatus winfripp_pcre2_get_match(WinfrippP2Regex *regex, bool alloc_value, int match_offset, WinfrippP2RType match_type, wchar_t *subject, void *pvalue, size_t *pvalue_size);
WfrStatus winfripp_pcre2_match_global(WinFrippP2MGState *state, size_t *pbegin, size_t *pend);

#endif // !PUTTY_WINFRIP_PCRE2_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
