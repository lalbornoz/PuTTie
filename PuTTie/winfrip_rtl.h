/*
 * winfrip_rtl.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_H
#define PUTTY_WINFRIP_RTL_H

#include <stdbool.h>

#include "winfrip_rtl_status.h"
#include "winfrip_rtl_macros.h"

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

unsigned int		WfrGetOsVersionMajor(void);
unsigned int		WfrGetOsVersionMinor(void);
bool			WfrIsVKeyDown(int nVirtKey);
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
