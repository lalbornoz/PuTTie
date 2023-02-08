/*
 * winfrip_rtl_random.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_RANDOM_H
#define PUTTY_WINFRIP_RTL_RANDOM_H

#include "winfrip_rtl_status.h"

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

WfrStatus	WfrGenRandom(PUCHAR pbBuffer, ULONG cbBuffer);

#endif // !PUTTY_WINFRIP_RTL_RANDOM_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
