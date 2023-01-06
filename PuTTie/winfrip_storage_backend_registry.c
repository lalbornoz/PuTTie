/*
 * winfrip_storage_backend_registry.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include "PuTTie/winfrip_rtl_status.h"
#include "PuTTie/winfrip_storage_jumplist_wrap.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_backend_registry.h"

#include <errno.h>

/*
 * Private variables
 */

/*
 * Names of registry subkeys containing host keys and sessions, resp.
 * [see windows/storage.c]
 */

static LPCSTR		WfspRegistrySubKeyHostKeys = PUTTY_REG_POS "\\SshHostKeys";
static LPCSTR		WfspRegistrySubKeySessions = PUTTY_REG_POS "\\Sessions";

/*
 * Private subroutine prototypes
 */

static WfrStatus	WfsppRegistryEnumerateInit(LPCSTR lpSubKey, WfspRegistryEnumerateState **enum_state);
static WfrStatus	WfsppRegistryEnumerateKeys(bool *pdonefl, char **pitem_name, void *state);
static WfrStatus	WfsppRegistryEnumerateValues(bool *pdonefl, void **pitem_data, size_t *pitem_data_len, char **pitem_name, WfspTreeItemType *pitem_type, void *state);
static void			WfsppRegistryEscapeKey(const char *key, char **pkey_escaped);

/*
 * Private subroutines
 */

static WfrStatus
WfsppRegistryEnumerateInit(
	LPCSTR							lpSubKey,
	WfspRegistryEnumerateState **	enum_state
	)
{
	WfrStatus		status;


	if (!(((*enum_state) = snew(WfspRegistryEnumerateState)))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		WFSP_REGISTRY_ENUMERATE_STATE_INIT((**enum_state));
		if (!((*enum_state)->hKey = open_regkey_ro(HKEY_CURRENT_USER, lpSubKey))) {
			sfree((*enum_state)); *enum_state = NULL;
			status = WFR_STATUS_FROM_ERRNO1(ENOENT);
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	return status;
}

static WfrStatus
WfsppRegistryEnumerateKeys(
	bool *		pdonefl,
	char **		pitem_name,
	void *		state
	)
{
	WfspRegistryEnumerateState *	enum_state;
	char *							item_name = NULL;
	strbuf *						item_name_sb;
	WfrStatus						status;


	enum_state = (WfspRegistryEnumerateState *)state;
	if (enum_state->donefl
	||  !(item_name = enum_regkey(enum_state->hKey, enum_state->dwIndex)))
	{
		*pdonefl = enum_state->donefl = true;
		*pitem_name = NULL;
		status = WFR_STATUS_CONDITION_SUCCESS;
	} else if (!(item_name_sb = strbuf_new())) {
		status = WFR_STATUS_FROM_WINDOWS();
	} else {
		unescape_registry_key(item_name, item_name_sb);
		sfree(item_name);

		*pdonefl = false;
		*pitem_name = strbuf_to_str(item_name_sb);
		enum_state->dwIndex++;
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

static WfrStatus
WfsppRegistryEnumerateValues(
	bool *				pdonefl,
	void **				pitem_data,
	size_t *			pitem_data_len,
	char **				pitem_name,
	WfspTreeItemType *	pitem_type,
	void *				state
	)
{
	bool							donefl;
	DWORD							dwType;
	WfspRegistryEnumerateState *	enum_state;
	void *							item_data = NULL;
	DWORD							item_data_len, item_data_size = 0;
	static char						item_name[32767]; // [see https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regenumvaluea]
	DWORD							item_name_len;
	strbuf *						item_name_sb;
	WfspTreeItemType				item_type;
	WfrStatus						status;
	LSTATUS							status_registry;


	donefl = false;
	enum_state = (WfspRegistryEnumerateState *)state;
	status = WFR_STATUS_CONDITION_SUCCESS;

	if (!(item_data = snewn(1, char))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		item_data_size = 1;

		do {
			item_data_len = item_data_size; item_name_len = sizeof(item_name);
			switch ((status_registry = RegEnumValue(
						enum_state->hKey, enum_state->dwIndex, item_name,
						&item_name_len, NULL, &dwType, item_data,
						&item_data_len)))
			{
			default:
				donefl = true; *pdonefl = false;
				status = WFR_STATUS_FROM_WINDOWS1(status_registry);
				break;

			case ERROR_MORE_DATA:
				item_data_len++;
				status = WFR_SRESIZE_IF_NEQ_SIZE(
							item_data, item_data_size,
							item_data_len, char);
				break;

			case ERROR_NO_MORE_ITEMS:
				donefl = true; *pdonefl = true;
				status = WFR_STATUS_CONDITION_SUCCESS;

				*pitem_name = NULL;
				close_regkey(enum_state->hKey);
				WFSP_REGISTRY_ENUMERATE_STATE_INIT(*enum_state);
				break;

			case ERROR_SUCCESS:
				donefl = true;

				switch (dwType) {
				default:
					donefl = true; *pdonefl = false;
					status = WFR_STATUS_FROM_ERRNO1(EINVAL);
					goto out;

				case REG_DWORD:
					item_type = WFSP_TREE_ITYPE_INT; item_data_len = sizeof(int); break;
				case REG_SZ:
					item_type = WFSP_TREE_ITYPE_STRING; item_data_len++; break;
				}
				item_name[item_name_len] = '\0';

				if (!(item_name_sb = strbuf_new())) {
					*pdonefl = false;
					status = WFR_STATUS_FROM_WINDOWS();
				} else {
					*pdonefl = false;
					status = WFR_STATUS_CONDITION_SUCCESS;

					unescape_registry_key(item_name, item_name_sb);
					if (pitem_data) {
						*pitem_data = item_data;
					}
					if (pitem_data_len) {
						*pitem_data_len = item_data_len;
					}
					*pitem_name = strbuf_to_str(item_name_sb);
					if (pitem_type) {
						*pitem_type = item_type;
					}
					enum_state->dwIndex++;
				}

			out:
				break;
			}
		} while (!(*pdonefl) && !donefl);
	}

	if (WFR_STATUS_FAILURE(status)) {
		WFR_SFREE_IF_NOTNULL(item_data);
	}

	return status;
}

static void
WfsppRegistryEscapeKey(
	const char *	key,
	char **			pkey_escaped
	)
{
	strbuf *	key_sb;


	key_sb = strbuf_new();
	escape_registry_key(key, key_sb);
	*pkey_escaped = strbuf_to_str(key_sb);
}

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c
 */

WfrStatus
WfspRegistryClearHostKeys(
	WfsBackend	backend
	)
{
	bool							donefl;
	WfspRegistryEnumerateState *	enum_state = NULL;
	HKEY							hKey;
	size_t							itemc = 0;
	char **							itemv = NULL;
	const char *					key_name;
	char *							key_name_escaped;
	WfrStatus						status;
	LSTATUS							status_registry;


	status = WfspRegistryEnumerateHostKeys(backend, true, &donefl, NULL, &enum_state);

	if (WFR_STATUS_SUCCESS(status)) {
		do {
			if (WFR_STATUS_SUCCESS(status = WfspRegistryEnumerateHostKeys(
						backend, false, &donefl,
						&key_name, enum_state)) && !donefl)
			{
				if (WFR_STATUS_FAILURE(status = WFR_SRESIZE_IF_NEQ_SIZE(
								itemv, itemc, itemc + 1, char *)))
				{
					sfree((void *)key_name);
				} else {
					WfsppRegistryEscapeKey((char *)key_name, &key_name_escaped);
					sfree((void *)key_name);
					itemv[itemc - 1] = key_name_escaped;
				}
			}
		} while (WFR_STATUS_SUCCESS(status) && !donefl);

		if (WFR_STATUS_SUCCESS(status)) {
			if (!(hKey = open_regkey_rw(HKEY_CURRENT_USER, WfspRegistrySubKeyHostKeys))) {
				status = WFR_STATUS_FROM_WINDOWS();
			} else {
				for (size_t nitem = 0; nitem < itemc; nitem++) {
					status_registry = RegDeleteValue(hKey, itemv[nitem]);
					if (status_registry != ERROR_SUCCESS) {
						status = WFR_STATUS_FROM_WINDOWS1(status_registry);
						break;
					}
				}
	    		close_regkey(hKey);
			}
		}

		for (size_t nitem = 0; nitem < itemc; nitem++) {
			WFR_SFREE_IF_NOTNULL(itemv[nitem]);
		}
		WFR_SFREE_IF_NOTNULL(itemv);
	}

	WFR_SFREE_IF_NOTNULL(enum_state);

	return status;
}

WfrStatus
WfspRegistryDeleteHostKey(
	WfsBackend		backend,
	const char *	key_name
	)
{
	HKEY		hKey;
	LPSTR		key_name_escaped = NULL;
	WfrStatus	status;
	LSTATUS		status_registry;


	(void)backend;

	WfsppRegistryEscapeKey(key_name, &key_name_escaped);

	if (!(hKey = open_regkey_rw(HKEY_CURRENT_USER, WfspRegistrySubKeyHostKeys))) {
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);
	} else {
		status_registry = RegDeleteValue(hKey, key_name_escaped);
		if (status_registry != ERROR_SUCCESS) {
			status = WFR_STATUS_FROM_WINDOWS1(status_registry);
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
		close_regkey(hKey);
	}

	WFR_SFREE_IF_NOTNULL(key_name_escaped);

	return status;
}

WfrStatus
WfspRegistryEnumerateHostKeys(
	WfsBackend		backend,
	bool			initfl,
	bool *			pdonefl,
	const char **	pkey_name,
	void *			state
	)
{
	WfspRegistryEnumerateState *	enum_state;
	char *							item_name;
	WfrStatus						status;


	(void)backend;

	if (initfl) {
		return WfsppRegistryEnumerateInit(
				WfspRegistrySubKeyHostKeys,
				(WfspRegistryEnumerateState **)state);
	}

	enum_state = state;
	if (WFR_STATUS_SUCCESS(status = WfsppRegistryEnumerateValues(
						pdonefl, NULL, NULL,
						&item_name, NULL, state)) && !(*pdonefl))
	{
		*pkey_name = item_name;
	} else {
		*pkey_name = NULL;
	    close_regkey(enum_state->hKey);
		WFSP_REGISTRY_ENUMERATE_STATE_INIT(*enum_state);
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfspRegistryLoadHostKey(
	WfsBackend		backend,
	const char *	key_name,
	const char **	pkey
	)
{
	HKEY			hKey = NULL;
	char *			key_name_escaped = NULL;
	char *			key = NULL;
	DWORD			key_size;
	WfrStatus		status;
	LSTATUS			status_registry;


	WfsppRegistryEscapeKey(key_name, &key_name_escaped);
	if (!(hKey = open_regkey_ro(HKEY_CURRENT_USER, WfspRegistrySubKeyHostKeys))) {
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);
	} else {
		key_size = 0;
		if (!(status_registry = RegGetValue(
					hKey, NULL, key_name_escaped,
					RRF_RT_REG_SZ, NULL, NULL,
					&key_size)) == ERROR_SUCCESS)
		{
			status = WFR_STATUS_FROM_WINDOWS1(status_registry);
		} else if (!(key = snewn(key_size, char))) {
			status = WFR_STATUS_FROM_ERRNO();
		} else if (!(status_registry = RegGetValue(
					hKey, NULL, key_name_escaped,
					RRF_RT_REG_SZ, NULL, key,
					&key_size)) == ERROR_SUCCESS)
		{
			status = WFR_STATUS_FROM_WINDOWS1(status_registry);
		} else {
			status = WfsSetHostKey(backend, key_name, key);
		}
	}

	if (hKey != NULL) {
	    close_regkey(hKey);
	}
	WFR_SFREE_IF_NOTNULL(key_name_escaped);

	if (WFR_STATUS_SUCCESS(status)) {
		*pkey = key;
	} else {
		WFR_SFREE_IF_NOTNULL(key);
	}

	return status;
}

WfrStatus
WfspRegistryRenameHostKey(
	WfsBackend		backend,
	const char *	key_name,
	const char *	key_name_new
	)
{
	HKEY			hKey = NULL;
	const char *	key;
	char *			key_name_escaped = NULL, *key_name_new_escaped = NULL;
	WfrStatus		status;
	LSTATUS			status_registry;


	(void)backend;

	if (WFR_STATUS_SUCCESS(status = WfspRegistryLoadHostKey(backend, key_name, &key)))
	{
		WfsppRegistryEscapeKey(key_name, &key_name_escaped);
		WfsppRegistryEscapeKey(key_name_new, &key_name_new_escaped);
		if (!(hKey = open_regkey_rw(HKEY_CURRENT_USER, WfspRegistrySubKeyHostKeys))) {
			status = WFR_STATUS_FROM_ERRNO1(ENOENT);
		} else {
			status_registry = RegSetValueEx(
					hKey, key_name_new_escaped, 0, REG_SZ,
					(const BYTE *)key, strlen(key));
			if (status_registry != ERROR_SUCCESS) {
				status = WFR_STATUS_FROM_WINDOWS1(status_registry);
			} else {
				status_registry = RegDeleteValue(hKey, key_name_escaped);
				if (status_registry != ERROR_SUCCESS) {
					status = WFR_STATUS_FROM_WINDOWS1(status_registry);
				} else {
					status = WFR_STATUS_CONDITION_SUCCESS;
				}
			}
		}
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (hKey != NULL) {
		close_regkey(hKey);
	}
	WFR_SFREE_IF_NOTNULL(key_name_escaped);
	WFR_SFREE_IF_NOTNULL(key_name_new_escaped);

	return status;
}

WfrStatus
WfspRegistrySaveHostKey(
	WfsBackend		backend,
	const char *	key_name,
	const char *	key
	)
{
	HKEY		hKey;
	char *		key_name_escaped = NULL;
	WfrStatus	status;
	LSTATUS		status_registry;


	(void)backend;

	WfsppRegistryEscapeKey(key_name, &key_name_escaped);

	if (!(hKey = open_regkey_rw(HKEY_CURRENT_USER, WfspRegistrySubKeyHostKeys))) {
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);
	} else {
		status_registry = RegSetValueEx(
				hKey, key_name_escaped, 0, REG_SZ,
				(const BYTE *)key, strlen(key));
		if (status_registry == ERROR_SUCCESS) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		} else {
			status = WFR_STATUS_FROM_WINDOWS1(status_registry);
		}
	}

	if (hKey != NULL) {
    	close_regkey(hKey);
	}
	WFR_SFREE_IF_NOTNULL(key_name_escaped);

	return status;
}


WfrStatus
WfspRegistryClearSessions(
	WfsBackend	backend
	)
{
	bool						donefl;
	WfspRegistryEnumerateState	enum_state;
	char *						item_name, *item_name_escaped;
	size_t						itemc = 0;
	char **						itemv = NULL;
	WfrStatus					status;
	LSTATUS						status_registry;


	(void)backend;
	WFSP_REGISTRY_ENUMERATE_STATE_INIT(enum_state);
	if (!(enum_state.hKey = open_regkey_ro(HKEY_CURRENT_USER, WfspRegistrySubKeySessions))) {
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
		do {
			if (WFR_STATUS_SUCCESS(status = WfsppRegistryEnumerateKeys(
						&donefl, &item_name, &enum_state)) && !donefl)
			{
				WfsppRegistryEscapeKey(item_name, &item_name_escaped);
				sfree(item_name);
				if (WFR_STATUS_FAILURE(status = WFR_SRESIZE_IF_NEQ_SIZE(
								itemv, itemc, itemc + 1, char *)))
				{
					sfree(item_name_escaped);
				} else {
					itemv[itemc - 1] = item_name_escaped;
				}
			}
		} while (WFR_STATUS_SUCCESS(status) && !donefl);

		if (WFR_STATUS_SUCCESS(status)) {
			for (size_t nitem = 0; nitem < itemc; nitem++) {
				status_registry = RegDeleteKey(enum_state.hKey, itemv[nitem]);
				if (status_registry != ERROR_SUCCESS) {
					status = WFR_STATUS_FROM_WINDOWS1(status_registry);
					break;
				}
			}
		}

		for (size_t nitem = 0; nitem < itemc; nitem++) {
			WFR_SFREE_IF_NOTNULL(itemv[nitem]);
		}
		WFR_SFREE_IF_NOTNULL(itemv);

    	close_regkey(enum_state.hKey);
	}

	return status;
}

WfrStatus
WfspRegistryCloseSession(
	WfsBackend		backend,
	WfspSession *	session
	)
{
	return WfsDeleteSession(backend, false, session->name);
}

WfrStatus
WfspRegistryDeleteSession(
	WfsBackend		backend,
	const char *	sessionname
	)
{
	HKEY		hKey;
	LPSTR		sessionname_escaped = NULL;
	WfrStatus	status;
	LSTATUS		status_registry;


	(void)backend;

	WfsppRegistryEscapeKey(sessionname, &sessionname_escaped);

	if (!(hKey = open_regkey_rw(HKEY_CURRENT_USER, WfspRegistrySubKeySessions))) {
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);
	} else {
		status_registry = RegDeleteKey(hKey, sessionname_escaped);
		if (status_registry != ERROR_SUCCESS) {
			status = WFR_STATUS_FROM_WINDOWS1(status_registry);
		} else {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
    	close_regkey(hKey);
	}

	WFR_SFREE_IF_NOTNULL(sessionname_escaped);

	return status;
}

WfrStatus
WfspRegistryEnumerateSessions(
	WfsBackend	backend,
	bool		initfl,
	bool *		pdonefl,
	char **		psessionname,
	void *		state
	)
{
	WfspRegistryEnumerateState *	enum_state;
	char *							item_name;
	WfrStatus						status;


	(void)backend;

	if (initfl) {
		return WfsppRegistryEnumerateInit(
				WfspRegistrySubKeySessions,
				(WfspRegistryEnumerateState **)state);
	}

	enum_state = state;
	if (WFR_STATUS_SUCCESS(status = WfsppRegistryEnumerateKeys(
					pdonefl, &item_name, state)) && !(*pdonefl)) {
		*psessionname = item_name;
	} else {
		*psessionname = NULL;
	    close_regkey(enum_state->hKey);
		WFSP_REGISTRY_ENUMERATE_STATE_INIT(*enum_state);
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

WfrStatus
WfspRegistryLoadSession(
	WfsBackend		backend,
	const char *	sessionname,
	WfspSession **	psession
	)
{
	bool						addedfl = false, donefl = false;
	WfspRegistryEnumerateState	enum_state;
	WfspTreeItemType			item_type;
	WfspSession *				session;
	char *						sessionname_escaped = NULL;
	void *						setting_data = NULL;
	size_t						setting_data_len;
	char *						setting_name;
	WfrStatus					status;


	WFSP_REGISTRY_ENUMERATE_STATE_INIT(enum_state);
	WfsppRegistryEscapeKey(sessionname, &sessionname_escaped);

    if (!(enum_state.hKey = open_regkey_ro(
					HKEY_CURRENT_USER, WfspRegistrySubKeySessions,
					sessionname_escaped)))
	{
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
		status = WfsGetSession(backend, true, sessionname, &session);
		if (WFR_STATUS_SUCCESS(status)) {
			(void)WfsDeleteSession(backend, false, session->name);
			status = WfsAddSession(backend, sessionname, &session);
			addedfl = false;
		} else if (WFR_STATUS_CONDITION(status) == ENOENT) {
			status = WfsAddSession(backend, sessionname, &session);
			addedfl = WFR_STATUS_SUCCESS(status);
		}

		while (WFR_STATUS_SUCCESS(status) && !donefl) {
			if (WFR_STATUS_SUCCESS(status = WfsppRegistryEnumerateValues(
									&donefl, &setting_data, &setting_data_len,
									&setting_name, &item_type, &enum_state)) && !donefl)
			{
				status = WfsSetSessionKey(
					session, setting_name, setting_data,
					setting_data_len, item_type);
				sfree(setting_name);

				if (WFR_STATUS_FAILURE(status)) {
					sfree(setting_data);
				}
			}
		}
	}

	if (enum_state.hKey != NULL) {
	    close_regkey(enum_state.hKey);
	}
	WFR_SFREE_IF_NOTNULL(sessionname_escaped)

	if (WFR_STATUS_SUCCESS(status)) {
		if (psession) {
			*psession = session;
		}
	} else {
		if (addedfl && session) {
			(void)WfsDeleteSession(backend, false, session->name);
		}
	}

	return status;
}

WfrStatus
WfspRegistryRenameSession(
	WfsBackend		backend,
	const char *	sessionname,
	const char *	sessionname_new
	)
{
	HKEY		hKey = NULL;
	WfrStatus	status;
	LSTATUS		status_registry;
	char *		sessionname_escaped = NULL, *sessionname_new_escaped = NULL;
	wchar_t *	sessionname_escaped_w = NULL, *sessionname_new_escaped_w = NULL;


	(void)backend;

	WfsppRegistryEscapeKey(sessionname, &sessionname_escaped);
	WfsppRegistryEscapeKey(sessionname_new, &sessionname_new_escaped);

	if (strcmp(sessionname, sessionname_new) != 0) {
		if (!(hKey = open_regkey_rw(HKEY_CURRENT_USER, WfspRegistrySubKeySessions))) {
			status = WFR_STATUS_FROM_ERRNO1(ENOENT);
		} else if (WFR_STATUS_SUCCESS(status = WfrToWcsDup(sessionname_escaped, strlen(sessionname_escaped) + 1, &sessionname_escaped_w))
				&& WFR_STATUS_SUCCESS(status = WfrToWcsDup(sessionname_new_escaped, strlen(sessionname_new_escaped) + 1, &sessionname_new_escaped_w)))
		{
			status_registry = RegRenameKey(hKey, sessionname_escaped_w, sessionname_new_escaped_w);
			if (status_registry != ERROR_SUCCESS) {
				status = WFR_STATUS_FROM_WINDOWS1(status_registry);
			} else {
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		}
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (hKey != NULL) {
		close_regkey(hKey);
	}
	WFR_SFREE_IF_NOTNULL(sessionname_escaped);
	WFR_SFREE_IF_NOTNULL(sessionname_new_escaped);
	WFR_SFREE_IF_NOTNULL(sessionname_escaped_w);
	WFR_SFREE_IF_NOTNULL(sessionname_new_escaped_w);

	return status;
}

WfrStatus
WfspRegistrySaveSession(
	WfsBackend		backend,
	WfspSession *	session
	)
{
	HKEY			hKey;
	WfspTreeItem *	item;
	char *			sessionname, *sessionname_escaped = NULL;
	WfrStatus		status;
	LSTATUS			status_registry;


	(void)backend;

	sessionname = (char *)session->name;
	WfsppRegistryEscapeKey(sessionname, &sessionname_escaped);

	(void)RegDeleteKey(hKey, sessionname_escaped);
    if (!(hKey = create_regkey(
					HKEY_CURRENT_USER, WfspRegistrySubKeySessions,
					sessionname_escaped)))
	{
		status = WFR_STATUS_FROM_ERRNO1(ENOENT);
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
		WFSP_TREE234_FOREACH(status, session->tree, idx, item) {
			switch (item->type) {
			default:
				status = WFR_STATUS_FROM_ERRNO1(EINVAL);
				break;

			case WFSP_TREE_ITYPE_INT:
				status_registry = RegSetValueEx(
						hKey, item->key, 0, REG_DWORD,
						(const BYTE *)item->value, item->value_size);
				if (status_registry != ERROR_SUCCESS) {
					status = WFR_STATUS_FROM_WINDOWS1(status_registry);
				}
				break;

			case WFSP_TREE_ITYPE_STRING:
				status_registry = RegSetValueEx(
						hKey, item->key, 0, REG_SZ,
						(const BYTE *)item->value, item->value_size);
				if (status_registry != ERROR_SUCCESS) {
					status = WFR_STATUS_FROM_WINDOWS1(status_registry);
				}
				break;
			}
		}
	}

	if (WFR_STATUS_FAILURE(status)) {
		(void)RegDeleteKey(hKey, sessionname_escaped);
	}

    close_regkey(hKey);
	WFR_SFREE_IF_NOTNULL(sessionname_escaped);

	return status;
}


void
WfspRegistryJumpListAdd(
	const char *const	sessionname
	)
{
	add_session_to_jumplist_PuTTY(sessionname);
}

void
WfspRegistryJumpListClear(
	void
	)
{
	clear_jumplist_PuTTY();
}

void
WfspRegistryJumpListRemove(
	const char *const	sessionname
	)
{
	remove_session_from_jumplist_PuTTY(sessionname);
}


WfrStatus
WfspRegistryInit(
	void
	)
{
	return WFR_STATUS_CONDITION_SUCCESS;
}

WfrStatus
WfspRegistrySetBackend(
	WfsBackend	backend_new
	)
{
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsClearHostKeys(backend_new, false))
	&&  WFR_STATUS_SUCCESS(status = WfsClearSessions(backend_new, false)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */