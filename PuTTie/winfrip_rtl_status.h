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
	WFR_STATUS_FACILITY_GDI_PLUS	= 1,
	WFR_STATUS_FACILITY_POSIX	= 2,
	WFR_STATUS_FACILITY_WINDOWS 	= 3,
	WFR_STATUS_FACILITY_WINDOWS_NT	= 4,
	WFR_STATUS_FACILITY_PCRE2	= 5,
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
 * Base WfrStatus constructors and values
 */

#define WFR_STATUS(facility, severity, condition)		\
	(WfrStatus){__FILE__, __LINE__, (facility), (condition), (severity)}
#define WFR_STATUS1(file, line, facility, severity, condition)	\
	(WfrStatus){(file), (line), (facility), (condition), (severity)}
#define WFR_STATUS_CONDITION_ERROR				\
	WFR_STATUS(WFR_STATUS_FACILITY_NONE, WFR_STATUS_SEVERITY_ERROR, WFR_STATUS_SEVERITY_SUCCESS)
#define WFR_STATUS_CONDITION_SUCCESS				\
	WFR_STATUS1(NULL, 0, WFR_STATUS_FACILITY_NONE, WFR_STATUS_SEVERITY_SUCCESS, WFR_STATUS_SEVERITY_SUCCESS)

/*
 * Bind LSTATUS expression to WfrStatus and evaluate to WFR_STATUS_SUCCESS() thereof
 */
#define WFR_STATUS_BIND_LSTATUS(status, ...) ({				\
	LSTATUS		lstatus;					\
	WfrStatus	status_;					\
									\
	lstatus = (__VA_ARGS__);					\
	if (lstatus == ERROR_SUCCESS) {					\
		status_ = WFR_STATUS_CONDITION_SUCCESS;			\
	} else {							\
		status_ = WFR_STATUS(					\
				WFR_STATUS_FACILITY_WINDOWS,		\
				WFR_STATUS_SEVERITY_ERROR,		\
				lstatus);				\
	}								\
									\
	(status) = status_;						\
	WFR_STATUS_SUCCESS((status));					\
})
#define WFR_STATUS_SUCCESS_LSTATUS(...)					\
	WFR_STATUS_BIND_LSTATUS(__VA_ARGS__)
#define WFR_STATUS_FAILURE_LSTATUS(...)					\
	!WFR_STATUS_BIND_LSTATUS(__VA_ARGS__)
#define WFR_SUCCESS_LSTATUS(...)					\
	WFR_STATUS_BIND_LSTATUS(__VA_ARGS__)
#define WFR_FAILURE_LSTATUS(...)					\
	!WFR_STATUS_BIND_LSTATUS(__VA_ARGS__)

/*
 * Bind POSIX expression to WfrStatus and evaluate to WFR_STATUS_SUCCESS() thereof
 */
#define WFR_STATUS_BIND_POSIX(status, ...) ({				\
	bool		result;						\
	WfrStatus	status_;					\
									\
	result = ((__VA_ARGS__));					\
	if (result) {							\
		status_ = WFR_STATUS_CONDITION_SUCCESS;			\
	} else {							\
		status_ = WFR_STATUS_FROM_ERRNO();			\
	}								\
									\
	(status) = status_;						\
	WFR_STATUS_SUCCESS((status));					\
})
#define WFR_STATUS_SUCCESS_POSIX(...)					\
	WFR_STATUS_BIND_POSIX(__VA_ARGS__)
#define WFR_STATUS_FAILURE_POSIX(...)					\
	!WFR_STATUS_BIND_POSIX(__VA_ARGS__)
#define WFR_SUCCESS_POSIX(...)						\
	WFR_STATUS_BIND_POSIX(__VA_ARGS__)
#define WFR_FAILURE_POSIX(...)						\
	!WFR_STATUS_BIND_POSIX(__VA_ARGS__)

/*
 * Bind Windows expression to WfrStatus and evaluate to WFR_STATUS_SUCCESS() thereof
 */
#define WFR_STATUS_BIND_WINDOWS(status, ...) ({				\
	DWORD		dwLastError;					\
	bool		result;						\
	WfrStatus	status_;					\
									\
	result = (__VA_ARGS__);						\
	if (result == TRUE) {						\
		status_ = WFR_STATUS_CONDITION_SUCCESS;			\
	} else {							\
		dwLastError = GetLastError();				\
		status_ = WFR_STATUS(					\
				WFR_STATUS_FACILITY_WINDOWS,		\
				WFR_STATUS_SEVERITY_ERROR,		\
				dwLastError);				\
	}								\
									\
	(status) = status_;						\
	WFR_STATUS_SUCCESS((status));					\
})
#define WFR_STATUS_SUCCESS_WINDOWS(...)					\
	WFR_STATUS_BIND_WINDOWS(__VA_ARGS__)
#define WFR_STATUS_FAILURE_WINDOWS(...)					\
	!WFR_STATUS_BIND_WINDOWS(__VA_ARGS__)
#define WFR_SUCCESS_WINDOWS(...)					\
	WFR_STATUS_BIND_WINDOWS(__VA_ARGS__)
#define WFR_FAILURE_WINDOWS(...)					\
	!WFR_STATUS_BIND_WINDOWS(__VA_ARGS__)

/*
 * Bind expression to WfrStatus and evaluate to WFR_STATUS_SUCCESS() thereof
 * with optional explicit errno value on failure
 */
#define WFR_STATUS_BIND_ERRNO1(status, errno1, ...) ({			\
	WfrStatus	status_;					\
									\
	if ((__VA_ARGS__)) {						\
		status_ = WFR_STATUS_CONDITION_SUCCESS;			\
	} else {							\
		status_ = WFR_STATUS_FROM_ERRNO1((errno1));		\
	}								\
									\
	(status) = status_;						\
	WFR_STATUS_SUCCESS(status);					\
})
#define WFR_STATUS_SUCCESS_ERRNO1(...)					\
	WFR_STATUS_BIND_ERRNO1(__VA_ARGS__)
#define WFR_STATUS_FAILURE_ERRNO1(...)					\
	!WFR_STATUS_BIND_ERRNO1(__VA_ARGS__)
#define WFR_SUCCESS_ERRNO1(...)						\
	WFR_STATUS_BIND_ERRNO1(__VA_ARGS__)
#define WFR_FAILURE_ERRNO1(...)						\
	!WFR_STATUS_BIND_ERRNO1(__VA_ARGS__)

/*
 * WfrStatus constructors from POSIX errno, GDI+ return codes, Windows results,
 * Windows NT NTSTATUS, Windows COM HRESults, and PCRE2 error codes
 */

#define WFR_STATUS_FROM_ERRNO()				\
	WFR_STATUS(WFR_STATUS_FACILITY_POSIX, WFR_STATUS_SEVERITY_ERROR, errno)
#define WFR_STATUS_FROM_ERRNO1(errno_)			\
	WFR_STATUS(WFR_STATUS_FACILITY_POSIX, WFR_STATUS_SEVERITY_ERROR, (errno_))
#define WFR_STATUS_FROM_GDI_PLUS(rc)			\
	WFR_STATUS(WFR_STATUS_FACILITY_GDI_PLUS, WFR_STATUS_SEVERITY_ERROR, (rc))
#define WFR_STATUS_FROM_WINDOWS()			\
	WFR_STATUS(WFR_STATUS_FACILITY_WINDOWS, WFR_STATUS_SEVERITY_ERROR, GetLastError())
#define WFR_STATUS_FROM_WINDOWS1(dwResult)		\
	WFR_STATUS(WFR_STATUS_FACILITY_WINDOWS, WFR_STATUS_SEVERITY_ERROR, (dwResult))
#define WFR_STATUS_FROM_WINDOWS_NT(ntstatus)		\
	WFR_STATUS(WFR_STATUS_FACILITY_WINDOWS_NT, WFR_STATUS_SEVERITY_ERROR, (ntstatus))
#define WFR_STATUS_FROM_WINDOWS_HRESULT(hres)		\
	WFR_STATUS(WFR_STATUS_FACILITY_WINDOWS, WFR_STATUS_SEVERITY_ERROR, (WfrStatusCondition)(hres))
#define WFR_STATUS_FROM_PCRE2(condition, severity)	\
	WFR_STATUS(WFR_STATUS_FACILITY_PCRE2, (severity), (condition))

/*
 * Check if WfrStatus is POSIX or Windows error that indicates non-existence
 * of an item, file, or resource, etc.
 */

#ifndef ERROR_FILE_NOT_FOUND
#define ERROR_FILE_NOT_FOUND	2
#endif /* !ERROR_FILE_NOT_FOUND */
#ifndef ERROR_PATH_NOT_FOUND
#define ERROR_PATH_NOT_FOUND	3
#endif /* !ERROR_PATH_NOT_FOUND */
#define WFR_STATUS_IS_NOT_FOUND(status)						\
	(   WFR_STATUS_FAILURE((status))					\
	 && (((WFR_STATUS_FACILITY((status)) == WFR_STATUS_FACILITY_POSIX)	\
	 &&   (WFR_STATUS_CONDITION((status)) == ENOENT))			\
	 ||  ((WFR_STATUS_FACILITY((status)) == WFR_STATUS_FACILITY_WINDOWS)	\
	 &&   (WFR_STATUS_CONDITION((status)) == ERROR_FILE_NOT_FOUND))		\
	 ||  ((WFR_STATUS_FACILITY((status)) == WFR_STATUS_FACILITY_WINDOWS)	\
	 &&   (WFR_STATUS_CONDITION((status)) == ERROR_PATH_NOT_FOUND))))

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
#define WFR_FAILURE(status)		\
	WFR_STATUS_FAILURE(status)

#define WFR_STATUS_SUCCESS(status)	\
	((WFR_STATUS_SEVERITY(status) & 0x1) == 1)
#define WFR_SUCCESS(status)		\
	WFR_STATUS_SUCCESS(status)

#endif // !PUTTY_WINFRIP_RTL_STATUS_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
