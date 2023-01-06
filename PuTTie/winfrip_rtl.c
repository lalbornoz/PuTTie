/*
 * winfrip_rtl.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdarg.h>
#include <stdio.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#pragma GCC diagnostic pop

#include <windows.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_pcre2.h"

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

WfrStatus
WfrSnDuprintf(
	char **restrict			ps,
	size_t *				pn,
	const char *restrict	format,
							...
	)
{
	va_list		ap;
	WfrStatus	status;
	char *		s = NULL;
	int			s_len;
	size_t		s_size;


	if (!(s = snewn(1, char))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		s_size = 1;
		va_start(ap, format);
		s_len = vsnprintf(s, s_size, format, ap);
		va_end(ap);

		if (s_len < 0) {
			status = WFR_STATUS_FROM_ERRNO();
		} else if (s_len == 0) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else if (WFR_STATUS_SUCCESS(status = WFR_SRESIZE_IF_NEQ_SIZE(
						s, s_size, (size_t)s_len + 1, char)))
		{
			va_start(ap, format);
			s_len = vsnprintf(s, s_size, format, ap);
			va_end(ap);

			s[((s_len < 0) ? 0 : s_len)] = '\0';
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	if (WFR_STATUS_SUCCESS(status)) {
		*ps = s;
		if (pn) {
			*pn = s_size;
		}
	} else {
		WFR_SFREE_IF_NOTNULL(s);
	}

	return status;
}

const char *
WfrStatusToErrorMessage(
	const char *	context,
	WfrStatus		status
	)
{
	static char		condition_msg[128];
	static wchar_t	condition_msg_w[128];
	static char		error_msg[256];


	switch (WFR_STATUS_FACILITY(status)) {
	default:
		strncpy(condition_msg, "(unknown facility)", sizeof(condition_msg));
		break;

	case WFR_STATUS_FACILITY_POSIX:
		strncpy(condition_msg, strerror(status.condition), sizeof(condition_msg) - 1);
		break;

	case WFR_STATUS_FACILITY_WINDOWS:
		condition_msg[0] = '\0';
		(void)FormatMessageA(
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, status.condition,
				LANG_USER_DEFAULT,
				condition_msg, sizeof(condition_msg),
				NULL);
		break;

	case WFR_STATUS_FACILITY_PCRE2:
		condition_msg[0] = '\0';
		condition_msg_w[0] = L'\0';
		(void)pcre2_get_error_message(
				status.condition,
				condition_msg_w, sizeof(condition_msg_w));
		(void)WideCharToMultiByte(
				CP_UTF8, 0, condition_msg_w,
				wcslen(condition_msg_w) + 1,
				condition_msg, sizeof(condition_msg),
				NULL, NULL);
		break;
	}

	WFR_SNPRINTF(
			error_msg, sizeof(error_msg),
			"Error %s in %s:%d: %s",
			context, status.file, status.line,
			condition_msg);

	return error_msg;
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

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
