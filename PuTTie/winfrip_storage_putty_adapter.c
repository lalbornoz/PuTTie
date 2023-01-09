/*
 * winfrip_storage_putty_adapter.c - pointless frippery & tremendous amounts of bloat
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

/*
 * Private variables
 */

/*
 * Vector of key names of missing {int,string} settings to ignore
 * if absent in a debugging build
 */

#ifdef WINFRIP_DEBUG
static const char *		WfspIgnoreMissingIntSettings[] = {
	"BuggyMAC",
	"BugDHGEx2",
	"BugFilterKexinit",
	"NoRemoteQTitle",
};

static const char *		WfspIgnoreMissingStrSettings[] = {
	"AuthPlugin",
	"Wordness0",
	"Wordness32",
	"Wordness64",
	"Wordness96",
	"Wordness128",
	"Wordness160",
	"Wordness192",
	"Wordness224",
};
#endif /* WINFRIP_DEBUG */

/*
 * Private subroutine prototypes
 */

static WfrStatus		WfspExpandFontSpecKeys(const char *key, char **pkey_charset, char **pkey_height, char **pkey_isbold);

/*
 * Private subroutines
 */

static WfrStatus
WfspExpandFontSpecKeys(
	const char *	key,
	char **			pkey_charset,
	char **			pkey_height,
	char **			pkey_isbold
	)
{
	size_t		key_size;
	WfrStatus	status;

	key_size = strlen(key) + 1;
	*pkey_charset = *pkey_height = *pkey_isbold = NULL;
	if (((*pkey_charset) = snewn(key_size + (sizeof("CharSet") - 1), char))
	&&  ((*pkey_height) = snewn(key_size + (sizeof("Height") - 1), char))
	&&  ((*pkey_isbold) = snewn(key_size + (sizeof("IsBold") - 1), char)))
	{
		WFR_SNPRINTF(*pkey_charset, key_size + (sizeof("CharSet") - 1), "%sCharSet", key);
		WFR_SNPRINTF(*pkey_height, key_size + (sizeof("Height") - 1), "%sHeight", key);
		WFR_SNPRINTF(*pkey_isbold, key_size + (sizeof("IsBold") - 1), "%sIsBold", key);
		status = WFR_STATUS_CONDITION_SUCCESS;
	} else {
		WFR_SFREE_IF_NOTNULL(*pkey_charset);
		WFR_SFREE_IF_NOTNULL(*pkey_height);
		WFR_SFREE_IF_NOTNULL(*pkey_isbold);
		status = WFR_STATUS_FROM_ERRNO();
	}

	return status;
}

/*
 * Public type definitions
 */

typedef struct settings_w {
} settings_w;
typedef struct settings_r {
} settings_r;

/*
 * Public wrapped PuTTY storage.h type definitions and subroutines
 */

/* ----------------------------------------------------------------------
 * Functions to save and restore PuTTY sessions. Note that this is
 * only the low-level code to do the reading and writing. The
 * higher-level code that translates an internal Conf structure into
 * a set of (key,value) pairs in their external storage format is
 * elsewhere, since it doesn't (mostly) change between platforms.
 */

/*
 * Write a saved session. The caller is expected to call
 * open_setting_w() to get a `void *' handle, then pass that to a
 * number of calls to write_setting_s() and write_setting_i(), and
 * then close it using close_settings_w(). At the end of this call
 * sequence the settings should have been written to the PuTTY
 * persistent storage area.
 *
 * A given key will be written at most once while saving a session.
 * Keys may be up to 255 characters long.  String values have no length
 * limit.
 *
 * Any returned error message must be freed after use.
 */

settings_w *
open_settings_w(
	const char *	sessionname,
	char **			errmsg
	)
{
	WfsBackend		backend;
	WfspSession *	session;
	WfrStatus		status;


	(void)errmsg;

	WFSP_SESSION_NAME_DEFAULT(sessionname);

	backend = WfsGetBackend();
	status = WfsAddSession(backend, sessionname, &session);
	if (WFR_STATUS_CONDITION(status) == EEXIST) {
		status = WfsClearSession(backend, session, NULL);
	}
	if (WFR_STATUS_SUCCESS(status)) {
		return (settings_w *)session;
	} else {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "opening session");
		return NULL;
	}
}

void
write_setting_s(
	settings_w *	handle,
	const char *	key,
	const char *	value
	)
{
	WfspSession *	session;
	WfrStatus		status;
	char *			value_new;
	size_t			value_new_size;


	session = (WfspSession *)handle;

	if (!(value_new = strdup(value))) {
		WFR_DEBUG_FAIL();
	} else {
		value_new_size = strlen(value_new) + 1;
		if (WFR_STATUS_FAILURE(status = WfsSetSessionKey(
				session, key, value_new, value_new_size,
				WFSP_TREE_ITYPE_STRING)))
		{
			sfree(value_new);
			WFR_DEBUG_FAIL();
		}
	}
}

void
write_setting_i(
	settings_w *	handle,
	const char *	key,
	int				value
	)
{
	WfspSession *	session;
	WfrStatus		status;
	int *			value_new;


	session = (WfspSession *)handle;

	if (!(value_new = snew(int))) {
		WFR_DEBUG_FAIL();
	} else {
		*value_new = value;
		if (WFR_STATUS_FAILURE(status = WfsSetSessionKey(
				session, key, value_new, sizeof(*value_new),
				WFSP_TREE_ITYPE_INT)))
		{
			sfree(value_new);
			WFR_DEBUG_FAIL();
		}
	}
}

void
write_setting_filename(
	settings_w *	handle,
	const char *	key,
	Filename *		value
	)
{
	WfspSession *	session;
	WfrStatus		status;
	char *			value_new;
	size_t			value_new_size;


	session = (WfspSession *)handle;

	if (!(value_new = strdup(value->path))) {
		WFR_DEBUG_FAIL();
	} else {
		value_new_size = strlen(value_new) + 1;
		if (WFR_STATUS_FAILURE(status = WfsSetSessionKey(
				session, key, value_new, value_new_size,
				WFSP_TREE_ITYPE_STRING)))
		{
			sfree(value_new);
			WFR_DEBUG_FAIL();
		}
	}
}

void
write_setting_fontspec(
	settings_w *	handle,
	const char *	key,
	FontSpec *		font
	)
{
	char *			key_charset = NULL, *key_height = NULL, *key_isbold = NULL;
	WfspSession *	session;
	WfrStatus		status;
	int *			val_charset = NULL, *val_height = NULL, *val_isbold = NULL;
	char *			val_name = NULL;
	size_t			val_name_size;


	session = (WfspSession *)handle;

	if (WFR_STATUS_SUCCESS(status = WfspExpandFontSpecKeys(
			key, &key_charset, &key_height, &key_isbold)))
	{
		val_name_size = strlen(font->name) + 1;

		if (!(val_charset = snew(int))
		||  !(val_height = snew(int))
		||  !(val_isbold = snew(int))
		||  !(val_name = snewn(val_name_size, char)))
		{
			WFR_DEBUG_FAIL();
		} else {
			*val_charset = font->charset;
			*val_height = font->height;
			*val_isbold = font->isbold;
			strcpy(val_name, font->name);

			if (WFR_STATUS_SUCCESS(status = WfsSetSessionKey(
					session, key_charset, val_charset,
					sizeof(*val_charset), WFSP_TREE_ITYPE_INT))
			&&  WFR_STATUS_SUCCESS(status = WfsSetSessionKey(
					session, key_height, val_height,
					sizeof(*val_height), WFSP_TREE_ITYPE_INT))
			&&  WFR_STATUS_SUCCESS(status = WfsSetSessionKey(
					session, key_isbold, val_isbold,
					sizeof(*val_isbold), WFSP_TREE_ITYPE_INT))
			&&  WFR_STATUS_SUCCESS(status = WfsSetSessionKey(
					session, key, val_name, val_name_size,
					WFSP_TREE_ITYPE_STRING)))
			{
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		}

		WFR_SFREE_IF_NOTNULL(key_charset);
		WFR_SFREE_IF_NOTNULL(key_height);
		WFR_SFREE_IF_NOTNULL(key_isbold);

		if (WFR_STATUS_FAILURE(status)) {
			WFR_SFREE_IF_NOTNULL(val_charset);
			WFR_SFREE_IF_NOTNULL(val_height);
			WFR_SFREE_IF_NOTNULL(val_isbold);
			WFR_SFREE_IF_NOTNULL(val_name);
		}
	} else {
		WFR_DEBUG_FAIL();
	}
}

void
close_settings_w(
	settings_w *	handle
	)
{
	WfspSession *	session;
	WfrStatus		status;


	session = (WfspSession *)handle;
	status = WfsSaveSession(WfsGetBackend(), session);
	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "closing session");
}

/*
 * Read a saved session. The caller is expected to call
 * open_setting_r() to get a `void *' handle, then pass that to a
 * number of calls to read_setting_s() and read_setting_i(), and
 * then close it using close_settings_r().
 *
 * read_setting_s() returns a dynamically allocated string which the
 * caller must free. read_setting_filename() and
 * read_setting_fontspec() likewise return dynamically allocated
 * structures.
 *
 * If a particular string setting is not present in the session,
 * read_setting_s() can return NULL, in which case the caller
 * should invent a sensible default. If an integer setting is not
 * present, read_setting_i() returns its provided default.
 */

settings_r *
open_settings_r(
	const char *	sessionname
	)
{
	WfspSession *	session;
	WfrStatus		status;


	if (!sessionname || !sessionname[0]
	||  (strcmp(sessionname, "Default Settings") == 0))
	{
		return NULL;
	}

	status = WfsGetSession(
			WfsGetBackend(), false,
			sessionname, &session);

	if (WFR_STATUS_SUCCESS(status)) {
		return (settings_r *)session;
	} else {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "opening session (read-only)");
		return NULL;
	}
}

char *
read_setting_s(
	settings_r *	handle,
	const char *	key
	)
{
	WfspSession *	session;
	WfrStatus		status;
	void *			value;


	session = (WfspSession *)handle;
	if (!session) {
		return NULL;
	}

	if (WFR_STATUS_SUCCESS(status = WfsGetSessionKey(
			session, key, WFSP_TREE_ITYPE_STRING,
			&value, NULL)))
	{
		return strdup(value);
	} else {
	#ifdef WINFRIP_DEBUG
		for (size_t nkey = 0;
			 nkey < (sizeof(WfspIgnoreMissingStrSettings) / sizeof(WfspIgnoreMissingStrSettings[0]));
			 nkey++)
		{
			if (strcmp(key, WfspIgnoreMissingStrSettings[nkey]) == 0) {
				return NULL;
			}
		}
	#endif /* WINFRIP_DEBUG */
		WFR_DEBUG_FAIL();
		return NULL;
	}
}

int
read_setting_i(
	settings_r *	handle,
	const char *	key,
	int				defvalue
	)
{
	WfspSession *	session;
	WfrStatus		status;
	int				value;


	session = (WfspSession *)handle;
	if (!session) {
		return defvalue;
	}
	if (WFR_STATUS_SUCCESS(status = WfsGetSessionKey(
			session, key, WFSP_TREE_ITYPE_INT,
			(void *)&value, NULL)))
	{
		return value;
	} else {
	#ifdef WINFRIP_DEBUG
		for (size_t nkey = 0;
			 nkey < (sizeof(WfspIgnoreMissingIntSettings) / sizeof(WfspIgnoreMissingIntSettings[0]));
			 nkey++)
		{
			if (strcmp(key, WfspIgnoreMissingIntSettings[nkey]) == 0) {
				return defvalue;
			}
		}
	#endif /* WINFRIP_DEBUG */
		WFR_DEBUG_FAIL();
		return defvalue;
	}
}

Filename *
read_setting_filename(
	settings_r *	handle,
	const char *	key
	)
{
	WfspSession *	session;
	WfrStatus		status;
	Filename *		value = NULL;


	session = (WfspSession *)handle;
	if (!session) {
		return NULL;
	}

	if (!(value = snew(Filename))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		status = WfsGetSessionKey(
					session, key, WFSP_TREE_ITYPE_STRING,
					(void *)&value->path, NULL);
	}

	if (WFR_STATUS_SUCCESS(status)) {
		if (!(value->path = strdup(value->path))) {
			WFR_DEBUG_FAIL();
			sfree(value);
			return NULL;
		} else {
			return value;
		}
	} else {
		WFR_DEBUG_FAIL();
		WFR_SFREE_IF_NOTNULL(value);
		return NULL;
	}
}

FontSpec *
read_setting_fontspec(
	settings_r *	handle,
	const char *	key
	)
{
	char *			key_charset = NULL, *key_height = NULL, *key_isbold = NULL;
	WfspSession *	session;
	WfrStatus		status;
	FontSpec *		value;


	session = (WfspSession *)handle;
	if (!session) {
		return NULL;
	}

	if (!(value = snew(FontSpec))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		value->name = NULL;
		if (WFR_STATUS_SUCCESS(status = WfspExpandFontSpecKeys(
					key, &key_charset, &key_height, &key_isbold))
		&&  WFR_STATUS_SUCCESS(status = WfsGetSessionKey(
					session, key_charset, WFSP_TREE_ITYPE_INT,
					(void **)&value->charset, NULL))
		&&  WFR_STATUS_SUCCESS(status = WfsGetSessionKey(
					session, key_height, WFSP_TREE_ITYPE_INT,
					(void **)&value->height, NULL))
		&&  WFR_STATUS_SUCCESS(status = WfsGetSessionKey(
					session, key_isbold, WFSP_TREE_ITYPE_INT,
					(void **)&value->isbold, NULL))
		&&  WFR_STATUS_SUCCESS(status = WfsGetSessionKey(
					session, key, WFSP_TREE_ITYPE_STRING,
					(void **)&value->name, NULL)))
		{
			if (!(value->name = strdup(value->name))) {
				status = WFR_STATUS_FROM_ERRNO();
			} else {
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		}
	}

	WFR_SFREE_IF_NOTNULL(key_charset);
	WFR_SFREE_IF_NOTNULL(key_height);
	WFR_SFREE_IF_NOTNULL(key_isbold);

	if (WFR_STATUS_SUCCESS(status)) {
		return value;
	} else {
		if (value) {
			WFR_SFREE_IF_NOTNULL(value->name);
		}
		WFR_SFREE_IF_NOTNULL(value);

		WFR_DEBUG_FAIL();
		return NULL;
	}
}

void
close_settings_r(
	settings_r *	handle
	)
{
	WfrStatus	status;


	if (handle) {
		status = WfsCloseSession(WfsGetBackend(), (WfspSession *)handle);
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "closing session (read-only)");
}

/*
 * Delete a whole saved session.
 */

void
del_settings(
	const char *	sessionname
	)
{
	WfrStatus	status;


	status = WfsDeleteSession(WfsGetBackend(), false, sessionname);
	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "deleting session");
}

/*
 * Enumerate all saved sessions.
 */

settings_e *
enum_settings_start(
	void
	)
{
	void *		handle;
	WfrStatus	status;


	if (WFR_STATUS_SUCCESS(status = WfsEnumerateSessions(
					WfsGetBackend(), false, true,
					NULL, NULL, (void **)&handle)))
	{
		return handle;
	} else {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "enumerating sessions");
		return NULL;
	}
}

bool
enum_settings_next(
	settings_e *	handle,
	strbuf *		out
	)
{
	bool			donefl;
	char *			sessionname;
	size_t			sessionname_len;
	WfrStatus		status;


	status = WfsEnumerateSessions(
			WfsGetBackend(), false, false,
			&donefl, &sessionname, handle);

	if (WFR_STATUS_FAILURE(status) || donefl) {
		if (WFR_STATUS_FAILURE(status)) {
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "enumerating sessions");
		}
		return false;
	} else {
		sessionname_len = strlen(sessionname);
		for (size_t nchar = 0; nchar < sessionname_len; nchar++) {
			put_byte(out, sessionname[nchar]);
		}
		sfree(sessionname);
		return true;
	}
}

void
enum_settings_finish(
	settings_e *	handle
	)
{
	WFR_SFREE_IF_NOTNULL(handle);
}

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
	int				port,
	const char *	keytype,
	const char *	key
	)
{
	const char *	key_;
	char *			key_name;
	WfrStatus		status;


	if (WFR_STATUS_SUCCESS(status = WfsPrintHostKeyName(
							hostname, port, keytype, &key_name)))
	{
		status = WfsGetHostKey(WfsGetBackend(), false, key_name, &key_);
		sfree(key_name);
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
	Seat *			seat,
	const char *	hostname,
	int				port,
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
			sfree(key_name);
			if (WFR_STATUS_FAILURE(status)) {
				sfree(key_);
			}
		}
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "storing host key");
}

/* ----------------------------------------------------------------------
 * Cleanup function: remove all of PuTTY's persistent state.
 */

void
cleanup_all(
	void
	)
{
	WfsBackend	backend;


	backend = WfsGetBackend();
	(void)WfsClearHostKeys(backend, true);
	(void)WfsClearSessions(backend, true);
}

/*
 * Public wrapped PuTTY windows/jump-list.c subroutine prototypes
 */

void
add_session_to_jumplist(
	const char *const	sessionname
	)
{
	WfsJumpListAdd(WfsGetBackend(), sessionname);
}

void
clear_jumplist(
	void
	)
{
	WfsJumpListClear(WfsGetBackend());
}

void
remove_session_from_jumplist(
	const char *const	sessionname
	)
{
	WfsJumpListRemove(WfsGetBackend(), sessionname);
}

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
