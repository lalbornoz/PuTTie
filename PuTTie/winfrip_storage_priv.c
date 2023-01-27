/*
 * winfrip_storage_priv.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include "PuTTie/winfrip_rtl_status.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "PuTTie/winfrip_storage_wrap.h"
#include "storage.h"
#include "PuTTie/winfrip_storage_unwrap.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_priv.h"
#include "PuTTie/winfrip_storage_backend_ephemeral.h"
#include "PuTTie/winfrip_storage_backend_file.h"
#include "PuTTie/winfrip_storage_backend_registry.h"

/*
 * Private variables
 */

/*
 * Storage backends for the ephemeral, file, and registry backends
 */

static WfspBackend	WfspBackends[WFS_BACKEND_MAX + 1] = {
	[WFS_BACKEND_EPHEMERAL]		= WFSP_EPHEMERAL_BACKEND,
	[WFS_BACKEND_FILE]		= WFSP_FILE_BACKEND,
	[WFS_BACKEND_REGISTRY]		= WFSP_REGISTRY_BACKEND,
};

/*
 * Public storage backend subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfsGetBackendImpl(
	WfsBackend	backend,
	void *		pbackend
	)
{
	WfrStatus	status;


	if (((backend) < WFS_BACKEND_MIN)
	||  ((backend) > WFS_BACKEND_MAX)) {
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
	} else {
		*((WfspBackend **)pbackend) = &WfspBackends[(backend)];
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
