/*
 * winfrip_rtl_macros.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_MACROS_H
#define PUTTY_WINFRIP_RTL_MACROS_H

/*
 * Public memory management macros private to PuTTie/winfrip*.c
 */

#define WFR_ARRAYCOUNT(array)								\
	(sizeof((array)) / sizeof((array)[0]))

#define WFR_FREE(p) ({									\
	free((void *)(p)); (p) = NULL;							\
	WFR_STATUS_CONDITION_SUCCESS;							\
})

#define WFR_FREE_IF_NOTNULL(p) ({							\
	if ((p)) {									\
		WFR_FREE((p));								\
	}										\
	WFR_STATUS_CONDITION_SUCCESS;							\
})

#define WFR_FREE_VECTOR_IF_NOTNULL(len, vector) ({					\
	if ((vector)) {									\
		for (size_t nitem = 0; nitem < (len); nitem++) {			\
			WFR_FREE_IF_NOTNULL((vector)[nitem]);				\
		}									\
		WFR_FREE((vector));							\
	}										\
	(len) = 0;									\
	WFR_STATUS_CONDITION_SUCCESS;							\
})

#define WFR_NEW(status, p, type)							\
	WFR_NEWN(status, p, 1, type)

#define WFR_NEW1(p, type)								\
	WFR_NEWN1(p, 1, type)

#define WFR_NEWN(status, p, n, type) ({							\
	type *		_p;								\
	WfrStatus	_status;							\
       											\
	_p = malloc((n) * sizeof(type));						\
	if ((_p)) {									\
		(p) = _p;								\
		_status = WFR_STATUS_CONDITION_SUCCESS;					\
	} else {									\
		_status = WFR_STATUS_FROM_ERRNO();					\
	}										\
											\
	(status) = _status;								\
	WFR_STATUS_SUCCESS((status));							\
})

#define WFR_NEWN_CAST(status, p, n, type, type_cast) ({					\
	type *		_p;								\
	WfrStatus	_status;							\
       											\
	_p = malloc((n) * sizeof(type));						\
	if ((_p)) {									\
		(p) = (type_cast *)_p;							\
		_status = WFR_STATUS_CONDITION_SUCCESS;					\
	} else {									\
		_status = WFR_STATUS_FROM_ERRNO();					\
	}										\
											\
	(status) = _status;								\
	WFR_STATUS_SUCCESS(_status);							\
})

#define WFR_NEWN1(p, n, type) ({							\
	type *		_p;								\
	WfrStatus	_status;							\
       											\
	_p = malloc((n) * sizeof(type));						\
	if ((_p)) {									\
		(p) = _p;								\
		_status = WFR_STATUS_CONDITION_SUCCESS;					\
	} else {									\
		_status = WFR_STATUS_FROM_ERRNO();					\
	}										\
											\
	WFR_STATUS_SUCCESS(_status);							\
})

#define WFR_RELEASE_IF_NOTNULL(p)							\
	if ((p)) {									\
		(void)(p)->lpVtbl->Release((p)); (p) = NULL;				\
	}

#define WFR_RESIZE(status, p, size, size_new, type) ({					\
	typeof (p)	_p;								\
	WfrStatus	_status;							\
											\
	if (!(_p = (type *)realloc((p), (size_new) * sizeof(type)))) {			\
		_status = WFR_STATUS_FROM_ERRNO();					\
	} else {									\
		(p) = _p;								\
		(size) = (size_new);							\
		_status = WFR_STATUS_CONDITION_SUCCESS;					\
	}										\
											\
	(status) = _status;								\
	WFR_STATUS_SUCCESS((status));							\
})

#define WFR_RESIZE_IF_NEQ_SIZE(status, p, size, size_new, type) ({			\
	typeof (p)	_p;								\
	WfrStatus	_status;							\
											\
	if ((size) != (size_new)) {							\
		if (!(_p = realloc((p), (size_new) * sizeof(type)))) {			\
			_status = WFR_STATUS_FROM_ERRNO();				\
		} else {								\
			(p) = _p;							\
			(size) = (size_new);						\
			_status = WFR_STATUS_CONDITION_SUCCESS;				\
		}									\
	} else {									\
			_status = WFR_STATUS_CONDITION_SUCCESS;				\
	}										\
											\
	(status) = _status;								\
	WFR_STATUS_SUCCESS((status));							\
})

#define WFR_SIZEOF_WSTRING(wstring)							\
	WFR_ARRAYCOUNT(wstring)

#define WFR_WMKTEMP_S(nameTemplate) ({							\
	errno_t		_errno;								\
	WfrStatus	_status;							\
       											\
	_errno = _wmktemp_s((nameTemplate), wcslen((nameTemplate)) + 1);		\
	if (_errno == 0) {								\
		_status = WFR_STATUS_CONDITION_SUCCESS;					\
	} else {									\
		_status = WFR_STATUS_FROM_ERRNO1(_errno);				\
	}										\
											\
	_status;									\
})

/*
 * Public string processing macros private to PuTTie/winfrip*.c
 */

#define WFR_SNPRINTF(s, n, fmt, ...) ({							\
	int		c = snprintf((s), (n), (fmt), ## __VA_ARGS__);			\
	((char *)(s))[(n) - 1] = '\0';							\
	c;										\
})

#define WFR_SNWPRINTF(ws, n, fmt, ...) ({						\
	int		c = snwprintf((ws), (n), (fmt), ## __VA_ARGS__);		\
	((wchar_t *)(ws))[(n) - 1] = L'\0';						\
	c;										\
})

#define WFR_STRNCPY(s1, s2, n) ({							\
	char *	rc = strncpy((s1), (s2), (n));						\
	(s1)[(n) ? ((n) - 1) : 0] = '\0';						\
	rc;										\
})

#define WFR_WCSNCPY(ws1, ws2, n) ({							\
	wchar_t *	rc = wcsncpy((ws1), (ws2), (n));				\
	(ws1)[(n) ? ((n) - 1) : 0] = L'\0';						\
	rc;										\
})

/*
 * Public general macros private to PuTTie/winfrip*.c
 */

#define WFR_DIV_ROUNDDOWN(a, b)								\
	(((a) - ((a) % (b))) / (b))
#define WFR_DIV_ROUNDUP(a, b)								\
	(((a) / (b)) + ((a) % (b)))

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

#endif // !PUTTY_WINFRIP_RTL_MACROS_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
