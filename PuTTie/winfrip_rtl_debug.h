/*
 * winfrip_rtl_debug.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_DEBUG_H
#define PUTTY_WINFRIP_RTL_DEBUG_H

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

void	WfrDebugInit(void);

#ifdef WINFRIP_DEBUG

#define WFR_DEBUG_ASSERT(expr) do {				\
	if (!(expr)) {						\
		WFR_DEBUGF("assertion failure: %s\n"		\
				   "GetLastError(): 0x%08x",	\
				   #expr, GetLastError());	\
		}						\
	} while (0)

#define WFR_DEBUG_ASSERTF(expr, fmt, ...) do {			\
	if (!(expr)) {						\
		WFR_DEBUGF(fmt "\n" "assertion failure: %s\n"	\
				   "GetLastError(): 0x%08x",	\
				   ##__VA_ARGS__, #expr,	\
				   GetLastError());		\
		}						\
	} while (0)

#define WFR_DEBUG_FAIL()					\
	WFR_DEBUGF("failure condition", __FILE__, __func__, __LINE__)

#define WFR_DEBUGF(fmt, ...)					\
	WfrDebugF(fmt, __FILE__, __func__, __LINE__, ##__VA_ARGS__)

void	WfrDebugF(const char *fmt, const char *file, const char *func, int line, ...);

#else

#define WFR_DEBUG_ASSERT(expr)			(void)(expr)
#define WFR_DEBUG_ASSERTF(expr, fmt, ...)	(void)(expr)
#define WFR_DEBUG_FAIL()
#define WFR_DEBUGF(fmt, ...)

#endif // WINFRIP_DEBUG

#endif // !PUTTY_WINFRIP_RTL_DEBUG_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
