/*
 * winfrip_rtl_shortcut.cxx - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#define _UNICODE
#define UNICODE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <wchar.h>

#include <windows.h>
#include <knownfolders.h>
#include <propkey.h>
#include <propvarutil.h>

#include "winfrip_rtl.h"
#include "winfrip_rtl_file.h"
#include "winfrip_rtl_shortcut.h"

/*
 * Private COM definitions
 */

/* {{{ typedef struct IShellLinkWVtbl */
typedef struct IShellLinkWVtbl {
	HRESULT (__stdcall *QueryInterface)(
		/* [in] IShellLink*/ void *This,
		/* [in] */ const GUID * const riid,
		/* [out] */ void **ppvObject);

	ULONG (__stdcall *AddRef)(
		/* [in] IShellLink*/ void *This);

	ULONG (__stdcall *Release)(
		/* [in] IShellLink*/ void *This);

	HRESULT (__stdcall *GetPath)(
		/* [in] IShellLink*/ void *This,
		/* [string][out] */ LPWSTR pszFile,
		/* [in] */ int cch,
		/* [unique][out][in] */ WIN32_FIND_DATAA *pfd,
		/* [in] */ DWORD fFlags);

	HRESULT (__stdcall *GetIDList)(
		/* [in] IShellLink*/ void *This,
		/* [out] LPITEMIDLIST*/ void **ppidl);

	HRESULT (__stdcall *SetIDList)(
		/* [in] IShellLink*/ void *This,
		/* [in] LPITEMIDLIST*/ void *pidl);

	HRESULT (__stdcall *GetDescription)(
		/* [in] IShellLink*/ void *This,
		/* [string][out] */ LPWSTR pszName,
		/* [in] */ int cch);

	HRESULT (__stdcall *SetDescription)(
		/* [in] IShellLink*/ void *This,
		/* [string][in] */ LPCWSTR pszName);

	HRESULT (__stdcall *GetWorkingDirectory)(
		/* [in] IShellLink*/ void *This,
		/* [string][out] */ LPWSTR pszDir,
		/* [in] */ int cch);

	HRESULT (__stdcall *SetWorkingDirectory)(
		/* [in] IShellLink*/ void *This,
		/* [string][in] */ LPCWSTR pszDir);

	HRESULT (__stdcall *GetArguments)(
		/* [in] IShellLink*/ void *This,
		/* [string][out] */ LPWSTR pszArgs,
		/* [in] */ int cch);

	HRESULT (__stdcall *SetArguments)(
		/* [in] IShellLink*/ void *This,
		/* [string][in] */ LPCWSTR pszArgs);

	HRESULT (__stdcall *GetHotkey)(
		/* [in] IShellLink*/ void *This,
		/* [out] */ WORD *pwHotkey);

	HRESULT (__stdcall *SetHotkey)(
		/* [in] IShellLink*/ void *This,
		/* [in] */ WORD wHotkey);

	HRESULT (__stdcall *GetShowCmd)(
		/* [in] IShellLink*/ void *This,
		/* [out] */ int *piShowCmd);

	HRESULT (__stdcall *SetShowCmd)(
		/* [in] IShellLink*/ void *This,
		/* [in] */ int iShowCmd);

	HRESULT (__stdcall *GetIconLocation)(
		/* [in] IShellLink*/ void *This,
		/* [string][out] */ LPWSTR pszIconPath,
		/* [in] */ int cch,
		/* [out] */ int *piIcon);

	HRESULT (__stdcall *SetIconLocation)(
		/* [in] IShellLink*/ void *This,
		/* [string][in] */ LPCWSTR pszIconPath,
		/* [in] */ int iIcon);

	HRESULT (__stdcall *SetRelativePath)(
		/* [in] IShellLink*/ void *This,
		/* [string][in] */ LPCWSTR pszPathRel,
		/* [in] */ DWORD dwReserved);

	HRESULT (__stdcall *Resolve)(
		/* [in] IShellLink*/ void *This,
		/* [unique][in] */ HWND hwnd,
		/* [in] */ DWORD fFlags);

	HRESULT (__stdcall *SetPath)(
		/* [in] IShellLink*/ void *This,
		/* [string][in] */ LPCWSTR pszFile);
} IShellLinkWVtbl;
/* }}} */

typedef struct IShellLinkW {
	IShellLinkWVtbl *lpVtbl;
} IShellLinkW;

static const CLSID CLSID_ShellLinkW = {
	0x00021401, 0x0000, 0x0000, {0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}
};

static const IID IID_IShellLinkW = {
	0x000214f9, 0x0000, 0x0000, {0xc0,0x00, 0x00,0x00,0x00,0x00,0x00,0x46}
};

/* {{{ typedef struct IPropertyStoreVtbl */
typedef struct IPropertyStoreVtbl
{
	HRESULT (__stdcall *QueryInterface)(
		/* [in] IPropertyStore*/ void *This,
		/* [in] */ const GUID * const riid,
		/* [iid_is][out] */ void **ppvObject);

	ULONG (__stdcall *AddRef)(
		/* [in] IPropertyStore*/ void *This);

	ULONG (__stdcall *Release)(
		/* [in] IPropertyStore*/ void *This);

	HRESULT (__stdcall *GetCount)(
		/* [in] IPropertyStore*/ void *This,
		/* [out] */ DWORD *cProps);

	HRESULT (__stdcall *GetAt)(
		/* [in] IPropertyStore*/ void *This,
		/* [in] */ DWORD iProp,
		/* [out] */ PROPERTYKEY *pkey);

	HRESULT (__stdcall *GetValue)(
		/* [in] IPropertyStore*/ void *This,
		/* [in] */ const PROPERTYKEY * const key,
		/* [out] */ PROPVARIANT *pv);

	HRESULT (__stdcall *SetValue)(
		/* [in] IPropertyStore*/ void *This,
		/* [in] */ const PROPERTYKEY * const key,
		/* [in] */ REFPROPVARIANT propvar);

	HRESULT (__stdcall *Commit)(
		/* [in] IPropertyStore*/ void *This);
} IPropertyStoreVtbl;
/* }}} */

typedef struct IPropertyStore {
	IPropertyStoreVtbl *lpVtbl;
} IPropertyStore;

static const IID IID_IPropertyStore = {
	0x886d8eeb, 0x8cf2, 0x4446, {0x8d,0x02,0xcd,0xba,0x1d,0xbd,0xcf,0x99}
};

/*
 * Type-checking macro to provide arguments for CoCreateInstance()
 * etc, ensuring that 'obj' really is a 'type **'.
 */

#define typecheck(checkexpr, result)	\
	(sizeof(checkexpr) ? (result) : (result))
#define COMPTR(type, obj)		\
	&IID_##type,			\
	typecheck((obj) - (type **)(obj), (void **)(void *)(obj))

/*
 * Private subroutine prototypes
 */

/* [propvarutil.h] */
static HRESULT	InitPropVariantFromString(PCWSTR psz, PROPVARIANT *ppropvar);

/* [shlobj.h] */
STDAPI		SHGetKnownFolderPath(REFKNOWNFOLDERID rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath);

/*
 * Private subroutines
 */

/* propvarutil.h */
static HRESULT
InitPropVariantFromString(
	PCWSTR		psz,
	PROPVARIANT *	ppropvar
	)
{
	HRESULT hres;


	hres = SHStrDupW(psz, &ppropvar->pwszVal);
	if (SUCCEEDED(hres)) {
		ppropvar->vt = VT_LPWSTR;
	} else {
		PropVariantInit(ppropvar);
	}

	return hres;
}

/*
 * Public subroutines
 */

WfrStatus
WfrCreateShortcut(
	const wchar_t *		app_user_model_id,
	const wchar_t *		description,
	const wchar_t *		shortcut_pname,
	const wchar_t *		target_pname,
	const wchar_t *		working_dname
	)
{
	HRESULT			hres;
	IShellLinkW *		psl = NULL;
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
		&CLSID_ShellLinkW, NULL, CLSCTX_INPROC_SERVER,
		COMPTR(IShellLinkW, &psl));
	if (FAILED(hres)) {
		status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
		goto out;
	} else {
		hres = psl->lpVtbl->QueryInterface(psl, COMPTR(IPersistFile, &ppf));
		if (FAILED(hres)) {
			status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
			goto out;
		} else {
			hres = psl->lpVtbl->QueryInterface(psl, COMPTR(IPropertyStore, &pps));
			if (FAILED(hres)) {
				status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
				goto out;
			}
		}
	}

	hres = psl->lpVtbl->SetPath(psl, target_pname_abs);
	if (FAILED(hres)) {
		status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
		goto out;
	} else {
		hres = psl->lpVtbl->SetDescription(psl, description);
		if (FAILED(hres)) {
			status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
			goto out;
		} else {
			hres = psl->lpVtbl->SetWorkingDirectory(psl, working_dname_full);
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
			hres = pps->lpVtbl->SetValue(pps, &PKEY_AppUserModel_ID, &propvar);
			(void)PropVariantClear(&propvar);
			if (FAILED(hres)) {
				status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
				goto out;
			}
		}
	}

	hres = pps->lpVtbl->Commit(pps);
	if (FAILED(hres)) {
		status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
		goto out;
	}
	hres = ppf->lpVtbl->Save(ppf, shortcut_pname_abs, TRUE);
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
	const wchar_t *		app_user_model_id,
	const wchar_t *		description,
	const wchar_t *		shortcut_fname,
	const wchar_t *		target_pname,
	const wchar_t *		working_dname
	)
{
	HRESULT		hres;
	PWSTR		startup_dname_abs = NULL;
	WCHAR 		shortcut_pname[PATH_MAX + 1];
	WfrStatus	status;


	hres = SHGetKnownFolderPath(
		(REFKNOWNFOLDERID)&FOLDERID_Startup, 0, NULL, &startup_dname_abs);
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

WfrStatus
WfrDeleteShortcutStartup(
	const wchar_t *		shortcut_fname
	)
{
	HRESULT		hres;
	PWSTR		startup_dname_abs = NULL;
	WCHAR 		shortcut_pname[PATH_MAX + 1];
	WfrStatus	status;


	hres = SHGetKnownFolderPath(
		(REFKNOWNFOLDERID)&FOLDERID_Startup, 0, NULL, &startup_dname_abs);
	if (FAILED(hres)) {
		status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres);
	} else {
		WFR_SNWPRINTF(
			shortcut_pname, WFR_SIZEOF_WSTRING(shortcut_pname),
			L"%S\\%S", startup_dname_abs, shortcut_fname);
		status = WfrDeleteFileW(NULL, shortcut_pname);
	}

	CoTaskMemFree(startup_dname_abs);

	return status;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=100
 */
