/*
 * winfrip_rtl_pcre2.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdio.h>
#include <string.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_pcre2.h"

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

WfrStatus
Wfp2GetMatch(
	Wfp2Regex *	regex,
	bool		alloc_value,
	int		match_offset,
	Wfp2RType	match_type,
	wchar_t *	subject,
	void *		pvalue,
	size_t *	pvalue_size
	)
{
	wchar_t			int_string_buf[sizeof("-2147483647")];
	const wchar_t *		match_begin, *match_end;
	size_t			match_size;
	WfrStatus		status;
	void *			value_new;
	size_t			value_new_size;


	match_offset *= 2;
	if (WFR_SUCCESS_ERRNO1(status, EINVAL,
		   (match_offset < regex->ovecsize)
		|| ((match_offset + 1) < regex->ovecsize))
	&&  WFR_SUCCESS_ERRNO1(status, ENOENT,
		   ((int)regex->ovec[match_offset] != -1)
		&& ((int)regex->ovec[match_offset + 1] != -1)))
	{
		match_begin = &subject[regex->ovec[match_offset]];
		match_end = &subject[regex->ovec[match_offset + 1]];
		match_size = match_end - match_begin;

		switch (match_type) {
		default:
			status = WFR_STATUS_FROM_ERRNO1(EINVAL);
			break;

		case WFP2_RTYPE_BOOL:
		case WFP2_RTYPE_SINT:
		case WFP2_RTYPE_UINT:
			if (match_size < (sizeof(int_string_buf) / sizeof(int_string_buf[0]))) {
				memcpy(int_string_buf, match_begin, match_size * sizeof(match_begin[0]));
				int_string_buf[match_size] = L'\0';

				switch (match_type) {
				default:
					break;

				case WFP2_RTYPE_BOOL:
					if (alloc_value && WFR_NEW(status, value_new, bool)) {
						*(bool *)value_new = wcstol(int_string_buf, NULL, 10);
						*(bool **)pvalue = value_new;
					} else if (!alloc_value) {
						*(bool *)pvalue = wcstol(int_string_buf, NULL, 10);
						status = WFR_STATUS_CONDITION_SUCCESS;
					}

					if (WFR_SUCCESS(status) && pvalue_size) {
						*pvalue_size = sizeof(bool);
					}
					break;

				case WFP2_RTYPE_SINT:
					if (alloc_value && WFR_NEW(status, value_new, signed int)) {
						*(signed int *)value_new = wcstol(int_string_buf, NULL, 10);
						*(signed int **)pvalue = value_new;
					} else if (!alloc_value) {
						*(signed int *)pvalue = wcstol(int_string_buf, NULL, 10);
						status = WFR_STATUS_CONDITION_SUCCESS;
					}

					if (WFR_SUCCESS(status) && pvalue_size) {
						*pvalue_size = sizeof(signed int);
					}
					break;

				case WFP2_RTYPE_UINT:
					if (alloc_value && WFR_NEW(status, value_new, unsigned int)) {
						*(unsigned int *)value_new = wcstoul(int_string_buf, NULL, 10);
						*(unsigned int **)pvalue = value_new;
					} else if (!alloc_value) {
						*(unsigned int *)pvalue = wcstoul(int_string_buf, NULL, 10);
						status = WFR_STATUS_CONDITION_SUCCESS;
					}

					if (WFR_SUCCESS(status) && pvalue_size) {
						*pvalue_size = sizeof(unsigned int);
					}
					break;
				}
			} else {
				status = WFR_STATUS_FROM_ERRNO1(EFAULT);
			}
			break;

		case WFP2_RTYPE_STRING:
			value_new_size = match_size + 1;
			if (WFR_SUCCESS_ERRNO1(status, EINVAL, (alloc_value))
			&&  WFR_NEWN(status, value_new, value_new_size + 1, char))
			{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat="
				snprintf(
					value_new, value_new_size, "%*.*S",
					(int)(match_end - match_begin),
					(int)(match_end - match_begin),
					match_begin);
#pragma GCC diagnostic pop
				*(char **)pvalue = value_new;
				if (pvalue_size) {
					*pvalue_size = value_new_size;
				}
			}
			break;
		}
	}

	return status;
}

WfrStatus
Wfp2MatchGlobal(
	Wfp2MGState *	state,
	size_t *	pbegin,
	size_t *	pend
	)
{
	uint32_t	options = 0;
	PCRE2_SIZE	startchar;
	WfrStatus	status = WFR_STATUS_PCRE2_ERROR;


	/*
	 * If the previous match was for an empty string, we are finished if we are
	 * at the end of the subject. Otherwise, arrange to run another match at the
	 * same point to see if a non-empty match can be found.
	 */

	if (!state->first_match) {
		if (state->ovector[0] == state->ovector[1]) {
			if (state->ovector[0] == state->length) {
				*pbegin = state->ovector[0], *pend = state->ovector[1];
				status = WFR_STATUS_PCRE2_DONE;
				goto out;					// Subject is empty string
			} else {
				options = PCRE2_NOTEMPTY_ATSTART | PCRE2_ANCHORED;
			}

	/*
	 * If the previous match was not an empty string, there is one tricky case to
	 * consider. If a pattern contains \K within a lookbehind assertion at the
	 * start, the end of the matched string can be at the offset where the match
	 * started. Without special action, this leads to a loop that keeps on matching
	 * the same substring. We must detect this case and arrange to move the start on
	 * by one character. The pcre2_get_startchar() function returns the starting
	 * offset that was passed to pcre2_match().
	 */

		} else {
			if (state->startoffset <= (startchar = pcre2_get_startchar(state->md))) {
				if (startchar >= state->length) {
					*pbegin = state->ovector[0], *pend = state->ovector[1];
					status = WFR_STATUS_PCRE2_DONE;
					goto out;				// Reached end of subject.
				} else {
					state->startoffset = startchar + 1;	// Advance by one character.
				}
			}
		}
	}

	/*
	 * Execute pcre2_match(), either starting a new iterated sequence of attempts
	 * to match or restarting from a/the previous attempt.
	 */

match:
	state->last_error =
		pcre2_match(
			state->code, state->subject, state->length,
			state->startoffset, options, state->md, NULL);
	if (state->first_match) {
		state->first_match = false;
	}

	switch (state->last_error) {
	case 0:
		state->startoffset = state->ovector[1];
		*pbegin = state->ovector[0], *pend = state->ovector[1];
		status = WFR_STATUS_PCRE2_CONTINUE;
		goto out;

	default:
		if (state->last_error == PCRE2_ERROR_NOMATCH) {
			if (options == 0) {
				state->last_error = 0;
				status = WFR_STATUS_PCRE2_NO_MATCH;
				goto out;
			} else {
				state->startoffset++;				// Advance by one character.
			if (state->crlf_is_newline				// If CRLF is a newline &
			&&  (state->startoffset < state->length - 1)		// we are at CRLF,
			&&  (state->subject[state->startoffset] == '\r')
			&&  (state->subject[state->startoffset + 1] == '\n'))
			{
				state->startoffset++;				// Advance by one more.
			}
			goto match;						// Go round the loop again
			}
		} else {
			status = WFR_STATUS_PCRE2_ERROR; goto out;
		}
		break;
	}

out:
	/*
	 * We must guard against patterns such as /(?=.\K)/ that use \K in an
	 * assertion to set the start of a match later than its end. In this
	 * demonstration program, we just detect this case and give up.
	 */

	if (state->ovector[0] > state->ovector[1]) {
		status = WFR_STATUS_PCRE2_ERROR;
	}

	return status;
}


void
Wfp2Init(
	Wfp2MGState *		state,
	pcre2_code *		code,
	size_t			length,
	pcre2_match_data *	md,
	wchar_t *		subject
	)
{
	uint32_t	newline = 0;


	state->code = code;
	state->first_match = true;
	state->last_error = 0;
	state->length = length;
	state->md = md;
	state->ovector = pcre2_get_ovector_pointer(state->md);
	state->startoffset = 0;
	state->subject = subject;

	memset(state->ovector, 0, pcre2_get_ovector_count(state->md) * 2 * sizeof(*state->ovector));

	(void)pcre2_pattern_info(state->code, PCRE2_INFO_NEWLINE, &newline);
	state->crlf_is_newline =
		   (newline == PCRE2_NEWLINE_ANY)
		|| (newline == PCRE2_NEWLINE_CRLF)
		|| (newline == PCRE2_NEWLINE_ANYCRLF);
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
