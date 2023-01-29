/*
 * winfrip_storage.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdlib.h>
#include <string.h>

#include <windows.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_host_keys.h"
#include "PuTTie/winfrip_storage_jump_list.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"
#include "PuTTie/winfrip_storage_backend_ephemeral.h"
#include "PuTTie/winfrip_storage_backend_file.h"
#include "PuTTie/winfrip_storage_backend_registry.h"

/*
 * Private variables
 */

/*
 * Currently selected storage backend
 */

static WfsBackend	WfspBackendCurrent = WFS_DEFAULT_STORAGE_BACKEND;

/*
 * Last effective backend-setting command line argument(s)
 */

static WfsBackend	WfspLastBackend = WFS_DEFAULT_STORAGE_BACKEND,
			WfspLastBackendFrom = WFS_DEFAULT_STORAGE_BACKEND;

/*
 * Private subroutine prototypes
 */

static WfrStatus	WfspInit(WfsBackend backend);
static WfrStatus	WfspSetBackendFromArg(const char *arg, bool *psetfl, WfsBackend *pbackend, WfsBackend *pbackend_from);

/*
 * Private subroutines
 */

static WfrStatus
WfspInit(
	WfsBackend	backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		WFSP_BACKEND_INIT(*backend_impl);

		if (WFR_STATUS_SUCCESS(status = WfsTreeInit(&backend_impl->tree_host_ca))
		&&  WFR_STATUS_SUCCESS(status = WfsTreeInit(&backend_impl->tree_host_key))
		&&  WFR_STATUS_SUCCESS(status = WfsTreeInit(&backend_impl->tree_session))
		&&  WFR_STATUS_SUCCESS(status = backend_impl->Init()))
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else {
			WFR_FREE_IF_NOTNULL(backend_impl->tree_host_key);
			WFR_FREE_IF_NOTNULL(backend_impl->tree_session);
			WFSP_BACKEND_INIT(*backend_impl);
		}
	}

	return status;
}

static WfrStatus
WfspSetBackendFromArg(
	const char *	arg,
	bool *		psetfl,
	WfsBackend *	pbackend,
	WfsBackend *	pbackend_from
	)
{
	const char *	arg_suffix = NULL;
	size_t		arg_suffix_len;
	WfsBackend	backend;
	const char *	backend_arg;
	size_t		backend_arg_len;
	WfrStatus	status;


	backend = WFS_BACKEND_MIN;
	*psetfl = false;

	do {
		if (WFR_STATUS_SUCCESS(status = WfsGetBackendArg(backend, &backend_arg)))
		{
			backend_arg_len = strlen(backend_arg);
			if ((arg[0] == '-') && (arg[1] == '-')
			&&  (strncmp(&arg[2], backend_arg, backend_arg_len) == 0))
			{
				arg_suffix = &arg[2 + backend_arg_len];
				*psetfl = true;
				*pbackend = *pbackend_from = backend;
				status = WFR_STATUS_CONDITION_SUCCESS;
				break;
			}
		}
	} while (WFR_STATUS_SUCCESS(status) && WfsGetBackendNext(&backend));

	if (WFR_STATUS_SUCCESS(status)
	&&  arg_suffix && ((arg_suffix_len = strlen(arg_suffix)) > 0))
	{
		backend = WFS_BACKEND_MIN;
		*psetfl = false;
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);

		do {
			if (WFR_STATUS_SUCCESS(status = WfsGetBackendArg(backend, &backend_arg)))
			{
				if ((arg_suffix[0] == '=')
				&&  (strcmp(&arg_suffix[1], backend_arg) == 0))
				{
					*pbackend_from = backend;
					*psetfl = true;
					status = WFR_STATUS_CONDITION_SUCCESS;
					break;
				}
			}
		} while (WFR_STATUS_SUCCESS(status) && WfsGetBackendNext(&backend));
	}

	return status;
}

/*
 * Public storage backend subroutines private to PuTTie/winfrip_storage*.c
 */

WfsBackend
WfsGetBackend(
	void
	)
{
	return WfspBackendCurrent;
}

WfrStatus
WfsGetBackendArg(
	WfsBackend	backend,
	const char **	parg
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl)))
	{
		*parg = backend_impl->arg;
	}

	return status;
}

WfrStatus
WfsGetBackendArgString(
	char **		parg_string
	)
{
	WfsBackend	backend;
	const char *	backend_name, *backend_from_name;
	bool		fromfl;
	WfrStatus	status;


	backend = WfsGetBackend();
	if (backend == WfspLastBackend) {
		fromfl = (WfspLastBackend != WfspLastBackendFrom);
	} else {
		fromfl = false;
	}

	if (WFR_STATUS_SUCCESS(status = WfsGetBackendArg(backend, &backend_name))
	&&  WFR_STATUS_SUCCESS(status = WfsGetBackendArg(WfspLastBackendFrom, &backend_from_name)))
	{
		status = WfrSnDuprintf(
			parg_string, NULL, "--%s%s%s",
			backend_name,
			fromfl ? "=" : "",
			fromfl ? backend_from_name : "");
	}

	return status;
}

WfrStatus
WfsGetBackendName(
	WfsBackend	backend,
	const char **	pname
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl)))
	{
		*pname = backend_impl->name;
	}

	return status;
}

bool
WfsGetBackendNext(
	WfsBackend *	pbackend
	)
{
	if (*pbackend < WFS_BACKEND_MAX) {
		(*pbackend)++;
		return true;
	} else {
		return false;
	}
}

WfrStatus
WfsSetBackend(
	WfsBackend	new_backend,
	WfsBackend	new_backend_from,
	bool		reset
	)
{
	void		(*error_fn_host_ca)(const char *, WfrStatus) =
			WFR_LAMBDA(void, (const char *name, WfrStatus status) {
				WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "exporting host CA %s", name);
			});
	void		(*error_fn_host_key)(const char *, WfrStatus) =
			WFR_LAMBDA(void, (const char *key_name, WfrStatus status) {
				WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "exporting host key %s", key_name);
			});
	void		(*error_fn_session)(const char *, WfrStatus) =
			WFR_LAMBDA(void, (const char *sessionname, WfrStatus status) {
				WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "exporting session %s", sessionname);
			});

	WfspBackend *	new_backend_impl;
	WfrStatus	status;


	if (reset || (WfspBackendCurrent != new_backend)) {
		if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(new_backend, &new_backend_impl))) {
			status = new_backend_impl->SetBackend(new_backend);
			if ((new_backend_from != new_backend)
			&&  WFR_STATUS_SUCCESS(status))
			{
				if (WFR_STATUS_SUCCESS(status = WfsExportSessions(new_backend_from, new_backend, true, true, error_fn_session))
				&&  WFR_STATUS_SUCCESS(status = WfsExportHostCAs(new_backend_from, new_backend, true, true, error_fn_host_ca))
				&&  WFR_STATUS_SUCCESS(status = WfsExportHostKeys(new_backend_from, new_backend, true, true, error_fn_host_key))
				&&  WFR_STATUS_SUCCESS(status = WfsExportJumpList(new_backend_from, new_backend, false)))
				{
					status = WFR_STATUS_CONDITION_SUCCESS;
				}
			}
		}

		if (WFR_STATUS_SUCCESS(status)) {
			WfspBackendCurrent = new_backend;
		}
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfsSetBackendFromArgV(
	int *		pargc,
	char ***	pargv
	)
{
	char *		arg;
	int		argc_new = 0, argc_new_;
	char **		argv_new;
	WfsBackend	backend = WFS_DEFAULT_STORAGE_BACKEND, backend_from = WFS_DEFAULT_STORAGE_BACKEND;
	int		narg, narg_new;
	bool		setfl;
	WfrStatus	status;


	argc_new_ = *pargc;
	if ((argv_new = WFR_NEWN(argc_new_, char *))) {
		memset(argv_new, 0, argc_new_ * sizeof(*argv_new));

		status = WFR_STATUS_CONDITION_SUCCESS;
		for (narg = 0, narg_new = 0, setfl = false;
			 (narg < (*pargc)) && WFR_STATUS_SUCCESS(status);
			 narg++, setfl = false)
		{
			arg = (*pargv)[narg];
			status = WfspSetBackendFromArg(arg, &setfl, &backend, &backend_from);

			if (WFR_STATUS_SUCCESS(status)) {
				if (!setfl) {
					argc_new++;
					argv_new[narg_new++] = arg;
				}
			}
		}

		if (WFR_STATUS_SUCCESS(status)) {
			if (WFR_STATUS_FAILURE(status = WFR_RESIZE_IF_NEQ_SIZE(
					argv_new, argc_new_, argc_new, char *)))
			{
				WFR_FREE(argv_new);
			} else if (WFR_STATUS_FAILURE(status = WfsSetBackend(backend, backend_from, true))) {
				WFR_FREE(argv_new);
			} else {
				*pargc = argc_new_; *pargv = argv_new;
				WfspLastBackend = backend; WfspLastBackendFrom = backend_from;
			}
		}
	} else {
		status = WFR_STATUS_FROM_ERRNO();
	}

	return status;
}

WfrStatus
WfsSetBackendFromCmdLine(
	char *	cmdline
	)
{
	char *		arg, *arg_, *arg_full, *arg_next;
	size_t		arg_len;
	WfsBackend	backend = WFS_DEFAULT_STORAGE_BACKEND, backend_from = WFS_DEFAULT_STORAGE_BACKEND;
	bool		setfl;
	WfrStatus	status;


	status = WFR_STATUS_CONDITION_SUCCESS;
	for (arg = cmdline, setfl = false;
		 arg && arg[0] && WFR_STATUS_SUCCESS(status);
		 arg = arg_next, setfl = false)
	{
		arg_full = arg;
		while (*arg == ' ') {
			arg++;
		}
		arg_next = strchr(arg, ' ');
		arg_len = (arg_next ? (size_t)(arg_next - arg) : strlen(arg));

		if (!(arg_ = WFR_NEWN(arg_len + 1, char))) {
			status = WFR_STATUS_FROM_ERRNO(); break;
		} else {
			memcpy(arg_, arg, arg_len); arg_[arg_len] = '\0';
			status = WfspSetBackendFromArg(arg_, &setfl, &backend, &backend_from);
			WFR_FREE(arg_);
		}

		if (WFR_STATUS_SUCCESS(status)) {
			if (setfl) {
				if (arg_next) {
					memmove(arg_full, arg_next, strlen(arg_next) + 1);
					arg_next = arg_full;
				} else {
					memmove(arg_full, &arg_full[strlen(arg_full)], 1);
				}
			}
		}
	}

	if (WFR_STATUS_SUCCESS(status)) {
		status = WfsSetBackend(backend, backend_from, true);
		if (WFR_STATUS_SUCCESS(status)) {
			WfspLastBackend = backend; WfspLastBackendFrom = backend_from;
		}
	}

	return status;
}

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfsCleanupContainer(
	WfsBackend	backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->CleanupContainer(backend);
	}

	return status;
}

WfrStatus
WfsInit(
	void
	)
{
	WfsBackend	backend;
	WfrStatus	status;


	backend = WFS_BACKEND_MIN;
	do {
		status = WfspInit(backend);
	} while (WFR_STATUS_SUCCESS(status) && WfsGetBackendNext(&backend));

	if (WFR_STATUS_SUCCESS(status)) {
		status = WfsSetBackend(WfspBackendCurrent, WfspBackendCurrent, true);
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
