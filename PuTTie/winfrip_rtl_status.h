/*
 * winfrip_rtl_status.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_STATUS_H
#define PUTTY_WINFRIP_RTL_STATUS_H

#include <stdint.h>

/*
 * Public type definitions and preprocessor macros private to PuTTie/winfrip*.c
 */

typedef uint64_t WfrStatus;

#define WFR_STATUS_CONDITION(status)							\
		(((status) & 0x0ffffffffffffff0) >> 4)
#define WFR_STATUS_CONDITION_(condition)						\
		((((uint64_t)(condition)) & 0x00ffffffffffffff) << 4)

// FIXME TODO use SEVERITY_{BREAK,CONTINUE,DONE} everywhere
#define WFR_STATUS_SEVERITY_ERROR		0
#define WFR_STATUS_SEVERITY_SUCCESS		1
#define WFR_STATUS_SEVERITY_BREAK		3
#define WFR_STATUS_SEVERITY_CONTINUE	5
#define WFR_STATUS_SEVERITY_DONE		7
#define WFR_STATUS_SEVERITY(status)								\
		((status) & 0x000000000000000f)
#define WFR_STATUS_SEVERITY_(severity)							\
		(((uint64_t)(severity)) & 0x000000000000000f)

#define WFR_STATUS_FACILITY_NONE		0
#define WFR_STATUS_FACILITY_POSIX		1
#define WFR_STATUS_FACILITY_WINDOWS		2
#define WFR_STATUS_FACILITY(status)								\
		(((status) & 0xf000000000000000) >> 60)
#define WFR_STATUS_FACILITY_(facility)							\
		((((uint64_t)(facility)) & 0x000000000000000f) << 60)

#define WFR_STATUS_FAILURE(status)								\
		(((status) & 0x1) == 0)
#define WFR_STATUS_SUCCESS(status)								\
		(((status) & 0x1) == 1)

#define WFR_STATUS_CONDITION_ERROR								\
		((WfrStatus)											\
		   WFR_STATUS_SEVERITY_(WFR_STATUS_SEVERITY_SUCCESS)	\
		 | WFR_STATUS_CONDITION_(WFR_STATUS_SEVERITY_ERROR)		\
		 | WFR_STATUS_FACILITY_(WFR_STATUS_FACILITY_NONE)		\
		 )
#define WFR_STATUS_CONDITION_SUCCESS							\
		((WfrStatus)											\
		   WFR_STATUS_SEVERITY_(WFR_STATUS_SEVERITY_SUCCESS)	\
		 | WFR_STATUS_CONDITION_(WFR_STATUS_SEVERITY_SUCCESS)	\
		 | WFR_STATUS_FACILITY_(WFR_STATUS_FACILITY_NONE)		\
		 )
#define WFR_STATUS_FROM_ERRNO()									\
		((WfrStatus)											\
		   WFR_STATUS_SEVERITY_(WFR_STATUS_SEVERITY_ERROR)		\
		 | WFR_STATUS_CONDITION_(errno)							\
		 | WFR_STATUS_FACILITY_(WFR_STATUS_FACILITY_POSIX)		\
		 )
#define WFR_STATUS_FROM_ERRNO1(errno_)							\
		((WfrStatus)											\
		   WFR_STATUS_SEVERITY_(WFR_STATUS_SEVERITY_ERROR)		\
		 | WFR_STATUS_CONDITION_(errno_)						\
		 | WFR_STATUS_FACILITY_(WFR_STATUS_FACILITY_POSIX)		\
		 )
#define WFR_STATUS_FROM_WINDOWS()								\
		((WfrStatus)											\
		   WFR_STATUS_SEVERITY_(WFR_STATUS_SEVERITY_ERROR)		\
		 | WFR_STATUS_CONDITION_(GetLastError())				\
		 | WFR_STATUS_FACILITY_(WFR_STATUS_FACILITY_WINDOWS)	\
		 )
#define WFR_STATUS_FROM_WINDOWS1(dwResult)						\
		((WfrStatus)											\
		   WFR_STATUS_SEVERITY_(WFR_STATUS_SEVERITY_ERROR)		\
		 | WFR_STATUS_CONDITION_(dwResult)						\
		 | WFR_STATUS_FACILITY_(WFR_STATUS_FACILITY_WINDOWS)	\
		 )

//
// winfrip_rtl_pcre2.[ch]
//

#define WFR_STATUS_FACILITY_PCRE2		3
#define WFR_STATUS_FROM_PCRE2(cond, sev)						\
		((WfrStatus)											\
		   WFR_STATUS_SEVERITY_(sev)							\
		 | WFR_STATUS_CONDITION_(cond)							\
		 | WFR_STATUS_FACILITY_(WFR_STATUS_FACILITY_PCRE2)		\
		 )

#endif // !PUTTY_WINFRIP_RTL_STATUS_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
