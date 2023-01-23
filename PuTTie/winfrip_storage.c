/*
 * winfrip_storage.c - pointless frippery & tremendous amounts of bloat
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
#include "PuTTie/winfrip_storage_backend_ephemeral.h"
#include "PuTTie/winfrip_storage_backend_file.h"
#include "PuTTie/winfrip_storage_backend_registry.h"

/*
 * Private types
 */

/*
 * Storage backend implementation type definitions
 */

typedef struct WfspBackend {
	const char *	name;
	const char *	arg;

	WfrStatus		(*ClearHostKeys)(WfsBackend);
	WfrStatus		(*DeleteHostKey)(WfsBackend, const char *);
	WfrStatus		(*EnumerateHostKeys)(WfsBackend, bool, bool *, const char **, void *);
	WfrStatus		(*LoadHostKey)(WfsBackend, const char *, const char **);
	WfrStatus		(*RenameHostKey)(WfsBackend, const char *, const char *);
	WfrStatus		(*SaveHostKey)(WfsBackend, const char *, const char *);

	WfrStatus		(*ClearSessions)(WfsBackend);
	WfrStatus		(*CloseSession)(WfsBackend, WfspSession *);
	WfrStatus		(*DeleteSession)(WfsBackend, const char *);
	WfrStatus		(*EnumerateSessions)(WfsBackend, bool, bool *, char **, void *);
	WfrStatus		(*LoadSession)(WfsBackend, const char *, WfspSession **);
	WfrStatus		(*RenameSession)(WfsBackend, const char *, const char *);
	WfrStatus		(*SaveSession)(WfsBackend, WfspSession *);

	void			(*JumpListAdd)(const char *const);
	WfrStatus		(*JumpListCleanup)(void);
	void			(*JumpListClear)(void);
	WfrStatus		(*JumpListGetEntries)(char **, size_t *);
	void			(*JumpListRemove)(const char *const);
	WfrStatus		(*JumpListSetEntries)(const char *, size_t);

	WfrStatus		(*Init)(void);
	WfrStatus		(*SetBackend)(WfsBackend);

	tree234 *		tree_host_key, *tree_session;
} WfspBackend;

#define WFSP_BACKEND_INIT(backend)						\
		(backend).tree_host_key = NULL;					\
		(backend).tree_session = NULL;

#define WFSP_BACKEND_GET_IMPL(backend, pbackend) ({		\
		WfrStatus	status;								\
														\
		if (((backend) < WFS_BACKEND_MIN)				\
		||  ((backend) > WFS_BACKEND_MAX)) {			\
			status = WFR_STATUS_FROM_ERRNO1(EINVAL);	\
		} else {										\
			*(pbackend) = &WfspBackends[(backend)];		\
			status = WFR_STATUS_CONDITION_SUCCESS;		\
		}												\
														\
		status;											\
	})

/*
 * Private variables
 */

/*
 * Currently selected storage backend
 */

static WfsBackend	WfspBackendCurrent = WFS_DEFAULT_STORAGE_BACKEND;

/*
 * Storage backends for the ephemeral, file, and registry backends
 */

static WfspBackend	WfspBackends[WFS_BACKEND_MAX + 1] = {
	[WFS_BACKEND_EPHEMERAL]		= WFSP_EPHEMERAL_BACKEND,
	[WFS_BACKEND_FILE]			= WFSP_FILE_BACKEND,
	[WFS_BACKEND_REGISTRY]		= WFSP_REGISTRY_BACKEND,
};

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
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		WFSP_BACKEND_INIT(*backend_impl);

		if (WFR_STATUS_SUCCESS(status = WfspTreeInit(&backend_impl->tree_host_key))
		&&  WFR_STATUS_SUCCESS(status = WfspTreeInit(&backend_impl->tree_session))
		&&  WFR_STATUS_SUCCESS(status = backend_impl->Init()))
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else {
			WFR_SFREE_IF_NOTNULL(backend_impl->tree_host_key);
			WFR_SFREE_IF_NOTNULL(backend_impl->tree_session);
			WFSP_BACKEND_INIT(*backend_impl);
		}
	}

	return status;
}

static WfrStatus
WfspSetBackendFromArg(
	const char *	arg,
	bool *			psetfl,
	WfsBackend *	pbackend,
	WfsBackend *	pbackend_from
	)
{
	const char *	arg_suffix = NULL;
	size_t			arg_suffix_len;
	WfsBackend		backend;
	const char *	backend_arg;
	size_t			backend_arg_len;
	WfrStatus		status;


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
	WfsBackend		backend,
	const char **	parg
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl)))
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
	WfsBackend		backend;
	const char *	backend_name, *backend_from_name;
	bool			fromfl;
	WfrStatus		status;


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
	WfsBackend		backend,
	const char **	pname
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl)))
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
	void			(*error_fn_host_key)(const char *, WfrStatus) =
		WFR_LAMBDA(void, (const char *key_name, WfrStatus status) {
			(void)key_name;
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "exporting host key %s", key_name);
		});
	void			(*error_fn_session)(const char *, WfrStatus) =
		WFR_LAMBDA(void, (const char *sessionname, WfrStatus status) {
			(void)sessionname;
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "exporting session %s", sessionname);
		});

	WfspBackend *	new_backend_impl;
	WfrStatus		status;


	if (reset || (WfspBackendCurrent != new_backend)) {
		if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(
						new_backend, &new_backend_impl)))
		{
			status = new_backend_impl->SetBackend(new_backend);
			if ((new_backend_from != new_backend)
			&&  WFR_STATUS_SUCCESS(status))
			{
				if (WFR_STATUS_SUCCESS(status = WfsExportSessions(new_backend_from, new_backend, true, true, error_fn_session))
				&&  WFR_STATUS_SUCCESS(status = WfsExportHostKeys(new_backend_from, new_backend, true, true, error_fn_host_key)))
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
	int			argc_new = 0;
	char **		argv_new, **argv_new_;
	WfsBackend	backend = WFS_DEFAULT_STORAGE_BACKEND, backend_from = WFS_DEFAULT_STORAGE_BACKEND;
	int			narg, narg_new;
	bool		setfl;
	WfrStatus	status;


	if ((argv_new = snewn(*pargc, char *))) {
		memset(argv_new, 0, argc_new * sizeof(*argv_new));

		status = WFR_STATUS_CONDITION_SUCCESS;
		for (narg = 0, narg_new = 0, setfl = false;
			 (narg < (*pargc)) && WFR_STATUS_SUCCESS(status);
			 narg++, setfl = false)
		{
			arg = (*pargv)[narg];
			status = WfspSetBackendFromArg(
						arg, &setfl, &backend, &backend_from);

			if (WFR_STATUS_SUCCESS(status)) {
				if (!setfl) {
					argc_new++;
					argv_new[narg_new++] = arg;
				}
			}
		}

		if (WFR_STATUS_SUCCESS(status)) {
			if (!(argv_new_ = sresize(argv_new, argc_new, char *))) {
				status = WFR_STATUS_FROM_ERRNO();
				sfree(argv_new);
			} else if (WFR_STATUS_FAILURE(status = WfsSetBackend(backend, backend_from, true))) {
				sfree(argv_new);
			} else {
				argv_new = argv_new_;
				*pargc = argc_new; *pargv = argv_new;
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

		if (!(arg_ = snewn(arg_len + 1, char))) {
			status = WFR_STATUS_FROM_ERRNO(); break;
		} else {
			memcpy(arg_, arg, arg_len); arg_[arg_len] = '\0';
			status = WfspSetBackendFromArg(
						arg_, &setfl, &backend, &backend_from);
			sfree(arg_);
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
 * Public host key storage subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfsClearHostKeys(
	WfsBackend	backend,
	bool		delete_in_backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status = WFR_STATUS_CONDITION_SUCCESS;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		if (delete_in_backend) {
			status = backend_impl->ClearHostKeys(backend);
		}

		if (WFR_STATUS_SUCCESS(status)) {
			status = WfspTreeClear(&backend_impl->tree_host_key);
		}
	}

	return status;
}

WfrStatus
WfsDeleteHostKey(
	WfsBackend		backend,
	bool			delete_in_backend,
	const char *	key_name
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		if (delete_in_backend) {
			status = backend_impl->DeleteHostKey(backend, key_name);
		}

		if (WFR_STATUS_SUCCESS(status)) {
			status = WfspTreeDelete(
					backend_impl->tree_host_key, NULL,
					key_name, WFSP_TREE_ITYPE_HOST_KEY);

			if ((WFR_STATUS_CONDITION(status) == ENOENT)
			&&  delete_in_backend)
			{
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		}
	}

	return status;
}

WfrStatus
WfsEnumerateHostKeys(
	WfsBackend		backend,
	bool			cached,
	bool			initfl,
	bool *			pdonefl,
	const char **	pkey_name,
	void *			state
	)
{
	WfspBackend *	backend_impl = NULL;
	WfspTreeItem *	item;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		switch (cached) {
		case true:
			status = WfspTreeEnumerate(
						backend_impl->tree_host_key,
						initfl, pdonefl, &item, state);
			if (!initfl && WFR_STATUS_SUCCESS(status) && !(*pdonefl)) {
				if (!(*pkey_name = strdup(item->key))) {
					status = WFR_STATUS_FROM_ERRNO();
				}
			}
			break;

		case false:
			status = backend_impl->EnumerateHostKeys(
						backend, initfl, pdonefl,
						pkey_name, state);
			break;
		}
	}

	return status;
}

WfrStatus
WfsExportHostKey(
	WfsBackend		backend_from,
	WfsBackend		backend_to,
	bool			movefl,
	const char *	key_name
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	const char *	key;
	WfrStatus		status;


	if (WFR_STATUS_FAILURE(status = WFSP_BACKEND_GET_IMPL(backend_from, &backend_from_impl))
	||  WFR_STATUS_FAILURE(status = WFSP_BACKEND_GET_IMPL(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_STATUS_SUCCESS(status = backend_from_impl->LoadHostKey(backend_to, key_name, &key))
	&&  WFR_STATUS_SUCCESS(status = backend_to_impl->SaveHostKey(backend_to, key_name, key)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_SUCCESS(status) && movefl) {
		status = WfsDeleteHostKey(backend_from, true, key_name);
	}

	return status;
}

WfrStatus
WfsExportHostKeys(
	WfsBackend	backend_from,
	WfsBackend	backend_to,
	bool		clear_to,
	bool		continue_on_error,
	void		(*error_fn)(const char *, WfrStatus)
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	bool			donefl = false;
	void *			enum_state = NULL;
	const char *	key_name;
	WfrStatus		status;


	if (WFR_STATUS_FAILURE(status = WFSP_BACKEND_GET_IMPL(backend_from, &backend_from_impl))
	||  WFR_STATUS_FAILURE(status = WFSP_BACKEND_GET_IMPL(backend_to, &backend_to_impl))) {
		return status;
	}

	status = WfsEnumerateHostKeys(
			backend_from, false, true, &donefl,
			&key_name, &enum_state);

	if (WFR_STATUS_SUCCESS(status) && clear_to) {
		status = WfsClearHostKeys(backend_to, false);
	}

	if (WFR_STATUS_SUCCESS(status)) {
		do {
			key_name = NULL;
			status = WfsEnumerateHostKeys(
					backend_from, false, false,
					&donefl, &key_name, enum_state);

			if (WFR_STATUS_SUCCESS(status) && key_name) {
				status = WfsExportHostKey(
							backend_from, backend_to,
							false, key_name);
			}

			if (WFR_STATUS_FAILURE(status)) {
				error_fn(key_name, status);
			}
		} while(!donefl && (WFR_STATUS_SUCCESS(status) || continue_on_error));
	}

	if (WFR_STATUS_FAILURE(status) && continue_on_error) {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_FAILURE(status)) {
		(void)WfsClearHostKeys(backend_to, false);
	}

	WFR_SFREE_IF_NOTNULL(enum_state);

	return status;
}

WfrStatus
WfsGetHostKey(
	WfsBackend		backend,
	bool			cached,
	const char *	key_name,
	const char **	pkey
	)
{
	WfspBackend *	backend_impl;
	WfspTreeItem *	item;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		switch (cached) {
		case true:
			status = WfspTreeGet(
						backend_impl->tree_host_key,
						key_name, WFSP_TREE_ITYPE_HOST_KEY, &item);
			if (WFR_STATUS_SUCCESS(status)) {
				*pkey = item->value;
			}
			break;

		case false:
			status = backend_impl->LoadHostKey(backend, key_name, pkey);
			break;
		}
	}

	return status;
}

WfrStatus
WfsPrintHostKeyName(
	const char *	hostname,
	int				port,
	const char *	keytype,
	char **			pkey_name
	)
{
	char *		key_name;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfrSnDuprintf(
					&key_name, NULL, "%s@%u:%s",
					keytype, port, hostname)))
	{
		*pkey_name = key_name;
	}

	return status;
}

WfrStatus
WfsRenameHostKey(
	WfsBackend		backend,
	bool			rename_in_backend,
	const char *	key_name,
	const char *	key_name_new
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		status = WfspTreeRename(
					backend_impl->tree_host_key, NULL, key_name,
					WFSP_TREE_ITYPE_HOST_KEY, key_name_new);

		if (rename_in_backend
		&&  (WFR_STATUS_SUCCESS(status)
		||   (WFR_STATUS_CONDITION(status) == ENOENT)))
		{
			status = backend_impl->RenameHostKey(backend, key_name, key_name_new);
		}
	}

	return status;
}

WfrStatus
WfsSaveHostKey(
	WfsBackend		backend,
	const char *	key_name,
	const char *	key
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		status = WfspBackends[backend].SaveHostKey(backend, key_name, key);
	}

	return status;
}

WfrStatus
WfsSetHostKey(
	WfsBackend		backend,
	const char *	key_name,
	const char *	key
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		status = WfspTreeSet(
					backend_impl->tree_host_key, key_name,
					WFSP_TREE_ITYPE_HOST_KEY, (void *)key,
					strlen(key) + 1);
	}

	return status;
}

/*
 * Public session storage subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfsAddSession(
	WfsBackend		backend,
	const char *	sessionname,
	WfspSession **	psession
	)
{
	WfspBackend *	backend_impl;
	WfspSession *	session_new = NULL;
	const char *	sessionname_new;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		if (!(session_new = snew(WfspSession))
		||  !(sessionname_new = strdup(sessionname)))
		{
			WFR_SFREE_IF_NOTNULL(session_new);
			status = WFR_STATUS_FROM_ERRNO();
		} else {
			WFSP_SESSION_INIT(*session_new);
			session_new->name = sessionname_new;

			if (WFR_STATUS_SUCCESS(status = WfspTreeInit(&session_new->tree))
			&&  WFR_STATUS_SUCCESS(status = WfspTreeSet(
							backend_impl->tree_session, sessionname,
							WFSP_TREE_ITYPE_SESSION, session_new,
							sizeof(*session_new))))
			{
				status = WFR_STATUS_CONDITION_SUCCESS;
			}

			if (WFR_STATUS_SUCCESS(status)) {
				*psession = session_new;
			} else {
				sfree(session_new);
				sfree((void *)sessionname_new);
			}
		}
	}

	return status;
}

WfrStatus
WfsClearSession(
	WfsBackend		backend,
	WfspSession *	session,
	const char *	sessionname
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		if (WFR_STATUS_SUCCESS(status = WfsGetSession(backend, true, sessionname, &session))) {
			status = WfspTreeClear(&session->tree);
		}
	}

	return status;
}

WfrStatus
WfsClearSessions(
	WfsBackend	backend,
	bool		delete_in_backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status = WFR_STATUS_CONDITION_SUCCESS;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		if (delete_in_backend) {
			status = backend_impl->ClearSessions(backend);
		}

		if (WFR_STATUS_SUCCESS(status)) {
			status = WfspTreeClear(&backend_impl->tree_session);
		}
	}

	return status;
}

WfrStatus
WfsCloseSession(
	WfsBackend		backend,
	WfspSession *	session
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		status = backend_impl->CloseSession(backend, session);
	}

	return status;
}

WfrStatus
WfsCopySession(
	WfsBackend		backend_from,
	WfsBackend		backend_to,
	const char *	sessionname,
	WfspSession *	session,
	WfspSession **	psession
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	WfspSession *	session_to;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend_from, &backend_from_impl))
	&&  WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend_to, &backend_to_impl)))
	{
		if (!session) {
			status = WfsGetSession(backend_from, true, sessionname, &session);
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		if (WFR_STATUS_SUCCESS(status)
		&&  WFR_STATUS_SUCCESS(status = WfsAddSession(backend_to, sessionname, &session_to))
		&&  WFR_STATUS_SUCCESS(status = WfspTreeCopy(session->tree, session_to->tree)))
		{
			*psession = session_to;
		}
	}

	return status;
}

WfrStatus
WfsDeleteSession(
	WfsBackend		backend,
	bool			delete_in_backend,
	const char *	sessionname
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		if (delete_in_backend) {
			status = backend_impl->DeleteSession(backend, sessionname);
		}

		if (WFR_STATUS_SUCCESS(status)) {
			status = WfspTreeDelete(
						backend_impl->tree_session,
						NULL, sessionname,
						WFSP_TREE_ITYPE_SESSION);

			if ((WFR_STATUS_CONDITION(status) == ENOENT)
			&&  delete_in_backend)
			{
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		}
	}

	return status;
}

WfrStatus
WfsEnumerateSessions(
	WfsBackend	backend,
	bool		cached,
	bool		initfl,
	bool *		pdonefl,
	char **		psessionname,
	void *		state
	)
{
	WfspBackend *	backend_impl;
	WfspTreeItem *	item;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		switch (cached) {
		case true:
			status = WfspTreeEnumerate(
						backend_impl->tree_session, initfl,
						pdonefl, &item, state);
			if (!initfl && WFR_STATUS_SUCCESS(status) && !(*pdonefl)) {
				if (!(*psessionname = strdup(item->key))) {
					status = WFR_STATUS_FROM_ERRNO();
				}
			}
			break;

		case false:
			status = backend_impl->EnumerateSessions(
						backend, initfl, pdonefl,
						psessionname, state);
			break;
		}
	}

	return status;
}

WfrStatus
WfsExportSession(
	WfsBackend		backend_from,
	WfsBackend		backend_to,
	bool			movefl,
	char *			sessionname
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	WfspSession *	session;
	WfrStatus		status;


	if (WFR_STATUS_FAILURE(status = WFSP_BACKEND_GET_IMPL(backend_from, &backend_from_impl))
	||  WFR_STATUS_FAILURE(status = WFSP_BACKEND_GET_IMPL(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_STATUS_SUCCESS(status = backend_from_impl->LoadSession(backend_to, sessionname, &session))
	&&  WFR_STATUS_SUCCESS(status = backend_to_impl->SaveSession(backend_to, session)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_SUCCESS(status) && movefl) {
		status = WfsDeleteSession(backend_from, true, sessionname);
	}

	return status;
}

WfrStatus
WfsExportSessions(
	WfsBackend	backend_from,
	WfsBackend	backend_to,
	bool		clear_to,
	bool		continue_on_error,
	void		(*error_fn)(const char *, WfrStatus)
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	bool			donefl = false;
	void *			enum_state = NULL;
	char *			sessionname;
	WfrStatus		status;


	if (WFR_STATUS_FAILURE(status = WFSP_BACKEND_GET_IMPL(backend_from, &backend_from_impl))
	||  WFR_STATUS_FAILURE(status = WFSP_BACKEND_GET_IMPL(backend_to, &backend_to_impl))) {
		return status;
	}

	status = WfsEnumerateSessions(
			backend_from, false, true,
			&donefl, &sessionname, &enum_state);
	if (WFR_STATUS_SUCCESS(status) && clear_to) {
		status = WfsClearSessions(backend_to, true);
	}

	if (WFR_STATUS_SUCCESS(status)) {
		do {
			sessionname = NULL;
			status = WfsEnumerateSessions(
					backend_from, false, false,
					&donefl, &sessionname, enum_state);

			if (WFR_STATUS_SUCCESS(status) && sessionname) {
				status = WfsExportSession(
							backend_from, backend_to,
							false, sessionname);
			}

			if (WFR_STATUS_FAILURE(status)) {
				error_fn(sessionname, status);
			}
		} while(!donefl && (WFR_STATUS_SUCCESS(status) || continue_on_error));
	}

	if (WFR_STATUS_FAILURE(status) && continue_on_error) {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_FAILURE(status)) {
		(void)WfsClearSessions(backend_to, true);
	}

	WFR_SFREE_IF_NOTNULL(enum_state);

	return status;
}

WfrStatus
WfsGetSession(
	WfsBackend		backend,
	bool			cached,
	const char *	sessionname,
	WfspSession **	psession
	)
{
	WfspBackend *	backend_impl;
	WfspTreeItem *	item;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		switch (cached) {
		case true:
			status = WfspTreeGet(
						backend_impl->tree_session, sessionname,
						WFSP_TREE_ITYPE_SESSION, &item);
			if (WFR_STATUS_SUCCESS(status)) {
				*psession = item->value;
			}
			break;

		case false:
			status = backend_impl->LoadSession(backend, sessionname, psession);
			break;
		}
	}

	return status;
}

WfrStatus
WfsGetSessionKey(
	WfspSession *		session,
	const char *		key,
	WfspTreeItemType	item_type,
	void **				pvalue,
	size_t *			pvalue_size
	)
{
	WfspTreeItem *	item;
	WfrStatus		status;


	status = WfspTreeGet(session->tree, key, item_type, &item);
	if (WFR_STATUS_SUCCESS(status)) {
		switch (item->type) {
		default:
			status = WFR_STATUS_FROM_ERRNO1(EINVAL); break;

		case WFSP_TREE_ITYPE_HOST_KEY:
		case WFSP_TREE_ITYPE_SESSION:
		case WFSP_TREE_ITYPE_STRING:
			*pvalue = item->value;
			break;

		case WFSP_TREE_ITYPE_INT:
			*(int *)pvalue = *(int *)item->value;
			break;
		}

		if (pvalue_size) {
			*pvalue_size = item->value_size;
		}
	}

	return status;
}

WfrStatus
WfsRenameSession(
	WfsBackend		backend,
	bool			rename_in_backend,
	const char *	sessionname,
	const char *	sessionname_new
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		status = WfspTreeRename(
					backend_impl->tree_session, NULL,
					sessionname, WFSP_TREE_ITYPE_SESSION,
					sessionname_new);

		if (rename_in_backend
		&&  (WFR_STATUS_SUCCESS(status)
		||   (WFR_STATUS_CONDITION(status) == ENOENT)))
		{
			status = backend_impl->RenameSession(
						backend, sessionname, sessionname_new);
		}
	}

	return status;
}

WfrStatus
WfsSaveSession(
	WfsBackend		backend,
	WfspSession *	session
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		status = backend_impl->SaveSession(backend, session);
	}

	return status;
}

WfrStatus
WfsSetSessionKey(
	WfspSession *		session,
	const char *		key,
	void *				value,
	size_t				value_size,
	WfspTreeItemType	item_type
	)
{
	return WfspTreeSet(session->tree, key, item_type, value, value_size);
}

/*
 * Public jump list storage subroutines private to PuTTie/winfrip_storage*.c
 */

void
WfsJumpListAdd(
	WfsBackend			backend,
	const char *const	sessionname
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status;


	/* Do nothing on pre-Win7 systems. */
	if ((osMajorVersion < 6)
	||  ((osMajorVersion == 6) && (osMinorVersion < 1)))
	{
		return;
	}

	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		backend_impl->JumpListAdd(sessionname);
	}
}

void
WfsJumpListClear(
	WfsBackend	backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		backend_impl->JumpListClear();
	}
}

WfrStatus
WfsJumpListExport(
	WfsBackend	backend_from,
	WfsBackend	backend_to,
	bool		movefl
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	char *			jump_list = NULL;
	size_t			jump_list_size;
	WfrStatus		status;


	if (WFR_STATUS_FAILURE(status = WFSP_BACKEND_GET_IMPL(backend_from, &backend_from_impl))
	||  WFR_STATUS_FAILURE(status = WFSP_BACKEND_GET_IMPL(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_STATUS_SUCCESS(status = backend_from_impl->JumpListGetEntries(&jump_list, &jump_list_size))
	&&  WFR_STATUS_SUCCESS(status = backend_to_impl->JumpListSetEntries(jump_list, jump_list_size)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_STATUS_SUCCESS(status) && movefl) {
		status = backend_from_impl->JumpListCleanup();
	}

	WFR_SFREE_IF_NOTNULL(jump_list);

	return status;
}

char *
WfsJumpListGetEntries(
	WfsBackend	backend
	)
{
	WfspBackend *	backend_impl;
	char *			jump_list = NULL;
	size_t			jump_list_size;


	if (WFR_STATUS_SUCCESS(WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		(void)backend_impl->JumpListGetEntries(&jump_list, &jump_list_size);
	}

	if (!jump_list && (jump_list = snewn(2, char))) {
		jump_list[0] = '\0'; jump_list[1] = '\0';
	}

	return jump_list;
}

void
WfsJumpListRemove(
	WfsBackend			backend,
	const char *const	sessionname
	)
{
	WfspBackend *	backend_impl;
	WfrStatus		status;


	/* Do nothing on pre-Win7 systems. */
	if ((osMajorVersion < 6)
	||  ((osMajorVersion == 6) && (osMinorVersion < 1)))
	{
		return;
	}

	if (WFR_STATUS_SUCCESS(status = WFSP_BACKEND_GET_IMPL(backend, &backend_impl))) {
		backend_impl->JumpListRemove(sessionname);
	}
}

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c
 */

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
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
