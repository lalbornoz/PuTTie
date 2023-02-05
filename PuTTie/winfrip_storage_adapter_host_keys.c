/*
 * winfrip_storage_adapter_host_keys.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "PuTTie/winfrip_storage_wrap.h"
#include "storage.h"
#include "PuTTie/winfrip_storage_unwrap.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_debug.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_host_keys.h"
#include "PuTTie/winfrip_storage_jump_list.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"

/*
 * Public wrapped PuTTY storage.h type definitions and subroutines
 */

/* ----------------------------------------------------------------------
 * Functions to access PuTTY's host key database.
 */

/*
 * See if a host key matches the database entry. Return values can
 * be 0 (entry matches database), 1 (entry is absent in database),
 * or 2 (entry exists in database and is different).
 */

int
check_stored_host_key(
	const char *	hostname,
	int		port,
	const char *	keytype,
	const char *	key
	)
{
	const char *	key_;
	char *		key_name;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsPrintHostKeyName(
			hostname, port, keytype, &key_name)))
	{
		status = WfsGetHostKey(WfsGetBackend(), false, key_name, &key_);
		WFR_FREE(key_name);
		if (WFR_STATUS_SUCCESS(status)) {
			if (strcmp(key, key_) == 0) {
				return 0;
			} else {
				return 2;
			}
		} else {
			WFR_DEBUG_FAIL();
			return 1;
		}
	} else {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "checking stored host key");
		return 1;
	}
}

/*
 * Write a host key into the database, overwriting any previous
 * entry that might have been there.
 */

void
store_host_key(
	Seat *		seat,
	const char *	hostname,
	int		port,
	const char *	keytype,
	const char *	key
	)
{
	char *		key_, *key_name;
	WfrStatus	status;


	(void)seat;

	if (WFR_STATUS_SUCCESS(status = WfsPrintHostKeyName(
			hostname, port, keytype, &key_name)))
	{
		if (!(key_ = strdup(key))) {
			status = WFR_STATUS_FROM_ERRNO();
		} else {
			status = WfsSetHostKey(WfsGetBackend(), key_name, key_);
			WFR_FREE(key_name);
			if (WFR_STATUS_FAILURE(status)) {
				WFR_FREE(key_);
			}
		}
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "storing host key");
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
