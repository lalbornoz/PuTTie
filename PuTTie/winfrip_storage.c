/*
 * winfrip_storage.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <windows.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_windows.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_host_keys.h"
#include "PuTTie/winfrip_storage_jump_list.h"
#include "PuTTie/winfrip_storage_options.h"
#include "PuTTie/winfrip_storage_privkey_list.h"
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
static WfrStatus	WfspSetBackendFromArg(char *arg, WfsBackend *pbackend, WfsBackend *pbackend_from, const char **pbackend_arg, char **pbackend_args_extra);

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


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		WFSP_BACKEND_INIT(*backend_impl);

		if (WFR_SUCCESS(status = WfrTreeInit(&backend_impl->tree_host_ca))
		&&  WFR_SUCCESS(status = WfrTreeInit(&backend_impl->tree_host_key))
		&&  WFR_SUCCESS(status = WfrTreeInit(&backend_impl->tree_options))
		&&  WFR_SUCCESS(status = WfrTreeInit(&backend_impl->tree_session))
		&&  WFR_SUCCESS(status = backend_impl->Init()))
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
	char *		arg,
	WfsBackend *	pbackend,
	WfsBackend *	pbackend_from,
	const char **	pbackend_arg,
	char **		pbackend_args_extra
	)
{
	char *		arg_suffix = NULL;
	size_t		arg_suffix_len;
	WfsBackend	backend;
	const char *	backend_arg;
	size_t		backend_arg_len;
	WfrStatus	status;


	backend = WFS_BACKEND_MIN;
	*pbackend_arg = NULL;
	*pbackend_args_extra = NULL;
	status = WFR_STATUS_FROM_ERRNO1(EINVAL);

	do {
		if (WFR_SUCCESS(status = WfsGetBackendArg(backend, &backend_arg)))
		{
			backend_arg_len = strlen(backend_arg);
			if ((arg[0] == '-') && (arg[1] == '-')
			&&  (strncmp(&arg[2], backend_arg, backend_arg_len) == 0))
			{
				arg_suffix = &arg[2 + backend_arg_len];
				*pbackend = *pbackend_from = backend;
				*pbackend_arg = backend_arg;
				status = WFR_STATUS_CONDITION_SUCCESS;
				break;
			}
		}
	} while (WFR_SUCCESS(status) && WfsGetBackendNext(&backend));

	if (WFR_SUCCESS(status)
	&&  (arg_suffix != NULL)
	&&  ((arg_suffix_len = strlen(arg_suffix)) > 0))
	{
		if ((arg_suffix[0] == '=')
		&&  (arg_suffix[1] == ';'))
		{
			*pbackend_args_extra = &arg_suffix[2];
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else if (arg_suffix[0] == '=') {
			backend = WFS_BACKEND_MIN;
			do {
				if (WFR_SUCCESS(status = WfsGetBackendArg(backend, &backend_arg)))
				{
					backend_arg_len = strlen(backend_arg);
					if (strncmp(&arg_suffix[1], backend_arg, backend_arg_len) == 0) {
						if (arg_suffix[1 + backend_arg_len] == ';') {
							*pbackend_args_extra = &arg_suffix[1 + backend_arg_len + 1];
						}

						*pbackend_from = backend;
						status = WFR_STATUS_CONDITION_SUCCESS;
						break;
					}
				}
			} while (WFR_SUCCESS(status) && WfsGetBackendNext(&backend));
		} else {
			status = WFR_STATUS_FROM_ERRNO1(EINVAL);
		}
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


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl)))
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

	if (WFR_SUCCESS(status = WfsGetBackendArg(backend, &backend_name))
	&&  WFR_SUCCESS(status = WfsGetBackendArg(WfspLastBackendFrom, &backend_from_name)))
	{
		status = WfrSnDuprintF(
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


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl)))
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
	bool		reset,
	char *		args_extra
	)
{
	WfsErrorFn	error_fn =
			WFR_LAMBDA(void, (const char *name, WfrStatus status) {
				WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, NULL, "exporting %s", name);
			});
	WfspBackend *	new_backend_impl;
	WfrStatus	status;


	if (reset || (WfspBackendCurrent != new_backend)) {
		if (WFR_SUCCESS(status = WfsGetBackendImpl(new_backend, &new_backend_impl))) {
			status = new_backend_impl->SetBackend(new_backend, args_extra);
			if ((new_backend_from != new_backend)
			&&  WFR_SUCCESS(status))
			{
				if (WFR_SUCCESS(status = WfsExportHostCAs(new_backend_from, new_backend, true, true, error_fn))
				&&  WFR_SUCCESS(status = WfsExportHostKeys(new_backend_from, new_backend, true, true, error_fn))
				&&  WFR_SUCCESS(status = WfsExportJumpList(new_backend_from, new_backend, true, true, error_fn))
				&&  WFR_SUCCESS(status = WfsExportOptions(new_backend_from, new_backend, true, true, error_fn))
				&&  WFR_SUCCESS(status = WfsExportPrivKeyList(new_backend_from, new_backend, true, true, error_fn))
				&&  WFR_SUCCESS(status = WfsExportSessions(new_backend_from, new_backend, true, true, error_fn)))
				{
					status = WFR_STATUS_CONDITION_SUCCESS;
				}
			}
		}

		if (WFR_SUCCESS(status)
		&&  WFR_SUCCESS(status = WfsClearOptions(new_backend, false))
		&&  WFR_SUCCESS(status = WfsLoadOptions(new_backend)))
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		if (WFR_SUCCESS(status)) {
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
	const char *	backend_arg;
	char *		backend_args_extra = NULL, *backend_args_extra_new;
	char **		argv_new;
	WfsBackend	backend = WFS_DEFAULT_STORAGE_BACKEND;
	WfsBackend	backend_from = WFS_DEFAULT_STORAGE_BACKEND;
	int		narg, narg_new;
	WfrStatus	status;


	argc_new_ = *pargc;

	if (WFR_NEWN(status, argv_new, argc_new_, char *)) {
		memset(argv_new, 0, argc_new_ * sizeof(*argv_new));
		for (narg = 0, narg_new = 0;
		     (narg < (*pargc)) && WFR_SUCCESS(status);
		     narg++)
		{
			arg = (*pargv)[narg];

			status = WfspSetBackendFromArg(
				arg, &backend, &backend_from,
				&backend_arg, &backend_args_extra_new);

			if (WFR_SUCCESS(status) && (backend_args_extra_new != NULL)) {
				if (!backend_arg) {
					argc_new++;
					argv_new[narg_new++] = arg;
				}

				WFR_FREE_IF_NOTNULL(backend_args_extra);
				if (backend_args_extra_new != NULL) {
					WFR_STATUS_BIND_POSIX(status,
						(backend_args_extra = strdup(
							backend_args_extra_new)));
				} else {
					backend_args_extra = NULL;
				}
			}
		}

		if (WFR_SUCCESS(status)) {
			if (!WFR_RESIZE_IF_NEQ_SIZE(
					status, argv_new, argc_new_,
					argc_new, char *)
			||  WFR_FAILURE(status = WfsSetBackend(
					backend, backend_from,
					true, backend_args_extra)))
			{
				WFR_FREE(argv_new);
				WFR_FREE_IF_NOTNULL(backend_args_extra);
			} else {
				*pargc = argc_new_;
				*pargv = argv_new;
				WfspLastBackend = backend;
				WfspLastBackendFrom = backend_from;
			}
		} else {
			WFR_FREE_IF_NOTNULL(backend_args_extra);
		}
	}

	return status;
}

WfrStatus
WfsSetBackendFromCmdLine(
	char *	cmdline
	)
{
	char *		arg, *arg_, *arg_full, *arg_next;
	const char *	backend_arg;
	char *		backend_args_extra = NULL, *backend_args_extra_new;
	size_t		arg_len;
	WfsBackend	backend = WFS_DEFAULT_STORAGE_BACKEND;
	WfsBackend	backend_from = WFS_DEFAULT_STORAGE_BACKEND;
	WfrStatus	status;


	for (arg = cmdline, status = WFR_STATUS_CONDITION_SUCCESS;
	     arg && arg[0] && WFR_SUCCESS(status);
	     arg = arg_next)
	{
		arg_full = arg;
		while (*arg == ' ') {
			arg++;
		}
		arg_next = strchr(arg, ' ');
		arg_len = (arg_next ? (size_t)(arg_next - arg) : strlen(arg));

		if (WFR_NEWN(status, arg_, arg_len + 1, char)) {
			memcpy(arg_, arg, arg_len); arg_[arg_len] = '\0';

			status = WfspSetBackendFromArg(
				arg_, &backend, &backend_from,
				&backend_arg, &backend_args_extra_new);

			if (WFR_SUCCESS(status)) {
				if (backend_arg != NULL) {
					if ((arg_next != NULL) && (arg_full == cmdline)) {
						memmove(arg_full, arg_next + 1, strlen(arg_next));
						arg_next = arg_full;
					} else if ((arg_next != NULL) && (arg_full != cmdline)) {
						memmove(arg_full, arg_next, strlen(arg_next) + 1);
						arg_next = arg_full;
					} else {
						memmove(arg_full, &arg_full[strlen(arg_full)], 1);
					}
				}

				if (backend_args_extra_new != NULL) {
					WFR_FREE_IF_NOTNULL(backend_args_extra);
					if (backend_args_extra_new != NULL) {
						WFR_STATUS_BIND_POSIX(status,
							(backend_args_extra = strdup(
								backend_args_extra_new)));
					} else {
						backend_args_extra = NULL;
					}
				}
			}

			WFR_FREE(arg_);
		}
	}

	if (WFR_SUCCESS(status)
	&&  WFR_SUCCESS(status = WfsSetBackend(
			backend, backend_from,
			true, backend_args_extra)))
	{
		WfspLastBackend = backend;
		WfspLastBackendFrom = backend_from;
	}

	if (WFR_FAILURE(status)) {
		WFR_FREE_IF_NOTNULL(backend_args_extra);
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


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->CleanupContainer(backend);
	}

	return status;
}

WfrStatus
WfsEnumerateCancel(
	WfsBackend	backend,
	void **		pstate
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->EnumerateCancel(backend, pstate);
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
	} while (WFR_SUCCESS(status) && WfsGetBackendNext(&backend));

	if (WFR_SUCCESS(status)) {
		status = WfsSetBackend(WfspBackendCurrent, WfspBackendCurrent, true, NULL);
	}

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
