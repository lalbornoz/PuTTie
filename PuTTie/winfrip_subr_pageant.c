/*
 * winfrip_subr_pageant.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "pageant.h"
#pragma GCC diagnostic pop

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <windows.h>
#include <tchar.h>

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_debug.h"
#include "PuTTie/winfrip_rtl_shortcut.h"
#include "PuTTie/winfrip_rtl_windows.h"
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_host_ca.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_storage_priv.h"
#include "PuTTie/winfrip_storage_privkey_list.h"
#include "PuTTie/winfrip_storage_options.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_subr_pageant.h"

/*
 * Private constants
 */

#define PAGEANT_PATH_DEFAULT			L"pageant.exe"
#define PAGEANT_SHORTCUT_DESCRIPTION		L"PuTTY SSH authentication agent"
#define PAGEANT_SHORTCUT_PATH_DEFAULT		L"Pageant.lnk"

#define PAGEANT_OPTION_LAUNCH_AT_STARTUP	"PageantLaunchAtStartup"
#define PAGEANT_OPTION_PERSIST_KEYS		"PageantPersistKeys"

/*
 * External subroutines private to windows/pageant.c
 */

const char *	pageant_get_nth_ssh1_key_path(int idx);
const char *	pageant_get_nth_ssh2_key_path(int idx);

/*
 * Private subroutine prototypes
 */

WfrStatus	WfpPageantCommandLaunchAtStartup(int option);

/*
 * Private subroutines
 */

WfrStatus
WfpPageantCommandLaunchAtStartup(
	int	option
	)
{
	WfrStatus	status;


	switch (option) {
	default:
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
		break;

	case false:
		status = WfrDeleteShortcutStartup(PAGEANT_SHORTCUT_PATH_DEFAULT);
		if (WFR_STATUS_IS_NOT_FOUND(status)) {
			status = WFR_STATUS_CONDITION_SUCCESS;
		}

		break;

	case true:
		status = WfrCreateShortcutStartup(
			NULL, PAGEANT_SHORTCUT_DESCRIPTION,
			PAGEANT_SHORTCUT_PATH_DEFAULT, PAGEANT_PATH_DEFAULT, NULL);
		break;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX1(
		NULL, "Pageant", status,
		"creating/deleting Pageant launch at startup shortcut");

	return status;
}

/*
 * Public subroutines private to windows/pageant.c
 */

void
WfPageantAddKey(
	bool		encrypted,
	Filename *	fn,
	int		(*win_add_keyfile)(Filename *, bool)
	)
{
	int * 		option_persist;
	WfrStatus	status;


	if (win_add_keyfile(fn, encrypted) == PAGEANT_ACTION_OK) {
		if (WFR_SUCCESS(status = WfsGetOptionIntWithDefault(
				WfsGetBackend(), PAGEANT_OPTION_PERSIST_KEYS, false, &option_persist))
		&&  (*option_persist == true))
		{
			status = WfsAddPrivKeyList(WfsGetBackend(), fn->utf8path);
		}

		WFR_IF_STATUS_FAILURE_MESSAGEBOX(
			status, NULL,
			"adding %s to Pageant private key list", fn->utf8path);
	}
}

void
WfPageantAddKeysFromCmdLine(
	bool			add_keys_encrypted,
	CommandLineKey *	clkeys,
	size_t			nclkeys,
	int			(*win_add_keyfile)(Filename *, bool)
	)
{
	WfsBackend		backend;
	CommandLineKey *	clkey;
	int *			option_persist;
	char *			privkey_list = NULL;
	WfrStatus		status;


	backend = WfsGetBackend();

	if (WFR_FAILURE(status = WfsGetOptionIntWithDefault(
			WfsGetBackend(), PAGEANT_OPTION_PERSIST_KEYS, false, &option_persist)))
	{
		WFR_IF_STATUS_FAILURE_MESSAGEBOX1(
			NULL, "Pageant", status,
			"getting Pageant persist keys option");
		exit(1);
	}

	if (nclkeys > 0) {
		/*
		 * Add any keys provided on the command line.
		 */

		if (*option_persist == true) {
			status = WfsClearPrivKeyList(backend);
			WFR_IF_STATUS_FAILURE_MESSAGEBOX1(
				NULL, "Pageant", status,
				"clearing Pageant private key list");
		}

		for (size_t nclkey = 0; nclkey < nclkeys; nclkey++) {
			clkey = &clkeys[nclkey];
			if (win_add_keyfile(clkey->fn, clkey->add_encrypted) == PAGEANT_ACTION_OK) {
				if (*option_persist == true) {
					status = WfsAddPrivKeyList(backend, clkey->fn->utf8path);
					WFR_IF_STATUS_FAILURE_MESSAGEBOX1(
						NULL, "Pageant", status,
						"adding %s to Pageant private key list", clkey->fn->utf8path);
				}
			}
			filename_free(clkey->fn);
		}
		sfree(clkeys);
	} else if (*option_persist == true) {
		status = WfsGetEntriesPrivKeyList(backend, &privkey_list, NULL);
		WFR_IF_STATUS_FAILURE_MESSAGEBOX1(
			NULL, "Pageant", status,
			"getting Pageant private key list");

		for (char *privkey = privkey_list, *privkey_next = NULL;
		     privkey && *privkey; privkey = privkey_next)
		{
			if ((privkey_next = strchr(privkey, '\0'))) {
				if (win_add_keyfile(
						filename_from_str(privkey),
						add_keys_encrypted) == PAGEANT_ACTION_OK)
				{
					status = WfsAddPrivKeyList(WfsGetBackend(), privkey);
					WFR_IF_STATUS_FAILURE_MESSAGEBOX1(
						NULL, "Pageant", status,
						"adding %s to Pageant private key list",
						privkey);
				} else {
					status = WfsRemovePrivKeyList(WfsGetBackend(), privkey);
					WFR_IF_STATUS_FAILURE_MESSAGEBOX1(
						NULL, "Pageant", status,
						"deleting %s from Pageant private key list",
						privkey);
				}
				privkey_next++;
			}
		}
	}
}

void
WfPageantAppendCmdLineBackendArgString(
	TCHAR *		cmdline,
	size_t		cmdline_size
	)
{
	char *		backend_arg_string = NULL;
	WfrStatus	status;

	status = WfsGetBackendArgString(&backend_arg_string);
	WFR_IF_STATUS_FAILURE_MESSAGEBOX1(
		NULL, "Pageant", status,
		"getting argument string");

	if (backend_arg_string) {
		if (strlen(cmdline) > 0) {
			strncat(cmdline, " ", cmdline_size);
			cmdline[cmdline_size - 1] = '\0';
		}
		strncat(cmdline, backend_arg_string, cmdline_size);
		cmdline[cmdline_size - 1] = '\0';
	}
}

void
WfPageantAppendParamBackendArgString(
	TCHAR *		param,
	size_t		param_size
	)
{
	char *		backend_arg_string;
	WfrStatus	status;

	status = WfsGetBackendArgString(&backend_arg_string);
	WFR_IF_STATUS_FAILURE_MESSAGEBOX1(
		NULL, "Pageant", status,
		"getting argument string");

	if (backend_arg_string) {
		if (strlen(param) > 0) {
			strcat(param, " ");
			param[param_size - 1] = '\0';
		}
		strcat(param, backend_arg_string);
		param[param_size - 1] = '\0';
		strcat(param, " ");
		param[param_size - 1] = '\0';
	}
}

void
WfPageantCommandLaunchAtStartup(
	HMENU		systray_menu,
	UINT_PTR	idm_launch_at_startup
	)
{
	WfsBackend	backend;
	int *		option;
	WfrStatus	status;


	backend = WfsGetBackend();


	if (WFR_SUCCESS(status = WfsGetOptionIntWithDefault(
			WfsGetBackend(), PAGEANT_OPTION_LAUNCH_AT_STARTUP, false, &option)))
	{
		*option = ((*option == true) ? false : true);
		status = WfsSetOption(
			backend, false, true, PAGEANT_OPTION_LAUNCH_AT_STARTUP,
			option, sizeof(*option), WFR_TREE_ITYPE_INT);
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX1(
		NULL, "Pageant", status,
		"getting/setting Pageant launch at startup option");

	if (WFR_SUCCESS(status)) {
		status = WfpPageantCommandLaunchAtStartup(*option);
		if (WFR_SUCCESS(status)) {
			(void)CheckMenuItem(
				systray_menu, idm_launch_at_startup,
				  MF_BYCOMMAND
				| ((*option == true) ? MF_CHECKED : MF_UNCHECKED));
		}
	}
}

void
WfPageantCommandPersistKeys(
	HMENU		systray_menu,
	UINT_PTR	idm_persist_keys
	)
{
	WfsBackend	backend;
	int *		option;
	WfrStatus	status;


	backend = WfsGetBackend();


	if (WFR_SUCCESS(status = WfsGetOptionIntWithDefault(
			WfsGetBackend(), PAGEANT_OPTION_PERSIST_KEYS, false, &option)))
	{
		*option = ((*option == true) ? false : true);
		status = WfsSetOption(
			backend, false, true, PAGEANT_OPTION_PERSIST_KEYS,
			option, sizeof(*option), WFR_TREE_ITYPE_INT);
	}

	if (WFR_SUCCESS(status)) {
		switch (*option) {
		default:
			status = WFR_STATUS_FROM_ERRNO1(EINVAL);
			break;

		case false:
		case true:
			break;
		}

		if (WFR_SUCCESS(status)) {
			(void)CheckMenuItem(
				systray_menu, idm_persist_keys,
				  MF_BYCOMMAND
				| ((*option == true) ? MF_CHECKED : MF_UNCHECKED));
		}
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX1(
		NULL, "Pageant", status,
		"getting/setting Pageant persist keys option");
}

void
WfPageantDeleteAllKeys(
	void	(*pageant_delete_all)(void)
	)
{
	int *		option;
	WfrStatus	status;


	pageant_delete_all();

	if (WFR_SUCCESS(status = WfsGetOptionIntWithDefault(
			WfsGetBackend(), PAGEANT_OPTION_PERSIST_KEYS, false, &option))
	&&  (*option == true))
	{
		status = WfsClearPrivKeyList(WfsGetBackend());
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX1(
		NULL, "Pageant", status,
		"clearing Pageant private key list");
}

void
WfPageantDeleteKey(
	int		nkey,
	bool		(*pageant_delete_nth_key)(int),
	const char *	(*pageant_get_nth_key_path)(int)
	)
{
	int *		option;
	char *		path = NULL;
	WfrStatus	status;


	if (WFR_SUCCESS_POSIX(status, (path = strdup(pageant_get_nth_key_path(nkey))))
	&&  pageant_delete_nth_key(nkey)
	&&  WFR_SUCCESS(status = WfsGetOptionIntWithDefault(
			WfsGetBackend(), PAGEANT_OPTION_PERSIST_KEYS, false, &option))
	&&  (*option == true)
	&&  WFR_SUCCESS(status = WfsRemovePrivKeyList(WfsGetBackend(), path)))
	{
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, NULL, "deleting %s from Pageant private key list", path);
	WFR_FREE_IF_NOTNULL(path);
}

void
WfPageantInit(
	LPSTR *		pcmdline
	)
{
	char *		cmdline;
	WfrStatus	status;


	WfrDebugInit();

	if (WFR_FAILURE(status = WfsInit())) {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX1(NULL, "Pageant", status, "initialising storage");
		exit(1);
	} else if (WFR_FAILURE(status = WfrGetCommandLineAsUtf8(&cmdline))) {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX1(NULL, "Pageant", status, "initialising command line");
		exit(1);
	} else if (WFR_FAILURE(status = WfsSetBackendFromCmdLine(cmdline))) {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX1(NULL, "Pageant", status, "setting backend");
		exit(1);
	}

	*pcmdline = cmdline;
}

void
WfPageantInitSysTrayMenu(
	HMENU		systray_menu,
	UINT_PTR	idm_launch_at_startup,
	UINT_PTR	idm_persist_keys
	)
{
	int *		option_launch_at_startup;
	int *		option_persist;
	WfrStatus	status;



	if (WFR_FAILURE(status = WfsGetOptionIntWithDefault(
			WfsGetBackend(), PAGEANT_OPTION_LAUNCH_AT_STARTUP, false, &option_launch_at_startup)))
	{
		WFR_IF_STATUS_FAILURE_MESSAGEBOX1(NULL, "Pageant", status, "getting Pageant launch at startup option");
		exit(1);
	}
	if (WFR_FAILURE(status = WfsGetOptionIntWithDefault(
			WfsGetBackend(), PAGEANT_OPTION_PERSIST_KEYS, false, &option_persist)))
	{
		WFR_IF_STATUS_FAILURE_MESSAGEBOX1(NULL, "Pageant", status, "getting Pageant persist keys option");
		exit(1);
	}

	(void)AppendMenu(systray_menu, MF_ENABLED, idm_launch_at_startup, "&Launch at startup");
	(void)AppendMenu(systray_menu, MF_ENABLED, idm_persist_keys, "&Persist keys");
	(void)AppendMenu(systray_menu, MF_SEPARATOR, 0, 0);

	status = WfpPageantCommandLaunchAtStartup(*option_launch_at_startup);
	if (WFR_SUCCESS(status)) {
		(void)CheckMenuItem(
			systray_menu, idm_launch_at_startup,
			  MF_BYCOMMAND
			| ((*option_launch_at_startup == true) ? MF_CHECKED : MF_UNCHECKED));
	}

	switch (*option_persist) {
	default:
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
		break;

	case false:
	case true:
		(void)CheckMenuItem(
			systray_menu, idm_persist_keys,
			  MF_BYCOMMAND
			| ((*option_persist == true) ? MF_CHECKED : MF_UNCHECKED));
		break;
	}
}

void
WfPageantUpdateSessions(
	UINT	idm_sessions_base,
	int	initial_menuitems_count,
	char *	putty_default,
	char *	putty_path,
	HMENU	session_menu
	)
{
	WfsBackend	backend;
	bool		donefl;
	void *		enum_state = NULL;
	const char *	error_context;
	int		idx_menu;
	MENUITEMINFO	mii;
	char *		sessionname;
	char *		sessionname_new = NULL;
	WfrStatus	status;


	if (!putty_path) {
		return;
	}

	backend = WfsGetBackend();
	if (WFR_FAILURE(status = WfsEnumerateSessions(
			backend, false, true,
			NULL, NULL, &enum_state)))
	{
		error_context = "enumerating sessions";
		goto out;
	}

	for (int nentry = GetMenuItemCount(session_menu);
	     nentry > initial_menuitems_count;
	     nentry--)
	{
		(void)RemoveMenu(session_menu, nentry, MF_BYPOSITION);
	}

	error_context = "enumerating sessions";
	idx_menu = 0;

	while (WFR_SUCCESS(status = WfsEnumerateSessions(
			backend, false, false,
			&donefl, &sessionname, &enum_state)) && !donefl)
	{
		if (strcmp(sessionname, putty_default) != 0) {
			if (WFR_FAILURE_POSIX(status, (sessionname_new = strdup(sessionname)))) {
				error_context = "duplicating session name string";
				goto out;
			}

			memset(&mii, 0, sizeof(mii));
			mii.cbSize = sizeof(mii);
			mii.dwTypeData = sessionname_new;
			mii.fMask = MIIM_ID | MIIM_STATE | MIIM_TYPE;
			mii.fState = MFS_ENABLED;
			mii.fType = MFT_STRING;
			mii.wID = (idx_menu * 16) + idm_sessions_base;

			(void)InsertMenuItem(session_menu, idx_menu, true, &mii);
			idx_menu++;
		}
	}

	if (idx_menu == 0) {
		memset(&mii, 0, sizeof(mii));
		mii.cbSize = sizeof(mii);
		mii.dwTypeData = _T("(No sessions)");
		mii.fMask = MIIM_STATE | MIIM_TYPE;
		mii.fState = MFS_GRAYED;
		mii.fType = MFT_STRING;
		(void)InsertMenuItem(session_menu, idx_menu, true, &mii);
	}

out:
	if (enum_state) {
		(void)WfsEnumerateCancel(backend, &enum_state);
	}
	WFR_FREE_IF_NOTNULL(sessionname_new);

	WFR_IF_STATUS_FAILURE_MESSAGEBOX1(
		NULL, "Pageant", status,
		"%s", error_context);
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
