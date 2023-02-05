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
#include "PuTTie/winfrip_storage.h"
#include "PuTTie/winfrip_storage_privkey_list.h"
#include "PuTTie/winfrip_storage_sessions.h"
#include "PuTTie/winfrip_subr_pageant.h"

/*
 * External subroutines private to windows/pageant.c
 */

const char *pageant_get_nth_ssh1_key_path(int idx);
const char *pageant_get_nth_ssh2_key_path(int idx);

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
	WfrStatus	status;


	if (win_add_keyfile(fn, encrypted) == PAGEANT_ACTION_OK) {
		status = WfsAddPrivKeyList(WfsGetBackend(), fn->path);
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "adding %s to Pageant private key list", fn->path);
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
	char *			privkey_list = NULL;
	WfrStatus		status;


	backend = WfsGetBackend();

	if (nclkeys > 0) {
		/*
		 * Add any keys provided on the command line.
		 */

		status = WfsClearPrivKeyList(backend);
		WFR_IF_STATUS_FAILURE_MESSAGEBOX1("Pageant", status, "clearing Pageant private key list");
		for (size_t nclkey = 0; nclkey < nclkeys; nclkey++) {
			clkey = &clkeys[nclkey];
			if (win_add_keyfile(clkey->fn, clkey->add_encrypted) == PAGEANT_ACTION_OK) {
				status = WfsAddPrivKeyList(backend, clkey->fn->path);
				WFR_IF_STATUS_FAILURE_MESSAGEBOX1(
					"Pageant", status,
					"adding %s to Pageant private key list", clkey->fn->path);
			}
			filename_free(clkey->fn);
		}
		sfree(clkeys);
	} else {
		status = WfsGetEntriesPrivKeyList(backend, &privkey_list, NULL);
		WFR_IF_STATUS_FAILURE_MESSAGEBOX1("Pageant", status, "getting Pageant private key list");

		for (char *privkey = privkey_list, *privkey_next = NULL;
		     privkey && *privkey; privkey = privkey_next)
		{
			if ((privkey_next = strchr(privkey, '\0'))) {
				if (win_add_keyfile(
						filename_from_str(privkey),
						add_keys_encrypted) == PAGEANT_ACTION_OK)
				{
					status = WfsAddPrivKeyList(WfsGetBackend(), privkey);
					WFR_IF_STATUS_FAILURE_MESSAGEBOX1("Pageant", status, "adding %s to Pageant private key list", privkey);
				} else {
					status = WfsRemovePrivKeyList(WfsGetBackend(), privkey);
					WFR_IF_STATUS_FAILURE_MESSAGEBOX1("Pageant", status, "deleting %s from Pageant private key list", privkey);
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
	WFR_IF_STATUS_FAILURE_MESSAGEBOX1("Pageant", status, "getting argument string");
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
	WFR_IF_STATUS_FAILURE_MESSAGEBOX1("Pageant", status, "getting argument string");
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
WfPageantDeleteAllKeys(
	void	(*pageant_delete_all)(void)
	)
{
	WfrStatus	status;


	pageant_delete_all();
	status = WfsClearPrivKeyList(WfsGetBackend());
	WFR_IF_STATUS_FAILURE_MESSAGEBOX1("Pageant", status, "clearing Pageant private key list");
}

void
WfPageantDeleteKey(
	int		nkey,
	bool		(*pageant_delete_nth_key)(int),
	const char *	(*pageant_get_nth_key_path)(int)
	)
{
	char *		path;
	WfrStatus	status;


	if (!(path = strdup(pageant_get_nth_key_path(nkey)))) {
		status = WFR_STATUS_FROM_ERRNO();
	} else if (pageant_delete_nth_key(nkey)) {
		status = WfsRemovePrivKeyList(WfsGetBackend(), path);
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "deleting %s from Pageant private key list", path);
	WFR_FREE(path);
}

void
WfPageantInit(
	LPSTR *		pcmdline
	)
{
	WfrStatus	status;


	WfrDebugInit();

	if (WFR_STATUS_FAILURE(status = WfsInit())) {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX1("Pageant", status, "initialising storage");
		exit(1);
	} else if (WFR_STATUS_FAILURE(status = WfsSetBackendFromCmdLine(*pcmdline))) {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX1("Pageant", status, "setting backend");
		exit(1);
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
	int		nentries;
	char *		sessionname;
	char *		sessionname_new = NULL;
	WfrStatus	status;


	if (!putty_path) {
		return;
	}

	backend = WfsGetBackend();
	if (WFR_STATUS_FAILURE(status = WfsEnumerateSessions(
			backend, false, true,
			NULL, NULL, &enum_state)))
	{
		error_context = "enumerating sessions";
		goto out;
	}

	for(nentries = GetMenuItemCount(session_menu);
	    nentries > initial_menuitems_count;
	    nentries--);
	RemoveMenu(session_menu, 0, MF_BYPOSITION);

	error_context = "enumerating sessions";
	idx_menu = 0;

	while (WFR_STATUS_SUCCESS(status = WfsEnumerateSessions(
			backend, false, false,
			&donefl, &sessionname, &enum_state)) && !donefl)
	{
		if (strcmp(sessionname, putty_default) != 0) {
			if (!(sessionname_new = strdup(sessionname))) {
				status = WFR_STATUS_FROM_ERRNO();
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

	WFR_IF_STATUS_FAILURE_MESSAGEBOX1("Pageant", status, "%s", error_context);
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
