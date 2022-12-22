/*
 * winfrip_rtl.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_H
#define PUTTY_WINFRIP_RTL_H

#include "winfrip_rtl_status.h"
#include "winfrip_rtl_debug.h"

/*
 * Public macros private to PuTTie/winfrip*.c
 */

#define WFR_SFREE_IF_NOTNULL(p)									\
		if ((p) != NULL) {										\
			sfree((p));											\
		}

#define WFR_SNPRINTF(s, n, fmt, ...) ({							\
		int		c = snprintf((s), (n), (fmt), ## __VA_ARGS__);	\
		((char *)(s))[(n) - 1] = '\0';							\
		c;														\
	})

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

WfrStatus WfrToWcsDup(char *in, size_t in_size, wchar_t **pout_w);
wchar_t *WfrWcsNDup(const wchar_t *in_w, size_t in_w_len);

#endif // !PUTTY_WINFRIP_RTL_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
