/*
 * winfrip_storage_sessions.c - pointless frippery & tremendous amounts of bloat
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
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_backend_ephemeral.h"
#include "PuTTie/winfrip_storage_backend_file.h"
#include "PuTTie/winfrip_storage_backend_registry.h"

/*
 * Public session storage subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfsAddSession(
	WfsBackend	backend,
	const char *	sessionname,
	WfspSession **	psession
	)
{
	WfspBackend *	backend_impl;
	WfspSession *	session_new = NULL;
	const char *	sessionname_new;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
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
WfsCleanupSessions(
	WfsBackend	backend
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (WFR_STATUS_SUCCESS(status = backend_impl->ClearSessions(backend))) {
			status = backend_impl->CleanupSessions(backend);
		}
	}

	return status;
}

WfrStatus
WfsClearSession(
	WfsBackend	backend,
	WfspSession *	session,
	const char *	sessionname
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
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
	WfrStatus	status = WFR_STATUS_CONDITION_SUCCESS;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
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
	WfsBackend	backend,
	WfspSession *	session
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->CloseSession(backend, session);
	}

	return status;
}

WfrStatus
WfsCopySession(
	WfsBackend	backend_from,
	WfsBackend	backend_to,
	const char *	sessionname,
	WfspSession *	session,
	WfspSession **	psession
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	WfspSession *	session_to;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	&&  WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend_to, &backend_to_impl)))
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
	WfsBackend	backend,
	bool		delete_in_backend,
	const char *	sessionname
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
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
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
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
	WfsBackend	backend_from,
	WfsBackend	backend_to,
	bool		movefl,
	char *		sessionname
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	WfspSession *	session;
	WfrStatus	status;


	if (WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	||  WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_to, &backend_to_impl))) {
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
	bool		donefl = false;
	void *		enum_state = NULL;
	char *		sessionname;
	WfrStatus	status;


	if (WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	||  WFR_STATUS_FAILURE(status = WfsGetBackendImpl(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_STATUS_SUCCESS(status) && clear_to) {
		if (WFR_STATUS_FAILURE(status = WfsClearSessions(backend_to, true))) {
			return status;
		}
	}

	status = WfsEnumerateSessions(
		backend_from, false, true,
		&donefl, &sessionname, &enum_state);

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
		} while (!donefl && (WFR_STATUS_SUCCESS(status) || continue_on_error));
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
	WfsBackend	backend,
	bool		cached,
	const char *	sessionname,
	WfspSession **	psession
	)
{
	WfspBackend *	backend_impl;
	WfspTreeItem *	item;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
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
	void **			pvalue,
	size_t *		pvalue_size
	)
{
	WfspTreeItem *	item;
	WfrStatus	status;


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
	WfsBackend	backend,
	bool		rename_in_backend,
	const char *	sessionname,
	const char *	sessionname_new
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
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
	WfsBackend	backend,
	WfspSession *	session
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->SaveSession(backend, session);
	}

	return status;
}

WfrStatus
WfsSetSessionKey(
	WfspSession *		session,
	const char *		key,
	void *			value,
	size_t			value_size,
	WfspTreeItemType	item_type
	)
{
	return WfspTreeSet(session->tree, key, item_type, value, value_size);
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
