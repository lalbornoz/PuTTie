/*
 * winfrip_rtl_debug.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdio.h>

#include <windows.h>

#include "PuTTie/winfrip_rtl.h"

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
WfrDebugInit(
	void
	)
{
#ifdef WINFRIP_DEBUG
	COORD		dwSize = {.X = 125, .Y = 50 * 25};
	SMALL_RECT	consoleWindow = {.Left = 0, .Top = 0, .Right = dwSize.X - 1, .Bottom = (dwSize.Y / 25) - 1};
	HANDLE		hConsoleOutput;


#ifndef WINFRIP_DEBUG_NOCONSOLE
	AllocConsole();
#endif /* WINFRIP_DEBUG_NOCONSOLE */
	AttachConsole(GetCurrentProcessId());
	_wfreopen(L"CON", L"w", stdout);
	_wfreopen(L"CON", L"w", stderr);
	_wfreopen(L"CONIN", L"r", stdin);
	hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	(void)SetConsoleScreenBufferSize(hConsoleOutput, dwSize);
	(void)SetConsoleWindowInfo(hConsoleOutput, TRUE, &consoleWindow);
#endif
}

#ifdef WINFRIP_DEBUG
void
WfrDebugF(
	const char *	fmt,
	const char *	file,
	const char *	func,
	int		line,
			...
	)
{
	va_list		ap;


	fprintf(stderr, "In %s:%d:%s():\n", file, line, func);
	va_start(ap, line);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n\n");
}
#endif

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
