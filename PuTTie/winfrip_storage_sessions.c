/*
 * winfrip_storage_sessions.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"
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
	WfsSession **	psession
	)
{
	WfspBackend *	backend_impl;
	WfsSession *	session_new = NULL;
	const char *	sessionname_new = NULL;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))
	&&  WFR_NEW(status, session_new, WfsSession)
	&&  WFR_SUCCESS_POSIX(status, (sessionname_new = strdup(sessionname))))
	{
		WFS_SESSION_INIT(*session_new);
		session_new->name = sessionname_new;

		if (WFR_SUCCESS(status = WfrTreeInit(&session_new->tree))
		&&  WFR_SUCCESS(status = WfrTreeSet(
				backend_impl->tree_session, sessionname,
				WFR_TREE_ITYPE_SESSION, session_new,
				sizeof(*session_new), WfsTreeFreeItem)))
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		if (WFR_SUCCESS(status)) {
			*psession = session_new;
		}
	}

	if (WFR_FAILURE(status)) {
		WFR_FREE_IF_NOTNULL(session_new);
		WFR_FREE_IF_NOTNULL(sessionname_new);
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


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (WFR_SUCCESS(status = WfsClearSessions(backend, false))) {
			status = backend_impl->CleanupSessions(backend);
		}
	}

	return status;
}

WfrStatus
WfsClearSession(
	WfsBackend	backend,
	WfsSession *	session,
	const char *	sessionname
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (WFR_SUCCESS(status = WfsGetSession(backend, true, sessionname, &session))) {
			status = WfrTreeClear(&session->tree, WfsTreeFreeItem);
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


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (delete_in_backend) {
			status = backend_impl->ClearSessions(backend);
		}

		if (WFR_SUCCESS(status)) {
			status = WfrTreeClear(&backend_impl->tree_session, WfsTreeFreeItem);
		}
	}

	return status;
}

WfrStatus
WfsCloseSession(
	WfsBackend	backend,
	WfsSession *	session
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->CloseSession(backend, session);
	}

	return status;
}

WfrStatus
WfsCopySession(
	WfsBackend	backend_from,
	WfsBackend	backend_to,
	const char *	sessionname,
	WfsSession *	session,
	WfsSession **	psession
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	WfsSession *	session_to;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	&&  WFR_SUCCESS(status = WfsGetBackendImpl(backend_to, &backend_to_impl)))
	{
		if (!session) {
			status = WfsGetSession(backend_from, true, sessionname, &session);
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		if (WFR_SUCCESS(status)
		&&  WFR_SUCCESS(status = WfsAddSession(backend_to, sessionname, &session_to))
		&&  WFR_SUCCESS(status = WfrTreeCopy(session->tree, session_to->tree, WfsTreeCloneValue, WfsTreeFreeItem)))
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


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		if (delete_in_backend) {
			status = backend_impl->DeleteSession(backend, sessionname);
		}

		if (WFR_SUCCESS(status)) {
			status = WfrTreeDelete(
				backend_impl->tree_session, NULL,
				sessionname, WFR_TREE_ITYPE_SESSION,
				WfsTreeFreeItem);

			if (WFR_STATUS_IS_NOT_FOUND(status)) {
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
	void **		pstate
	)
{
	WfspBackend *	backend_impl;
	WfrTreeItem *	item;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		switch (cached) {
		case true:
			status = WfrTreeEnumerate(
				backend_impl->tree_session, initfl,
				pdonefl, &item, pstate);
			if (!initfl && WFR_SUCCESS(status) && !(*pdonefl)) {
				WFR_STATUS_BIND_POSIX(status,
					(*psessionname = strdup(item->key)));
			}
			break;

		case false:
			status = backend_impl->EnumerateSessions(
				backend, initfl, pdonefl,
				psessionname, pstate);
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
	WfsSession *	session;
	WfrStatus	status;


	if (WFR_FAILURE(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	||  WFR_FAILURE(status = WfsGetBackendImpl(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_SUCCESS(status = backend_from_impl->LoadSession(backend_to, sessionname, &session))
	&&  WFR_SUCCESS(status = backend_to_impl->SaveSession(backend_to, session)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_SUCCESS(status) && movefl) {
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
	WfsErrorFn	error_fn
	)
{
	WfspBackend *	backend_from_impl, *backend_to_impl;
	bool		donefl = false;
	void *		enum_state = NULL;
	char *		sessionname;
	WfrStatus	status;


	if (WFR_FAILURE(status = WfsGetBackendImpl(backend_from, &backend_from_impl))
	||  WFR_FAILURE(status = WfsGetBackendImpl(backend_to, &backend_to_impl))) {
		return status;
	}

	if (WFR_SUCCESS(status) && clear_to) {
		if (WFR_FAILURE(status = WfsClearSessions(backend_to, true))) {
			return status;
		}
	}

	status = WfsEnumerateSessions(
		backend_from, false, true,
		&donefl, &sessionname, &enum_state);

	if (WFR_SUCCESS(status)) {
		do {
			sessionname = NULL;
			status = WfsEnumerateSessions(
				backend_from, false, false,
				&donefl, &sessionname, &enum_state);

			if (WFR_SUCCESS(status) && sessionname) {
				status = WfsExportSession(
					backend_from, backend_to,
					false, sessionname);
			}

			if (WFR_FAILURE(status)) {
				error_fn(sessionname, status);
			}
		} while (!donefl && (WFR_SUCCESS(status) || continue_on_error));
	}

	if (WFR_FAILURE(status) && continue_on_error) {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_FAILURE(status)) {
		(void)WfsClearSessions(backend_to, true);
	}

	return status;
}

WfrStatus
WfsGetSession(
	WfsBackend	backend,
	bool		cached,
	const char *	sessionname,
	WfsSession **	psession
	)
{
	WfspBackend *	backend_impl;
	WfrTreeItem *	item;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		switch (cached) {
		case true:
			status = WfrTreeGet(
				backend_impl->tree_session, sessionname,
				WFR_TREE_ITYPE_SESSION, &item);
			if (WFR_SUCCESS(status)) {
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
	WfsSession *		session,
	const char *		key,
	WfrTreeItemType		item_type,
	void **			pvalue,
	size_t *		pvalue_size
	)
{
	WfrTreeItem *	item;
	WfrStatus	status;


	status = WfrTreeGet(session->tree, key, item_type, &item);
	if (WFR_SUCCESS(status)) {
		switch (item->type) {
		default:
			status = WFR_STATUS_FROM_ERRNO1(EINVAL); break;

		case WFR_TREE_ITYPE_HOST_KEY:
		case WFR_TREE_ITYPE_SESSION:
		case WFR_TREE_ITYPE_STRING:
			*pvalue = item->value;
			break;

		case WFR_TREE_ITYPE_INT:
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


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = WfrTreeRename(
			backend_impl->tree_session, NULL,
			sessionname, WFR_TREE_ITYPE_SESSION,
			sessionname_new, WfsTreeFreeItem);

		if (rename_in_backend
		&&  (WFR_SUCCESS(status) || WFR_STATUS_IS_NOT_FOUND(status)))
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
	WfsSession *	session
	)
{
	WfspBackend *	backend_impl;
	WfrStatus	status;


	if (WFR_SUCCESS(status = WfsGetBackendImpl(backend, &backend_impl))) {
		status = backend_impl->SaveSession(backend, session);
	}

	return status;
}

WfrStatus
WfsSetSessionKey(
	WfsSession *		session,
	const char *		key,
	void *			value,
	size_t			value_size,
	WfrTreeItemType		item_type
	)
{
	return WfrTreeSet(
		session->tree, key, item_type,
		value, value_size, WfsTreeFreeItem);
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
