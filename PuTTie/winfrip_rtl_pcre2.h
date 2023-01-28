/*
 * winfrip_rtl_pcre2.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_PCRE2_H
#define PUTTY_WINFRIP_PCRE2_H

#define PCRE2_CODE_UNIT_WIDTH			16
#define PCRE2_STATIC				1
#include "../PuTTie/pcre2@master/pcre2.h"

/*
 * Public status condition constants and struct types private to PuTTie/winfrip*.c
 */

#define WFR_STATUS_CONDITION_PCRE2_ERROR	0
#define WFR_STATUS_CONDITION_PCRE2_NO_MATCH	1
#define WFR_STATUS_CONDITION_PCRE2_CONTINUE	2
#define WFR_STATUS_CONDITION_PCRE2_DONE		3

#define WFR_STATUS_PCRE2_ERROR			WFR_STATUS_FROM_PCRE2(WFR_STATUS_CONDITION_PCRE2_ERROR, WFR_STATUS_SEVERITY_ERROR)
#define WFR_STATUS_PCRE2_NO_MATCH		WFR_STATUS_FROM_PCRE2(WFR_STATUS_CONDITION_PCRE2_NO_MATCH, WFR_STATUS_SEVERITY_ERROR)
#define WFR_STATUS_PCRE2_CONTINUE		WFR_STATUS_FROM_PCRE2(WFR_STATUS_CONDITION_PCRE2_CONTINUE, WFR_STATUS_SEVERITY_SUCCESS)
#define WFR_STATUS_PCRE2_DONE			WFR_STATUS_FROM_PCRE2(WFR_STATUS_CONDITION_PCRE2_DONE, WFR_STATUS_SEVERITY_SUCCESS)

typedef struct Wfp2MGState {
	pcre2_code *		code;
	bool			crlf_is_newline;
	bool			first_match;
	int			last_error;
	size_t			length;
	pcre2_match_data *	md;
	PCRE2_SIZE *		ovector;
	PCRE2_SIZE		startoffset;
	wchar_t *		subject;
} Wfp2MGState;

typedef struct Wfp2Regex {
	int			ovecsize;
	const wchar_t *		spec_w;

	pcre2_code *		code;
	wchar_t			error_message[256];
	int			errorcode;
	PCRE2_SIZE		erroroffset;
	pcre2_match_data *	md;
	size_t *		ovec;
} Wfp2Regex;

typedef enum Wfp2RType {
	WFP2_RTYPE_BOOL		= 0,
	WFP2_RTYPE_INT		= 1,
	WFP2_RTYPE_SINT		= WFP2_RTYPE_INT,
	WFP2_RTYPE_STRING	= 2,
	WFP2_RTYPE_UINT		= 3,
} Wfp2RType;

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

WfrStatus	Wfp2GetMatch(Wfp2Regex *regex, bool alloc_value, int match_offset, Wfp2RType match_type, wchar_t *subject, void *pvalue, size_t *pvalue_size);
WfrStatus	Wfp2MatchGlobal(Wfp2MGState *state, size_t *pbegin, size_t *pend);

void		Wfp2Init(Wfp2MGState *state, pcre2_code *code, size_t length, pcre2_match_data *md, wchar_t *subject);

#endif // !PUTTY_WINFRIP_PCRE2_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
