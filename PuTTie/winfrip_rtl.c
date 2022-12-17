/*
 * winfrip_rtl.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdio.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#pragma GCC diagnostic pop

#include <windows.h>

#include "PuTTie/winfrip_rtl.h"

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

wchar_t *
WfrWcsNDup(
	const wchar_t *		in_w,
	size_t				in_w_len
	)
{
	wchar_t *	out_w;
	size_t		out_w_size;


	WFR_DEBUG_ASSERT(in_w);
	WFR_DEBUG_ASSERT(in_w_len > 0);

	out_w_size = (in_w_len + 1) * sizeof(*out_w);
	out_w = snewn(out_w_size, wchar_t);
	ZeroMemory(out_w, out_w_size);
	wcsncpy(out_w, in_w, in_w_len);
	return out_w;
}

WfrStatus
WfrToWcsDup(
	char *		in,
	size_t		in_size,
	wchar_t **	pout_w
	)
{
	wchar_t *	out_w;
	int			out_w_len, out_w_size;
	WfrStatus	status;


	WFR_DEBUG_ASSERT(in);
	WFR_DEBUG_ASSERT(in_size > 0);
	WFR_DEBUG_ASSERT(pout_w);

	out_w_len = MultiByteToWideChar(CP_ACP, 0, in, in_size, NULL, 0);
	WFR_DEBUG_ASSERT(out_w_len > 0);
	if (out_w_len > 0) {
		out_w_size = out_w_len * sizeof(*out_w);
		out_w = snewn(out_w_size, wchar_t);
		ZeroMemory(out_w, out_w_size);
		if (MultiByteToWideChar(CP_ACP, 0, in, in_size, out_w, out_w_size) == out_w_len) {
			*pout_w = out_w;
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else {
			status = WFR_STATUS_FROM_WINDOWS();
		}
	} else {
		status = WFR_STATUS_FROM_WINDOWS();
	}

	return status;
}

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
