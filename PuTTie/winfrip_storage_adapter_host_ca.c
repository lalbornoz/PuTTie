/*
 * winfrip_storage_adapter_host_ca.c - pointless frippery & tremendous amounts of bloat
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
 * Functions to access PuTTY's configuration for trusted host
 * certification authorities. This must be stored separately from the
 * saved-session data, because the whole point is to avoid having to
 * configure CAs separately per session.
 */

host_ca_enum *
enum_host_ca_start(
	void
	)
{
	void *		handle;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsEnumerateHostCAs(
			WfsGetBackend(), false, true,
			NULL, NULL, &handle)))
	{
		return handle;
	} else {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "enumerating host CAs");
		return NULL;
	}
}

bool
enum_host_ca_next(
	host_ca_enum *	handle,
	strbuf *	out
	)
{
	bool		donefl;
	char *		name;
	size_t		name_len;
	WfrStatus	status;


	status = WfsEnumerateHostCAs(
		WfsGetBackend(), false, false,
		&donefl, &name, (void **)&handle);

	if (WFR_STATUS_FAILURE(status) || donefl) {
		if (WFR_STATUS_FAILURE(status)) {
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "enumerating host CAs");
		}
		return false;
	} else {
		name_len = strlen(name);
		for (size_t nchar = 0; nchar < name_len; nchar++) {
			put_byte(out, name[nchar]);
		}
		WFR_FREE(name);
		return true;
	}
}

void
enum_host_ca_finish(
	host_ca_enum *	handle
	)
{
	(void)handle;
}

host_ca *
host_ca_load(
	const char *	name
	)
{
	WfsHostCA *	hca;
	host_ca *	hca_out;
	WfrStatus	status;


	status = WfsGetHostCA(WfsGetBackend(), false, name, &hca);

	if (WFR_STATUS_SUCCESS(status)) {
		if (!(hca_out = WFR_NEW(host_ca))) {
			status = WFR_STATUS_FROM_ERRNO();
			return NULL;
		} else {
			hca_out->name = NULL;
			hca_out->ca_public_key = NULL;
			hca_out->validity_expression = NULL;

			if (!(hca_out->name = strdup(hca->name))
			||  !(hca_out->ca_public_key = strbuf_dup(make_ptrlen(hca->public_key, strlen(hca->public_key))))
			||  !(hca_out->validity_expression = strdup(hca->validity)))
			{
				status = WFR_STATUS_FROM_ERRNO();
				WFR_FREE_IF_NOTNULL(hca_out->name);
				if (hca_out->ca_public_key) {
					strbuf_free(hca_out->ca_public_key);
				}
				WFR_FREE_IF_NOTNULL(hca_out->validity_expression);
				WFR_FREE(hca_out);
				return NULL;
			} else {
				hca_out->opts.permit_rsa_sha1 = hca->permit_rsa_sha1 ? 1 : 0;
				hca_out->opts.permit_rsa_sha256 = hca->permit_rsa_sha256 ? 1 : 0;
				hca_out->opts.permit_rsa_sha512 = hca->permit_rsa_sha512 ? 1 : 0;
				return hca_out;
			}
		}
	} else {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "loading host CA");
		return NULL;
	}
}

char *
host_ca_save(
	host_ca *	hca
	)
{
	WfsHostCA	hca_storage;
	WfrStatus	status;


	WFS_HOST_CA_INIT(hca_storage);
	hca_storage.public_key = hca->ca_public_key->s;
	hca_storage.name = hca->name;
	hca_storage.permit_rsa_sha1 = hca->opts.permit_rsa_sha1;
	hca_storage.permit_rsa_sha256 = hca->opts.permit_rsa_sha256;
	hca_storage.permit_rsa_sha512 = hca->opts.permit_rsa_sha512;
	hca_storage.validity = hca->validity_expression;

	status = WfsSaveHostCA(WfsGetBackend(), &hca_storage);

	if (WFR_STATUS_SUCCESS(status)) {
		return NULL;
	} else {
		return strdup(WfrStatusToErrorMessage(status));
	}
}

char *
host_ca_delete(
	const char *	name
	)
{
	WfrStatus	status;


	status = WfsDeleteHostCA(WfsGetBackend(), false, name);

	if (WFR_STATUS_SUCCESS(status)) {
		return NULL;
	} else {
		return strdup(WfrStatusToErrorMessage(status));
	}
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
