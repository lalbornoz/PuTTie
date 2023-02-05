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
#include "PuTTie/winfrip_rtl_terminal.h"

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
		if (WFR_STATUS_FAILURE(status = WFR_RESIZE(
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
			if (WFR_STATUS_FAILURE(status = WfrGetTermLine(
					term, y, &line_new_w, &line_new_w_len)))
			{
				break;
			} else if (WFR_STATUS_FAILURE(status = WFR_RESIZE(
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

	if (WFR_STATUS_SUCCESS(status)) {
		*pline_w = line_w;
	} else {
		WFR_FREE_IF_NOTNULL(line_w);
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
