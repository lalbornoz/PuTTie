/*
 * winfrip_storage_adapter_sessions.c - pointless frippery & tremendous amounts of bloat
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
#include "PuTTie/winfrip_rtl_windows.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_adapter.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_host_keys.h"
#include "PuTTie/winfrip_storage_jump_list.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"

/*
 * Private variables
 */

/*
 * Vector of key names of missing {int,string} settings to ignore
 * if absent in a debugging build
 */

#ifdef WINFRIP_DEBUG
static const char *	WfspIgnoreMissingIntSettings[] = {
	"BuggyMAC",
	"BugDHGEx2",
	"BugFilterKexinit",
	"NoRemoteQTitle",
};

static const char *	WfspIgnoreMissingStrSettings[] = {
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

static WfrStatus	WfspExpandFontSpecKeys(const char *key, char **pkey_charset, char **pkey_height, char **pkey_isbold);

/*
 * Private subroutines
 */

static WfrStatus
WfspExpandFontSpecKeys(
	const char *	key,
	char **		pkey_charset,
	char **		pkey_height,
	char **		pkey_isbold
	)
{
	size_t		key_size;
	WfrStatus	status;


	key_size = strlen(key) + 1;
	*pkey_charset = NULL;
	*pkey_height = NULL;
	*pkey_isbold = NULL;

	if (WFR_NEWN(status, (*pkey_charset), key_size + (sizeof("CharSet") - 1), char)
	&&  WFR_NEWN(status, (*pkey_height), key_size + (sizeof("Height") - 1), char)
	&&  WFR_NEWN(status, (*pkey_isbold), key_size + (sizeof("IsBold") - 1), char))
	{
		WFR_SNPRINTF(*pkey_charset, key_size + (sizeof("CharSet") - 1), "%sCharSet", key);
		WFR_SNPRINTF(*pkey_height, key_size + (sizeof("Height") - 1), "%sHeight", key);
		WFR_SNPRINTF(*pkey_isbold, key_size + (sizeof("IsBold") - 1), "%sIsBold", key);
	}

	if (WFR_FAILURE(status)) {
		WFR_FREE_IF_NOTNULL(*pkey_charset);
		WFR_FREE_IF_NOTNULL(*pkey_height);
		WFR_FREE_IF_NOTNULL(*pkey_isbold);
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
	char **		errmsg
	)
{
	WfsBackend	backend;
	WfsSession *	session;
	WfrStatus	status;


	(void)errmsg;

	WFS_SESSION_NAME_DEFAULT(sessionname);

	backend = WfsGetBackend();
	status = WfsAddSession(backend, sessionname, &session);
	if (WFR_STATUS_CONDITION(status) == EEXIST) {
		status = WfsClearSession(backend, session, NULL);
	}
	if (WFR_SUCCESS(status)) {
		return (settings_w *)session;
	} else {
		if (WfsGetAdapterDisplayErrors()) {
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, NULL, "opening session");
		}

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
	WfsSession *	session;
	WfrStatus	status;
	char *		value_new;
	size_t		value_new_size;


	session = (WfsSession *)handle;

	if (!(value_new = strdup(value))) {
		WFR_DEBUG_FAIL();
	} else {
		value_new_size = strlen(value_new) + 1;
		if (WFR_FAILURE(status = WfsSetSessionKey(
				session, key, value_new, value_new_size,
				WFR_TREE_ITYPE_STRING)))
		{
			WFR_FREE(value_new);
			WFR_DEBUG_FAIL();
		}
	}
}

void
write_setting_i(
	settings_w *	handle,
	const char *	key,
	int		value
	)
{
	WfsSession *	session;
	WfrStatus	status;
	int *		value_new;


	session = (WfsSession *)handle;

	if (WFR_NEW(status, value_new, int)) {
		*value_new = value;
		if (WFR_FAILURE(status = WfsSetSessionKey(
				session, key, value_new, sizeof(*value_new),
				WFR_TREE_ITYPE_INT)))
		{
			WFR_FREE(value_new);
			WFR_DEBUG_FAIL();
		}
	} else {
		WFR_DEBUG_FAIL();
	}
}

void
write_setting_filename(
	settings_w *	handle,
	const char *	key,
	Filename *	value
	)
{
	WfsSession *	session;
	WfrStatus	status;
	char *		value_new;
	size_t		value_new_size;


	session = (WfsSession *)handle;

	if (!(value_new = strdup(value->path))) {
		WFR_DEBUG_FAIL();
	} else {
		value_new_size = strlen(value_new) + 1;
		if (WFR_FAILURE(status = WfsSetSessionKey(
				session, key, value_new, value_new_size,
				WFR_TREE_ITYPE_STRING)))
		{
			WFR_FREE(value_new);
			WFR_DEBUG_FAIL();
		}
	}
}

void
write_setting_fontspec(
	settings_w *	handle,
	const char *	key,
	FontSpec *	font
	)
{
	char *		key_charset = NULL;
	char *		key_height = NULL;
	char *		key_isbold = NULL;
	WfsSession *	session;
	WfrStatus	status;
	int *		val_charset = NULL;
	int *		val_height = NULL;
	int *		val_isbold = NULL;
	char *		val_name = NULL;
	size_t		val_name_size;


	session = (WfsSession *)handle;

	if (WFR_SUCCESS(status = WfspExpandFontSpecKeys(
			key, &key_charset, &key_height, &key_isbold)))
	{
		val_name_size = strlen(font->name) + 1;

		if (!WFR_NEW(status, val_charset, int)
		||  !WFR_NEW(status, val_height, int)
		||  !WFR_NEW(status, val_isbold, int)
		||  !WFR_NEWN(status, val_name, val_name_size, char))
		{
			WFR_DEBUG_FAIL();
		} else {
			*val_charset = font->charset;
			*val_height = font->height;
			*val_isbold = font->isbold;
			strcpy(val_name, font->name);

			if (WFR_SUCCESS(status = WfsSetSessionKey(
					session, key_charset, val_charset,
					sizeof(*val_charset), WFR_TREE_ITYPE_INT))
			&&  WFR_SUCCESS(status = WfsSetSessionKey(
					session, key_height, val_height,
					sizeof(*val_height), WFR_TREE_ITYPE_INT))
			&&  WFR_SUCCESS(status = WfsSetSessionKey(
					session, key_isbold, val_isbold,
					sizeof(*val_isbold), WFR_TREE_ITYPE_INT))
			&&  WFR_SUCCESS(status = WfsSetSessionKey(
					session, key, val_name, val_name_size,
					WFR_TREE_ITYPE_STRING)))
			{
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		}

		WFR_FREE_IF_NOTNULL(key_charset);
		WFR_FREE_IF_NOTNULL(key_height);
		WFR_FREE_IF_NOTNULL(key_isbold);

		if (WFR_FAILURE(status)) {
			WFR_FREE_IF_NOTNULL(val_charset);
			WFR_FREE_IF_NOTNULL(val_height);
			WFR_FREE_IF_NOTNULL(val_isbold);
			WFR_FREE_IF_NOTNULL(val_name);
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
	WfsSession *	session;
	WfrStatus	status;


	session = (WfsSession *)handle;
	status = WfsSaveSession(WfsGetBackend(), session);

	if (WfsGetAdapterDisplayErrors()) {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, NULL, "closing session");
	}
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
	bool		defaultfl;
	WfsSession *	session;
	WfrStatus	status;


	defaultfl = WFS_SESSION_NAME_DEFAULT(sessionname);

	status = WfsGetSession(
		WfsGetBackend(), false,
		sessionname, &session);

	if (WFR_SUCCESS(status)) {
		return (settings_r *)session;
	} else {
		if (WFR_STATUS_IS_NOT_FOUND(status) && defaultfl) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		if (WfsGetAdapterDisplayErrors()) {
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, NULL, "opening session (read-only)");
		}

		return NULL;
	}
}

char *
read_setting_s(
	settings_r *	handle,
	const char *	key
	)
{
	WfsSession *	session;
	WfrStatus	status;
	void *		value;


	session = (WfsSession *)handle;
	if (!session) {
		return NULL;
	}

	if (WFR_SUCCESS(status = WfsGetSessionKey(
			session, key, WFR_TREE_ITYPE_STRING,
			&value, NULL)))
	{
		return strdup(value);
	} else if (WFS_SESSION_NAME_IS_DEFAULT(session->name)) {
		return NULL;
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
	int		defvalue
	)
{
	WfsSession *	session;
	WfrStatus	status;
	int		value;


	session = (WfsSession *)handle;
	if (!session) {
		return defvalue;
	}
	if (WFR_SUCCESS(status = WfsGetSessionKey(
			session, key, WFR_TREE_ITYPE_INT,
			(void *)&value, NULL)))
	{
		return value;
	} else if (WFS_SESSION_NAME_IS_DEFAULT(session->name)) {
		return defvalue;
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
	WfsSession *	session;
	WfrStatus	status;
	Filename *	value = NULL;


	session = (WfsSession *)handle;
	if (!session) {
		return NULL;
	}

	if (WFR_NEW(status, value, Filename)
	&&  WFR_SUCCESS(status = WfsGetSessionKey(
			session, key, WFR_TREE_ITYPE_STRING,
			(void *)&value->path, NULL)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WFR_SUCCESS(status)) {
		if (!(value->path = strdup(value->path))) {
			WFR_DEBUG_FAIL();
			WFR_FREE(value);
			return NULL;
		} else {
			return value;
		}
	} else if (WFS_SESSION_NAME_IS_DEFAULT(session->name)) {
		return NULL;
	} else {
		WFR_DEBUG_FAIL();
		WFR_FREE_IF_NOTNULL(value);
		return NULL;
	}
}

FontSpec *
read_setting_fontspec(
	settings_r *	handle,
	const char *	key
	)
{
	char *		key_charset = NULL;
	char *		key_height = NULL;
	char *		key_isbold = NULL;
	WfsSession *	session;
	WfrStatus	status;
	FontSpec *	value = NULL;


	session = (WfsSession *)handle;
	if (!session) {
		return NULL;
	}

	if (WFR_NEW(status, value, FontSpec)) {
		value->name = NULL;
		if (WFR_SUCCESS(status = WfspExpandFontSpecKeys(
				key, &key_charset, &key_height, &key_isbold))
		&&  WFR_SUCCESS(status = WfsGetSessionKey(
				session, key_charset, WFR_TREE_ITYPE_INT,
				(void **)&value->charset, NULL))
		&&  WFR_SUCCESS(status = WfsGetSessionKey(
				session, key_height, WFR_TREE_ITYPE_INT,
				(void **)&value->height, NULL))
		&&  WFR_SUCCESS(status = WfsGetSessionKey(
				session, key_isbold, WFR_TREE_ITYPE_INT,
				(void **)&value->isbold, NULL))
		&&  WFR_SUCCESS(status = WfsGetSessionKey(
				session, key, WFR_TREE_ITYPE_STRING,
				(void **)&value->name, NULL))
		&&  WFR_SUCCESS_POSIX(status, (value->name = strdup(value->name))))
		{
			status = WFR_STATUS_CONDITION_SUCCESS;
		}
	}

	WFR_FREE_IF_NOTNULL(key_charset);
	WFR_FREE_IF_NOTNULL(key_height);
	WFR_FREE_IF_NOTNULL(key_isbold);

	if (WFR_SUCCESS(status)) {
		return value;
	} else if (WFS_SESSION_NAME_IS_DEFAULT(session->name)) {
		return NULL;
	} else {
		if (value) {
			WFR_FREE_IF_NOTNULL(value->name);
		}
		WFR_FREE_IF_NOTNULL(value);

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
		status = WfsCloseSession(WfsGetBackend(), (WfsSession *)handle);
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WfsGetAdapterDisplayErrors()) {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, NULL, "closing session (read-only)");
	}
}

/*
 * Delete a whole saved session.
 */

void
del_settings(
	const char *	sessionname
	)
{
	bool		defaultfl;
	WfrStatus	status;


	defaultfl = WFS_SESSION_NAME_DEFAULT(sessionname);

	status = WfsDeleteSession(WfsGetBackend(), true, sessionname);

	if (WFR_STATUS_IS_NOT_FOUND(status) && defaultfl) {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	if (WfsGetAdapterDisplayErrors()) {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, NULL, "deleting session");
	}
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


	if (WFR_SUCCESS(status = WfsEnumerateSessions(
			WfsGetBackend(), false, true,
			NULL, NULL, &handle)))
	{
		return handle;
	} else {
		if (WfsGetAdapterDisplayErrors()) {
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, NULL, "enumerating sessions");
		}

		return NULL;
	}
}

bool
enum_settings_next(
	settings_e *	handle,
	strbuf *	out
	)
{
	bool		donefl;
	char *		sessionname;
	size_t		sessionname_len;
	WfrStatus	status;


	status = WfsEnumerateSessions(
		WfsGetBackend(), false, false,
		&donefl, &sessionname, (void **)&handle);

	if (WFR_FAILURE(status) || donefl) {
		if (WfsGetAdapterDisplayErrors() && WFR_FAILURE(status)) {
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, NULL, "enumerating sessions");
		}

		return false;
	} else {
		sessionname_len = strlen(sessionname);
		for (size_t nchar = 0; nchar < sessionname_len; nchar++) {
			put_byte(out, sessionname[nchar]);
		}
		WFR_FREE(sessionname);
		return true;
	}
}

void
enum_settings_finish(
	settings_e *	handle
	)
{
	(void)handle;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
