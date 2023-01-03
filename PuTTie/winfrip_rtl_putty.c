/*
 * winfrip_rtl_putty.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "dialog.h"
#include "terminal.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_rtl.h"

#include <errno.h>

/*
 * Private type definitions
 */

typedef struct compressed_scrollback_line {
	size_t	len;
} compressed_scrollback_line;

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

bool
WfrpIsVKeyDown(
	int		nVirtKey
	)
{
	return (GetKeyState(nVirtKey) < 0);
}

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
