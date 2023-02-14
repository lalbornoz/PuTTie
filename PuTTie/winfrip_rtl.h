/*
 * winfrip_rtl.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_H
#define PUTTY_WINFRIP_RTL_H

#include <stdio.h>
#include <stdbool.h>

#include "winfrip_rtl_status.h"
#include "winfrip_rtl_macros.h"

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

WfrStatus		WfrConvertUtf8ToUtf16String(const char *in, size_t in_len, wchar_t **poutW);
WfrStatus		WfrConvertUtf8ToUtf16String1(const char *in, size_t in_len, wchar_t **poutW, size_t *poutW_size);
WfrStatus		WfrConvertUtf16ToUtf8String(const wchar_t *inW, size_t inW_len, char **pout);
WfrStatus		WfrConvertUtf16ToUtf8String1(const wchar_t *inW, size_t inW_len, char **pout, size_t *pout_size);
int			WfrFPrintUtf8AsUtf16F(FILE *restrict stream, const char *restrict format, ...);
bool			WfrIsNULTerminatedW(wchar_t *ws, size_t ws_size);
#define			WfrPrintUtf8AsUtf16F(...)	WfrFPrintUtf8AsUtf16F(stdout, ##__VA_ARGS__)
WfrStatus		WfrSnDuprintF(char **ps, size_t *pn, const char *format, ...);
WfrStatus		WfrSnDuprintV(char **ps, size_t *pn, const char *format, va_list ap);
const char *		WfrStatusToErrorMessage(WfrStatus status);
const wchar_t *		WfrStatusToErrorMessageW(WfrStatus status);
WfrStatus		WfrUpdateStringList(bool addfl, bool delfl, char **plist, size_t *plist_size, const char *const trans_item);
wchar_t *		WfrWcsNDup(const wchar_t *inW, size_t inW_len);

#endif // !PUTTY_WINFRIP_RTL_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
