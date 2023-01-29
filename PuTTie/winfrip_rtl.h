/*
 * winfrip_rtl.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_H
#define PUTTY_WINFRIP_RTL_H

#include <stdbool.h>

#include "winfrip_rtl_status.h"

/*
 * Public macros private to PuTTie/winfrip*.c
 */

#define WFR_FREE(p) ({									\
	free((void *)(p)); (p) = NULL;							\
})

#define WFR_FREE_IF_NOTNULL(p)								\
	if ((p)) {									\
		WFR_FREE((p));								\
	}

#define WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, fmt, ...)				\
	WFR_IF_STATUS_FAILURE_MESSAGEBOX1("PuTTie", (status), fmt, ## __VA_ARGS__)
#define WFR_IF_STATUS_FAILURE_MESSAGEBOX1(caption, status, fmt, ...)			\
	if (WFR_STATUS_FAILURE((status))) {						\
		(void)WfrMessageBoxF(							\
				(caption), MB_ICONERROR | MB_OK | MB_DEFBUTTON1,	\
				fmt ": %s",	## __VA_ARGS__,				\
				WfrStatusToErrorMessage((status)));			\
	}

#define WFR_INTCMP(i1, i2)								\
	(((i1) < (i2)) ? -1 : ((i1) > (i2)) ? 1 : 0)

#define WFR_IS_ABSOLUTE_PATHW(pname)							\
	   (((((pname)[0] >= L'a') && ((pname)[0] <= L'z'))				\
	||   (((pname)[0] >= L'A') && ((pname)[0] <= L'Z')))				\
	&&  ((pname)[1] == L':')							\
	&&  ((pname)[2] == L'\\'))

#define WFR_LAMBDA(type, fn) ({								\
	type __fn__ fn									\
		__fn__;									\
})

#define WFR_MAX(a, b) ({								\
	__typeof__ (a) _a = (a);							\
	__typeof__ (b) _b = (b);							\
	_a > _b ? _a : _b;								\
})

#define WFR_MIN(a, b) ({								\
	__typeof__ (a) _a = (a);							\
	__typeof__ (b) _b = (b);							\
	_a < _b ? _a : _b;								\
})

#define WFR_NEW(type)									\
	(type *)malloc(sizeof(type))

#define WFR_NEWN(n, type)								\
	(type *)malloc((n) * sizeof(type))

#define WFR_RELEASE_IF_NOTNULL(p)							\
	if ((p)) {									\
		(void)(p)->Release(); (p) = NULL;					\
	}

#define WFR_RESIZE(p, size, size_new, type) ({						\
	typeof (p)	_p;								\
	WfrStatus	status;								\
											\
	if (!(_p = (type *)realloc((p), (size_new) * sizeof(type)))) {			\
		status = WFR_STATUS_FROM_ERRNO();					\
	} else {									\
		(p) = _p;								\
		(size) = (size_new);							\
		(status) = WFR_STATUS_CONDITION_SUCCESS;				\
	}										\
											\
	status;										\
})

#define WFR_RESIZE_IF_NEQ_SIZE(p, size, size_new, type) ({				\
	typeof (p)	_p;								\
	WfrStatus	status;								\
											\
	if ((size) != (size_new)) {							\
		if (!(_p = realloc((p), (size_new) * sizeof(type)))) {			\
			status = WFR_STATUS_FROM_ERRNO();				\
		} else {								\
			(p) = _p;							\
			(size) = (size_new);						\
			(status) = WFR_STATUS_CONDITION_SUCCESS;			\
		}									\
	}										\
											\
	status;										\
})

#define WFR_RESIZE_VECTOR_WITHNULL(p, count, count_new, type) ({			\
	typeof (p)	_p;								\
	WfrStatus	status;								\
											\
	if (!(_p = realloc((p), ((count_new) + 1) * sizeof(type)))) {			\
		status = WFR_STATUS_FROM_ERRNO();					\
	} else {									\
		(p) = _p;								\
		(count) = (count_new);							\
		(status) = WFR_STATUS_CONDITION_SUCCESS;				\
	}										\
											\
	status;										\
})

#define WFR_SNPRINTF(s, n, fmt, ...) ({							\
	int		c = snprintf((s), (n), (fmt), ## __VA_ARGS__);			\
	((char *)(s))[(n) - 1] = '\0';							\
	c;										\
})

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

unsigned int		WfrGetOsVersionMajor(void);
unsigned int		WfrGetOsVersionMinor(void);
int			WfrMessageBoxF(const char *lpCaption, unsigned int uType, const char *format, ...);
int			WfrMessageBoxFW(const wchar_t *lpCaption, unsigned int uType, const wchar_t *format, ...);
WfrStatus		WfrSnDuprintf(char **ps, size_t *pn, const char *format, ...);
const char *		WfrStatusToErrorMessage(WfrStatus status);
const wchar_t *		WfrStatusToErrorMessageW(WfrStatus status);
WfrStatus		WfrToWcsDup(char *in, size_t in_size, wchar_t **pout_w);
wchar_t *		WfrWcsNDup(const wchar_t *in_w, size_t in_w_len);

#endif // !PUTTY_WINFRIP_RTL_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
