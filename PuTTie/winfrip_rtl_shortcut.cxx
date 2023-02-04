/*
 * winfrip_rtl_shortcut.cxx - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#define _UNICODE
#define UNICODE

#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <wchar.h>

#include <windows.h>
#include <winnls.h>
#include <shobjidl.h>
#include <objbase.h>
#include <objidl.h>
#include <shlguid.h>
#include <shlobj.h>
#include <propkey.h>
#include <propvarutil.h>

#include "winfrip_rtl.h"
#include "winfrip_rtl_file.h"
#include "winfrip_rtl_shortcut.hpp"

/*
 * Public subroutines
 */

WfrStatus
WfrCreateShortcut(
	LPCWSTR		app_user_model_id,
	LPCWSTR		description,
	LPCWSTR		shortcut_pname,
	LPCWSTR		target_pname,
	LPCWSTR		working_dname
	)
{
	HRESULT			hres;
	IShellLink *		psl = NULL;
	IPersistFile *		ppf = NULL;
	IPropertyStore *	pps = NULL;
	PROPVARIANT		propvar;
	LPWSTR			shortcut_pname_abs = NULL;
	WfrStatus		status;
	LPWSTR			target_pname_abs = NULL;
	LPWSTR			working_dname_full = NULL;


	if (WFR_STATUS_FAILURE(status = WfrPathNameToAbsoluteW(shortcut_pname, &shortcut_pname_abs))
	||  WFR_STATUS_FAILURE(status = WfrPathNameToAbsoluteW(target_pname, &target_pname_abs)))
	{
		goto out;
	}
	if (working_dname) {
		working_dname_full = (LPWSTR)working_dname;
	} else if (WFR_STATUS_FAILURE(status = WfrPathNameToDirectoryW(
			target_pname_abs, &working_dname_full)))
	{
		goto out;
	}


	hres = CoInitialize(NULL);
	if ((hres != S_OK) && (hres != S_FALSE)) {
		status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
		goto out;
	}

	hres = CoCreateInstance(
		CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
		IID_IShellLink, (LPVOID *)&psl);
	if (FAILED(hres)) {
		status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
		goto out;
	} else {
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
		if (FAILED(hres)) {
			status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
			goto out;
		} else {
			hres = psl->QueryInterface(IID_IPropertyStore, (LPVOID *)&pps);
			if (FAILED(hres)) {
				status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
				goto out;
			}
		}
	}

	hres = psl->SetPath(target_pname_abs);
	if (FAILED(hres)) {
		status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
		goto out;
	} else {
		hres = psl->SetDescription(description);
		if (FAILED(hres)) {
			status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
			goto out;
		} else {
			hres = psl->SetWorkingDirectory(working_dname_full);
			if (FAILED(hres)) {
				status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
				goto out;
			}
		}
	}

	if (app_user_model_id) {
		hres = InitPropVariantFromString(app_user_model_id, &propvar);
		if (FAILED(hres)) {
			status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
			goto out;
		} else {
			hres = pps->SetValue(PKEY_AppUserModel_ID, propvar);
			(void)PropVariantClear(&propvar);
			if (FAILED(hres)) {
				status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
				goto out;
			}
		}
	}

	hres = pps->Commit();
	if (FAILED(hres)) {
		status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
		goto out;
	}
	hres = ppf->Save(shortcut_pname_abs, TRUE);
	if (FAILED(hres)) {
		status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
		goto out;
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

out:
	WFR_RELEASE_IF_NOTNULL(pps);
	WFR_RELEASE_IF_NOTNULL(ppf);
	WFR_RELEASE_IF_NOTNULL(psl);

	WFR_FREE_IF_NOTNULL(shortcut_pname_abs);
	WFR_FREE_IF_NOTNULL(target_pname_abs);
	WFR_FREE_IF_NOTNULL(working_dname_full);

	return status;
}

WfrStatus
WfrCreateShortcutStartup(
	LPCWSTR		app_user_model_id,
	LPCWSTR		description,
	LPCWSTR		shortcut_fname,
	LPCWSTR		target_pname,
	LPCWSTR		working_dname
	)
{
	HRESULT		hres;
	PWSTR		startup_dname_abs = NULL;
	WCHAR 		shortcut_pname[PATH_MAX + 1];
	WfrStatus	status;


	hres = SHGetKnownFolderPath(
		FOLDERID_Startup, 0, NULL, &startup_dname_abs);
	if (FAILED(hres)) {
		status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
	} else {
		WFR_SNWPRINTF(
			shortcut_pname, WFR_SIZEOF_WSTRING(shortcut_pname),
			L"%S\\%S", startup_dname_abs, shortcut_fname);
		status = WfrCreateShortcut(
			app_user_model_id, description, shortcut_pname,
			target_pname, working_dname);
	}

	CoTaskMemFree(startup_dname_abs);

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=100
 */
