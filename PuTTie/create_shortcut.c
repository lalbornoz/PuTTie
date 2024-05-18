/*
 * create_shortcut.c - pointless frippery & tremendous amounts of bloat
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

#include "winfrip_rtl.h"
#include "winfrip_rtl_file.h"
#include "winfrip_rtl_shortcut.h"
#include "winfrip_rtl_windows.h"

/*
 * Private constants
 */

// [see windows/putty.c]
#define PUTTIE_APP_USER_MODEL_ID	L"Roarie.PuTTie"
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
	LPWSTR			lpCmdLine_full;
	LPWSTR			puttie_path = NULL;
	LPWSTR			shortcut_path = NULL;
	WfrStatus		status;


	(void)hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nShowCmd;

	if (!(lpCmdLine_full = GetCommandLine())) {
		status = WFR_STATUS_FROM_WINDOWS();
	} else if (!(argv = CommandLineToArgvW(lpCmdLine_full, &argc))) {
		status = WFR_STATUS_FROM_WINDOWS();
	} else if ((argc != 1) && (argc != 2) && (argc != 3)) {
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
		(void)WfrMessageBoxF(
			NULL, "PuTTie", MB_OK | MB_ICONEXCLAMATION,
			"Invalid arguments, should be: [<shortcut file path>=%S] [<putty.exe file path>=%S]",
			PUTTIE_SHORTCUT_PATH_DEFAULT, PUTTIE_PATH_DEFAULT);
	} else if (((argc >= 2) && !(shortcut_path = wcsdup(argv[1])))
		|| (!shortcut_path && (!(shortcut_path = wcsdup(PUTTIE_SHORTCUT_PATH_DEFAULT))))
		|| ((argc == 3) && (!(puttie_path = wcsdup(argv[2]))))
		|| (!puttie_path && (!(puttie_path = wcsdup(PUTTIE_PATH_DEFAULT)))))
	{
		status = WFR_STATUS_FROM_ERRNO();
	} else {
		status = WfrCreateShortcut(
			PUTTIE_APP_USER_MODEL_ID, PUTTIE_SHORTCUT_DESCRIPTION,
			shortcut_path, puttie_path, NULL);

		if (WFR_SUCCESS(status)) {
			(void)WfrMessageBoxF(
				NULL, "PuTTie", MB_ICONINFORMATION | MB_OK | MB_DEFBUTTON1,
				"Successfully created shortcut to %S as %S",
				puttie_path, shortcut_path);
		} else {
			(void)WfrMessageBoxF(
				NULL, "PuTTie", MB_ICONERROR | MB_OK | MB_DEFBUTTON1,
				"%s", WfrStatusToErrorMessageW((status)));
		}

		WFR_FREE_IF_NOTNULL(puttie_path);
		WFR_FREE_IF_NOTNULL(shortcut_path);
	}

	return WFR_SUCCESS(status) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=100
 */
