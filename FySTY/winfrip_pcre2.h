/*
 * winfrip_pcre2.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_PCRE2_H
#define PUTTY_WINFRIP_PCRE2_H

#define PCRE2_CODE_UNIT_WIDTH	16
#define PCRE2_STATIC			1
#include "../FySTY/pcre2@master/pcre2.h"

/*
 * Public enumeration and struct types
 */

typedef enum WinFrippP2MGReturn {
	WINFRIPP_P2MG_ERROR		= 0,
	WINFRIPP_P2MG_NO_MATCH	= 1,
	WINFRIPP_P2MG_CONTINUE	= 2,
	WINFRIPP_P2MG_DONE		= 3,
} WinFrippP2MGReturn;

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

/*
 * Public subroutines private to FySTY/winfrip*.c prototypes
 */

void winfripp_pcre2_init(WinFrippP2MGState *state, pcre2_code *code, size_t length, pcre2_match_data *md, wchar_t *subject);
WinFrippP2MGReturn winfripp_pcre2_match_global(WinFrippP2MGState *state, size_t *pbegin, size_t *pend);

#endif

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
