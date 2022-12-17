/*
 * winfrip_rtl_putty.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "dialog.h"
#include "terminal.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_rtl.h"

#include <errno.h>

/*
 * Private type definitions
 */

typedef struct compressed_scrollback_line {
	size_t	len;
} compressed_scrollback_line;

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

WfrStatus
WfrpGetTermLine(
	Terminal *	term,
	wchar_t **	pline_w,
	size_t *	pline_w_len,
	int			y
	)
{
	size_t		idx_in, idx_out;
	termline *	line = NULL;
	wchar_t *	line_w;
	size_t		line_w_len;
	size_t		rtl_idx, rtl_len;
	int			rtl_start;
	wchar_t		wch;


	WFR_DEBUG_ASSERT(term);
	WFR_DEBUG_ASSERT(pline_w);
	WFR_DEBUG_ASSERT(pline_w_len);

	/*
	 * Reject invalid y coordinates falling outside of term->{cols,rows}. Fail given
	 * failure to allocate sufficient memory to the line_w buffer of term->cols + 1
	 * units of wchar_t.
	 */

	if (y >= term->rows) {
		WFR_DEBUG_FAIL();
		return WFR_STATUS_FROM_ERRNO1(EINVAL);
	} else {
		line_w_len = term->cols;
		if (!(line_w = sresize(NULL, line_w_len + 1, wchar_t))) {
			WFR_DEBUG_FAIL();
			return WFR_STATUS_FROM_ERRNO1(EINVAL);
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

bool
WfrpIsVKeyDown(
	int		nVirtKey
	)
{
	return (GetKeyState(nVirtKey) < 0);
}

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
