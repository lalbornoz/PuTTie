/*
 * winfrip_rtl.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_H
#define PUTTY_WINFRIP_RTL_H

#include "winfrip_rtl_status.h"
#include "winfrip_rtl_debug.h"

/*
 * Public macros private to PuTTie/winfrip*.c
 */

#define WFR_IF_STATUS_FAILURE_MESSAGEBOX(context, status)						\
		WFR_IF_STATUS_FAILURE_MESSAGEBOX1(context, status, "PuTTie")
#define WFR_IF_STATUS_FAILURE_MESSAGEBOX1(context, status, title)				\
		if (WFR_STATUS_FAILURE((status))) {										\
			MessageBox(															\
				NULL, WfrStatusToErrorMessage((context), (status)), (title),	\
				MB_ICONERROR | MB_OK | MB_DEFBUTTON1);							\
		}

#define WFR_INTCMP(i1, i2)														\
		(((i1) < (i2)) ? -1 : ((i1) > (i2)) ? 1 : 0)

#define WFR_SFREE_IF_NOTNULL(p)													\
		if ((p) != NULL) {														\
			sfree((p));															\
		}

#define WFR_SNPRINTF(s, n, fmt, ...) ({											\
		int		c = snprintf((s), (n), (fmt), ## __VA_ARGS__);					\
		((char *)(s))[(n) - 1] = '\0';											\
		c;																		\
	})

#define WFR_SRESIZE_IF_NEQ_SIZE(p, size, size_new, type) ({						\
		typeof (p)	_p;															\
		WfrStatus	status;														\
																				\
		if ((size) != (size_new)) {												\
			if (!(_p = sresize((p), (size_new), type))) {						\
				status = WFR_STATUS_FROM_ERRNO();								\
			} else {															\
				(p) = _p;														\
				(size) = (size_new);											\
				(status) = WFR_STATUS_CONDITION_SUCCESS;						\
			}																	\
		}																		\
																				\
		status;																	\
	})

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

WfrStatus WfrSnDuprintf(char **restrict ps, size_t *pn, const char *restrict format, ...);
const char *WfrStatusToErrorMessage(const char *context, WfrStatus status);
WfrStatus WfrToWcsDup(char *in, size_t in_size, wchar_t **pout_w);
wchar_t *WfrWcsNDup(const wchar_t *in_w, size_t in_w_len);

#endif // !PUTTY_WINFRIP_RTL_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
