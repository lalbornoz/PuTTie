/*
 * winfrip_rtl_terminal.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_TERMINAL_H
#define PUTTY_WINFRIP_RTL_TERMINAL_H

#include "winfrip_rtl_pcre2.h"

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

WfrStatus	WfrGetTermLine(Terminal *term, int y, wchar_t **pline_w, size_t *pline_w_len);
WfrStatus	WfrGetTermLines(Terminal *term, pos begin, pos end, wchar_t **pline_w);
WfrStatus	WfrGetTermLinesURLW(Terminal *term, pos *pbegin, pos *pend, wchar_t **phover_url_w, size_t *phover_url_w_size, pos begin, pos end);
WfrStatus	WfrInitTermLinesURLWRegex(const wchar_t *spec_w, int *pre_errorcode, PCRE2_SIZE *pre_erroroffset);

#endif // !PUTTY_WINFRIP_RTL_TERMINAL_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
