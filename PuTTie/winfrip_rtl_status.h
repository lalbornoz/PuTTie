/*
 * winfrip_rtl_status.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_STATUS_H
#define PUTTY_WINFRIP_RTL_STATUS_H

#include <stdint.h>

/*
 * Public status type definitions and preprocessor macros private to PuTTie/winfrip*.c
 */

typedef enum WfrStatusFacility {
	WFR_STATUS_FACILITY_NONE 	= 0,
	WFR_STATUS_FACILITY_POSIX	= 1,
	WFR_STATUS_FACILITY_WINDOWS 	= 2,
	WFR_STATUS_FACILITY_PCRE2	= 3,
} WfrStatusFacility;

typedef uint64_t WfrStatusCondition;

typedef enum WfrStatusSeverity {
	WFR_STATUS_SEVERITY_ERROR	= 0,
	WFR_STATUS_SEVERITY_SUCCESS	= 1,
	// FIXME TODO XXX use SEVERITY_{BREAK,CONTINUE,DONE} everywhere
	WFR_STATUS_SEVERITY_BREAK	= 3,
	WFR_STATUS_SEVERITY_CONTINUE	= 5,
	WFR_STATUS_SEVERITY_DONE	= 7,
} WfrStatusSeverity;

typedef struct WfrStatus {
	const char *		file;
	int			line;
	WfrStatusFacility	facility:4;
	WfrStatusCondition	condition:56;
	WfrStatusSeverity	severity:4;
} __attribute__((packed)) WfrStatus;

/*
 * Constructors
 */

#define WFR_STATUS(facility, severity, condition)		\
	(WfrStatus){__FILE__, __LINE__, (facility), (condition), (severity)}
#define WFR_STATUS1(file, line, facility, severity, condition)	\
	(WfrStatus){(file), (line), (facility), (condition), (severity)}

#define WFR_STATUS_CONDITION_ERROR				\
	WFR_STATUS(WFR_STATUS_FACILITY_NONE, WFR_STATUS_SEVERITY_ERROR, WFR_STATUS_SEVERITY_SUCCESS)
#define WFR_STATUS_CONDITION_SUCCESS				\
	WFR_STATUS1(NULL, 0, WFR_STATUS_FACILITY_NONE, WFR_STATUS_SEVERITY_SUCCESS, WFR_STATUS_SEVERITY_SUCCESS)
#define WFR_STATUS_FROM_ERRNO()					\
	WFR_STATUS(WFR_STATUS_FACILITY_POSIX, WFR_STATUS_SEVERITY_ERROR, errno)
#define WFR_STATUS_FROM_ERRNO1(errno_)				\
	WFR_STATUS(WFR_STATUS_FACILITY_POSIX, WFR_STATUS_SEVERITY_ERROR, (errno_))
#define WFR_STATUS_FROM_WINDOWS()				\
	WFR_STATUS(WFR_STATUS_FACILITY_WINDOWS, WFR_STATUS_SEVERITY_ERROR, GetLastError())
#define WFR_STATUS_FROM_WINDOWS1(dwResult)			\
	WFR_STATUS(WFR_STATUS_FACILITY_WINDOWS, WFR_STATUS_SEVERITY_ERROR, (dwResult))
#define WFR_STATUS_FROM_WINDOWS_HRESULT(hres)			\
	WFR_STATUS(WFR_STATUS_FACILITY_WINDOWS, WFR_STATUS_SEVERITY_ERROR, (WfrStatusCondition)(hres))
#define WFR_STATUS_FROM_PCRE2(condition, severity)		\
	WFR_STATUS(WFR_STATUS_FACILITY_PCRE2, (severity), (condition))

/*
 * Getters
 */

#define WFR_STATUS_CONDITION(status)	\
	(status).condition
#define WFR_STATUS_FACILITY(status)	\
	(status).facility
#define WFR_STATUS_SEVERITY(status)	\
	(status).severity
#define WFR_STATUS_FAILURE(status)	\
	((WFR_STATUS_SEVERITY(status) & 0x1) == 0)
#define WFR_STATUS_SUCCESS(status)	\
	((WFR_STATUS_SEVERITY(status) & 0x1) == 1)

#endif // !PUTTY_WINFRIP_RTL_STATUS_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
