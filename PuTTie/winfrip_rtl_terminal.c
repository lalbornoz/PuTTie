/*
 * winfrip_rtl_terminal.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "terminal.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_debug.h"
#include "PuTTie/winfrip_rtl_pcre2.h"
#include "PuTTie/winfrip_rtl_terminal.h"

/*
 * Private variables
 */

/*
 * PCRE2 regular expression compiled code and match data block
 */
static pcre2_code *		WfrpReCode = NULL;
static pcre2_match_data *	WfrpReMd = NULL;

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

WfrStatus
WfrGetTermLine(
	Terminal *	term,
	int		y,
	wchar_t **	pline_w,
	size_t *	pline_w_len
	)
{
	size_t		idx_in, idx_out;
	termline *	line = NULL;
	wchar_t *	line_w = NULL;
	size_t		line_w_len;
	size_t		rtl_idx, rtl_len;
	int		rtl_start;
	WfrStatus	status;
	wchar_t		wch;


	/*
	 * Reject invalid y coordinates falling outside of term->{cols,rows}. Fail given
	 * failure to allocate sufficient memory to the line_w buffer of term->cols + 1
	 * units of wchar_t.
	 */

	if (y >= term->rows) {
		return WFR_STATUS_FROM_ERRNO1(EINVAL);
	} else {
		line_w_len = term->cols;
		if (WFR_FAILURE(status = WFR_RESIZE(
				line_w, line_w_len, line_w_len + 1, wchar_t)))
		{
			return status;
		} else {
			line = term->disptext[y];
		}
	}

	/*
	 * Iteratively copy UTF-16 wide characters from the terminal's
	 * buffer of text on real screen into line_w. If a Right-To-Left
	 * character or continuous sequence thereof is encountered, it is
	 * iteratively copied in its entirety from right to left. If a
	 * direct-to-font mode character using the D800 hack is encountered,
	 * only its least significant 8 bits are copied.
	 */

	for (idx_in = idx_out = 0, rtl_len = 0, rtl_start = -1; idx_in < (size_t)term->cols; idx_in++) {
		wch = line->chars[idx_in].chr;

		switch (rtl_start) {
		/*
		 * Non-RTL character
		 */
		case -1:
			if (DIRECT_CHAR(wch) || DIRECT_FONT(wch) || !is_rtl(wch)) {
				if (DIRECT_CHAR(wch) || DIRECT_FONT(wch)) {
					wch &= 0xFF;
				}
				line_w[idx_out] = wch; idx_out++;
			} else if (is_rtl(wch)) {
				rtl_start = idx_in; rtl_len = 1;
			}
			break;

		/*
		 * RTL character or continuous sequence thereof
		 */
		default:
			if (DIRECT_CHAR(wch) || DIRECT_FONT(wch) || !is_rtl(wch)) {
				for (rtl_idx = 0; rtl_idx < rtl_len; rtl_idx++) {
					wch = line->chars[rtl_start + (rtl_len - 1 - rtl_idx)].chr;
					line_w[idx_out] = wch;
					idx_out++;
				}
				rtl_start = -1; rtl_len = 0;
				wch = line->chars[idx_in].chr;
				if (DIRECT_CHAR(wch) || DIRECT_FONT(wch)) {
					wch &= 0xFF;
				}
				line_w[idx_out] = wch; idx_out++;
			} else if (is_rtl(wch)) {
				rtl_len++;
			}
			break;
		}
	}

	/*
	 * Given a remnant of a continuous sequence of RTL characters,
	 * iteratively copy it from right to left.
	 */

	if (rtl_start != -1) {
		for (rtl_idx = 0; rtl_idx < rtl_len; rtl_idx++) {
			wch = line->chars[rtl_start + (rtl_len - 1 - rtl_idx)].chr;
			line_w[idx_out] = wch;
			idx_out++;
		}
		rtl_start = -1; rtl_len = 0;
	}

	/*
	 * NUL-terminate the line_w buffer, write out parameters, and return success.
	 */

	line_w[term->cols] = L'\0';
	*pline_w = line_w, *pline_w_len = line_w_len;

	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfrGetTermLines(
	Terminal *	term,
	pos		begin,
	pos		end,
	wchar_t **	pline_w
	)
{
	wchar_t *	line_new_w;
	size_t		line_new_w_len;
	wchar_t *	line_w = NULL;
	bool		line_w_initfl;
	size_t		line_w_size;
	WfrStatus	status;


	/*
	 * Reject invalid {begin,end}.{x,y} * coordinates, be they negative or
	 * falling outside of term->{cols,rows}
	 */

	if ((begin.x < 0) || (begin.x >= term->cols)
	||  (begin.y < 0) || (begin.y >= term->rows)
	||  (end.x < 0)   || (end.x >= term->cols)
	||  (end.y < 0)   || (end.y >= term->rows))
	{
		status = WFR_STATUS_FROM_ERRNO1(ERANGE);
	} else {
		/*
		 * Iteratively get and concatenate entire lines from the terminal's
		 * buffer of text on real screen specified by the coordinates {begin,end}.{x,y}.
		 */

		line_w_initfl = true;
		line_w_size = 0;
		status = WFR_STATUS_CONDITION_SUCCESS;

		for (int y = begin.y; y <= end.y; y++) {
			if (WFR_FAILURE(status = WfrGetTermLine(
					term, y, &line_new_w, &line_new_w_len)))
			{
				break;
			} else if (WFR_FAILURE(status = WFR_RESIZE(
					line_w, line_w_size,
					line_w_size ? (line_w_size + line_new_w_len) : (line_new_w_len + 1),
					wchar_t)))
			{
				WFR_FREE(line_new_w);
				break;
			} else {
				if (line_w_initfl) {
					wcscpy(line_w, line_new_w);
					line_w_initfl = false;
				} else {
					wcscat(line_w, line_new_w);
				}
				WFR_FREE(line_new_w);
			}
		}
	}

	if (WFR_SUCCESS(status)) {
		*pline_w = line_w;
	} else {
		WFR_FREE_IF_NOTNULL(line_w);
	}

	return status;
}

WfrStatus
WfrGetTermLinesURLW(
	Terminal *	term,
	pos *		pbegin,
	pos *		pend,
	wchar_t **	phover_url_w,
	size_t *	phover_url_w_size,
	pos		begin,
	pos		end
	)
{
	bool		donefl;
	wchar_t *	line_w = NULL;
	size_t		match_begin, match_end, match_len;
	Wfp2MGState	pcre2_state;
	WfrStatus	status;


	/*
	 * Fail given non-initialised pcre2 regular expression {code,match data
	 * block}, whether due to failure to initialise or an invalid regular
	 * expression provided by the user.
	 */

	if (!WfrpReCode || !WfrpReMd) {
		return WFR_STATUS_FROM_ERRNO1(EINVAL);
	} else if (WFR_SUCCESS(status = WfrGetTermLines(
			term, begin, end, &line_w)))
	{
		Wfp2Init(
			&pcre2_state, WfrpReCode,
			wcslen(line_w), WfrpReMd, line_w);
	}

	/*
	 * Iteratively attempt to match the regular expression and UTF-16 encoded
	 * terminal buffer string described by pcre2_state until either a non-zero
	 * length match comprising a range intersected by the x coordinate parameter
	 * is found or until either WFR_STATUS_CONDITION_PCRE2_ERROR,
	 * WFR_STATUS_CONDITION_PCRE2_NO_MATCH, or WFR_STATUS_CONDITION_PCRE2_DONE
	 * is returned.
	 */

	donefl = false;
	while (WFR_SUCCESS(status) && !donefl) {
		switch (WFR_STATUS_CONDITION(status = Wfp2MatchGlobal(
				&pcre2_state, &match_begin, &match_end)))
		{
		case WFR_STATUS_CONDITION_PCRE2_ERROR:
			WFR_DEBUGF("error %d trying to match any URL(s) in line `%S'", pcre2_state.last_error, line_w);
			break;
		case WFR_STATUS_CONDITION_PCRE2_NO_MATCH:
			break;

		/*
		 * Given a non-zero length match the range of which is intersected by {begin,end},
		 * attempt to duplicate the corresponding substring and return it, its size in bytes,
		 * and the beginning and end positions thereof. Given failure, return failure.
		 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
		case WFR_STATUS_CONDITION_PCRE2_DONE:
			donefl = true;
#pragma GCC diagnostic pop

		case WFR_STATUS_CONDITION_PCRE2_CONTINUE:
			if (((size_t)begin.x >= match_begin) && ((size_t)begin.x <= match_end)) {
				match_len = match_end - match_begin;
				if (match_len > 0) {
					WFR_DEBUGF("URL `%*.*S' matches regular expression", match_len, match_len, &line_w[match_begin]);
					if (WFR_SUCCESS_POSIX(status,
						(*phover_url_w = WfrWcsNDup(&line_w[match_begin], match_len))))
					{
						pbegin->x = match_begin;
						if (pbegin->x > term->cols) {
							pbegin->y = begin.y + (pbegin->x / term->cols);
							pbegin->x %= term->cols;
						} else {
							pbegin->y = begin.y;
						}

						pend->x = match_end;
						if (pend->x > term->cols) {
							pend->y = begin.y + (pend->x / term->cols);
							pend->x %= term->cols;
						} else {
							pend->y = end.y;
						}

						donefl = true;
						*phover_url_w_size = match_len + 1;
						status = WFR_STATUS_CONDITION_SUCCESS;
					}
				} else {
					status = WFR_STATUS_FROM_ERRNO1(EINVAL);
				}
			}
			break;
		}
	}

	/*
	 * Free line_w and implictly return failure.
	 */

	WFR_FREE_IF_NOTNULL(line_w);

	return status;
}

WfrStatus
WfrInitTermLinesURLWRegex(
	const wchar_t *		spec_w,
	int *			pre_errorcode,
	PCRE2_SIZE *		pre_erroroffset
	)
{
	if (WfrpReCode) {
		pcre2_code_free(WfrpReCode); WfrpReCode = NULL;
	}
	WfrpReCode = pcre2_compile(
		spec_w,
		PCRE2_ANCHORED | PCRE2_ZERO_TERMINATED,
		0, pre_errorcode, pre_erroroffset, NULL);

	if (WfrpReCode) {
		if ((WfrpReMd = pcre2_match_data_create(1, NULL))) {
			return WFR_STATUS_CONDITION_SUCCESS;
		} else {
			pcre2_code_free(WfrpReCode); WfrpReCode = NULL;
			return WFR_STATUS_FROM_ERRNO1(ENOMEM);
		}
	} else {
		return WFR_STATUS_FROM_ERRNO1(EINVAL);
	}
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
