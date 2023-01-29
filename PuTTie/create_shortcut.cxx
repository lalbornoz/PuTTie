/*
 * create_shortcut.cxx - pointless frippery & tremendous amounts of bloat
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
#include <propkey.h>
#include <propvarutil.h>

#include "winfrip_rtl.h"
#include "winfrip_rtl_file.h"

/*
 * Private constants
 */

// [see windows/putty.c]
#define PUTTIE_APP_USER_MODEL_ID	L"SimonTatham.PuTTY"
#define PUTTIE_PATH_DEFAULT		L"putty.exe"
#define PUTTIE_SHORTCUT_DESCRIPTION	L"SSH, Telnet and Rlogin client"
#define PUTTIE_SHORTCUT_PATH_DEFAULT	L"PuTTie.lnk"

/*
 * Public subroutines
 */

int
WinMain(
	HINSTANCE	hInstance,
	HINSTANCE	hPrevInstance,
	LPSTR		lpCmdLine,
	int		nShowCmd
	)
{
	int			argc;
	LPWSTR *		argv;
	HRESULT			hres;
	LPWSTR			lpCmdLine_;
	IShellLink *		psl = NULL;
	IPersistFile *		ppf = NULL;
	IPropertyStore *	pps = NULL;
	PROPVARIANT		propvar;
	LPWSTR			puttie_dname = NULL;
	LPWSTR			puttie_path = NULL;
	LPWSTR			shortcut_path = NULL;
	WfrStatus		status;


	(void)hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nShowCmd;

	if (!(lpCmdLine_ = GetCommandLine())) {
		status = WFR_STATUS_FROM_WINDOWS(); goto err;
	} else if (!(argv = CommandLineToArgvW(lpCmdLine_, &argc))) {
		status = WFR_STATUS_FROM_WINDOWS(); goto err;
	} else if ((argc != 1) && (argc != 2) && (argc != 3)) {
		(void)WfrMessageBoxFW(
			L"PuTTie", MB_OK | MB_ICONEXCLAMATION,
			L"Invalid arguments, should be: [<shortcut file path>=%S] [<putty.exe file path>=%S]",
			PUTTIE_SHORTCUT_PATH_DEFAULT, PUTTIE_PATH_DEFAULT);
		return EXIT_FAILURE;
	} else {
		if ((argc >= 2) && !(shortcut_path = wcsdup(argv[1]))) {
			status = WFR_STATUS_FROM_ERRNO(); goto err;
		} else if (!shortcut_path && (!(shortcut_path = wcsdup(PUTTIE_SHORTCUT_PATH_DEFAULT)))) {
			status = WFR_STATUS_FROM_ERRNO(); goto err;
		}

		if ((argc == 3) && (!(puttie_path = wcsdup(argv[2])))) {
			status = WFR_STATUS_FROM_ERRNO(); goto err;
		} else if (!puttie_path && (!(puttie_path = wcsdup(PUTTIE_PATH_DEFAULT)))) {
			status = WFR_STATUS_FROM_ERRNO(); goto err;
		}

		if (WFR_STATUS_FAILURE(status = WfrPathNameToAbsoluteW(&shortcut_path))
		||  WFR_STATUS_FAILURE(status = WfrPathNameToAbsoluteW(&puttie_path)))
		{
			goto err;
		}
	}

	hres = CoInitialize(NULL);
	if ((hres != S_OK) && (hres != S_FALSE)) {
		status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres); goto err;
	}

	hres = CoCreateInstance(
		CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
		IID_IShellLink, (LPVOID *)&psl);
	if (FAILED(hres)) {
		status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres); goto err;
	} else {
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
		if (FAILED(hres)) {
			status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres); goto err;
		} else {
			hres = psl->QueryInterface(IID_IPropertyStore, (LPVOID *)&pps);
			if (FAILED(hres)) {
				status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres); goto err;
			}
		}
	}

	hres = psl->SetPath(puttie_path);
	if (FAILED(hres)) {
		status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres); goto err;
	} else {
		hres = psl->SetDescription(PUTTIE_SHORTCUT_DESCRIPTION);
		if (FAILED(hres)) {
			status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres); goto err;
		} else {
			if (WFR_STATUS_FAILURE(status = WfrPathNameToDirectoryW(
					puttie_path, &puttie_dname)))
			{
				goto err;
			} else {
				hres = psl->SetWorkingDirectory(puttie_dname);
				if (FAILED(hres)) {
					status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres); goto err;
				}
			}
		}
	}

	hres = InitPropVariantFromString(PUTTIE_APP_USER_MODEL_ID, &propvar);
	if (FAILED(hres)) {
		status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres); goto err;
	} else {
		hres = pps->SetValue(PKEY_AppUserModel_ID, propvar);
		(void)PropVariantClear(&propvar);
		if (FAILED(hres)) {
			status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres); goto err;
		} else {
			hres = pps->Commit();
			if (FAILED(hres)) {
				status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres); goto err;
			}
		}
	}

	hres = ppf->Save(shortcut_path, TRUE);
	if (FAILED(hres)) {
		status = WFR_STATUS_FROM_WINDOWS_HRESULT(hres); goto err;
	} else {
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

out:
	WFR_RELEASE_IF_NOTNULL(pps);
	WFR_RELEASE_IF_NOTNULL(ppf);
	WFR_RELEASE_IF_NOTNULL(psl);
	WFR_FREE_IF_NOTNULL(puttie_dname);

	if (WFR_STATUS_SUCCESS(status)) {
		(void)WfrMessageBoxFW(
			L"PuTTie", MB_ICONINFORMATION | MB_OK | MB_DEFBUTTON1,
			L"Successfully created shortcut to %S as %S",
			puttie_path, shortcut_path);
		WFR_FREE_IF_NOTNULL(puttie_path);
		WFR_FREE_IF_NOTNULL(shortcut_path);
		return EXIT_SUCCESS;
	} else {
		WFR_FREE_IF_NOTNULL(puttie_path);
		WFR_FREE_IF_NOTNULL(shortcut_path);
		return EXIT_FAILURE;
	}

err:
	(void)WfrMessageBoxFW(
		L"PuTTie", MB_ICONERROR | MB_OK | MB_DEFBUTTON1,
		L"%s", WfrStatusToErrorMessageW((status)));
	goto out;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=100
 */
