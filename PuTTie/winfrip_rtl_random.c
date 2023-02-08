/*
 * winfrip_rtl_random.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <windows.h>
#include <bcrypt.h>
#include <ntstatus.h>

#include "PuTTie/winfrip_rtl.h"

/*
 * Private variables
 */

static BCRYPT_ALG_HANDLE	WffbphAlgorithm = NULL;

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

WfrStatus
WfrGenRandom(
	PUCHAR	pbBuffer,
	ULONG	cbBuffer
	)
{
	NTSTATUS	ntstatus;
	WfrStatus	status;


	if ((WffbphAlgorithm == NULL)
	&&  ((ntstatus = BCryptOpenAlgorithmProvider(
			&WffbphAlgorithm,
			BCRYPT_RNG_ALGORITHM, NULL, 0)) != STATUS_SUCCESS))
	{
		status = WFR_STATUS_FROM_WINDOWS_NT(ntstatus);
	} else if ((ntstatus = BCryptGenRandom(
			WffbphAlgorithm, pbBuffer, cbBuffer, 0) != STATUS_SUCCESS))
	{
		status = WFR_STATUS_FROM_WINDOWS_NT(ntstatus);
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
