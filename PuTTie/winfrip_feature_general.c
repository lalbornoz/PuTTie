/*
 * winfrip_feature_general.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "dialog.h"
#include "windows/putty-rc.h"
#include "windows/win-gui-seat.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_debug.h"
#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_feature_general.h"
#include "PuTTie/winfrip_storage_options.h"

/*
 * Preprocessor macros
 */

/*
 * IDM_TRAY_{FIRST,LAST,QUIT,RESTORE}: PuTTie system tray icon menu command identifier constants
 */

#define IDM_TRAY_FIRST		0x6010
#define IDM_TRAY_LAST		0x6020
#define IDM_TRAY_QUIT		0x6020
#define IDM_TRAY_RESTORE	0x6010

/*
 * WM_SYSTRAY: user-defined system tray window message constant
 */

#define WM_SYSTRAY		(WM_USER + (0x7FFF - WM_USER))

/*
 * Private constants
 */

#define WFFGP_OPTION_CONFIG_LAST_POSITION	"PuttyConfigLastPosition"
#define WFFGP_OPTION_CONFIG_LAST_SIZE		"PuttyConfigLastSize"

/*
 * Private variables
 */

/*
 * WffgpNotifyIcon: PuTTie notification area information structrue
 */

static NOTIFYICONDATAA	WffgpNotifyIcon;

/*
 * External subroutine prototypes
 */

/* [see windows/window.c] */
void			start_backend(WinGuiSeat *wgs);

/*
 * Private subroutine prototypes
 */

static void		WffgpSysTrayInit(HINSTANCE hinst, HWND hwnd);
static void		WffgpSysTrayMinimise(Conf *conf, HWND hwnd);
static WfReturn		WffgpSysTrayWmMenu(HWND hwnd, WPARAM wParam);
static void		WffgpSysTrayWmOther(HWND hwnd, LPARAM lParam);

/*
 * Private subroutines
 */

static void
WffgpSysTrayInit(
	HINSTANCE	hinst,
	HWND		hwnd
	)
{
	ZeroMemory(&WffgpNotifyIcon, sizeof(WffgpNotifyIcon));
	WffgpNotifyIcon.cbSize = sizeof(WffgpNotifyIcon);
	WffgpNotifyIcon.hIcon = LoadIcon(hinst, MAKEINTRESOURCE(IDI_MAINICON));
	WffgpNotifyIcon.hWnd = hwnd;
	WffgpNotifyIcon.uCallbackMessage = WM_SYSTRAY;
	WffgpNotifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	WffgpNotifyIcon.uVersion = NOTIFYICON_VERSION;
}

static void
WffgpSysTrayMinimise(
	Conf *	conf,
	HWND	hwnd
	)
{
	if (conf_get_bool(conf, CONF_frip_general_minimise_to_systray)) {
		GetWindowTextA(hwnd, WffgpNotifyIcon.szTip, sizeof(WffgpNotifyIcon.szTip));
		Shell_NotifyIconA(NIM_ADD, &WffgpNotifyIcon);
		ShowWindow(hwnd, SW_HIDE);
	}
}

static WfReturn
WffgpSysTrayWmMenu(
	HWND	hwnd,
	WPARAM	wParam
	)
{
	switch (wParam & ~0xF) {
	case IDM_TRAY_RESTORE:
		ShowWindow(hwnd, SW_RESTORE);
		SetForegroundWindow(hwnd);
		Shell_NotifyIconA(NIM_DELETE, &WffgpNotifyIcon);
		return WF_RETURN_BREAK;

	case IDM_TRAY_QUIT:
		Shell_NotifyIconA(NIM_DELETE, &WffgpNotifyIcon);
		PostQuitMessage(1);
		return WF_RETURN_BREAK;

	default:
		break;
	}

	return WF_RETURN_CONTINUE;
}

static void
WffgpSysTrayWmOther(
	HWND	hwnd,
	LPARAM	lParam
	)
{
	POINT	pt;
	HMENU	systray_menu;


	switch (lParam) {
	case WM_LBUTTONDBLCLK:
		ShowWindow(hwnd, SW_RESTORE);
		SetForegroundWindow(hwnd);
		Shell_NotifyIconA(NIM_DELETE, &WffgpNotifyIcon);
		break;

	case WM_RBUTTONUP:
		systray_menu = CreatePopupMenu();
		AppendMenu(systray_menu, MF_ENABLED, IDM_TRAY_RESTORE, "&Restore");
		AppendMenu(systray_menu, MF_ENABLED, IDM_TRAY_QUIT, "&Quit");

		SetForegroundWindow(hwnd); GetCursorPos(&pt);
		TrackPopupMenu(systray_menu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
		break;

	default:
		break;
	}
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
WffGeneralConfigPanel(
	struct controlbox *	b
	)
{
	struct controlset *	s;


	/*
	 * The Frippery: general panel.
	 */

	ctrl_settitle(b, "Frippery", "Configure pointless frippery: general frippery");
	s = ctrl_getset(b, "Frippery", "frip_general", "General pointless frippery");
	ctrl_checkbox(s, "Always on top", 'l', WFP_HELP_CTX, conf_checkbox_handler, I(CONF_frip_general_always_on_top));
	ctrl_checkbox(s, "Minimise to system tray", 'y', WFP_HELP_CTX, conf_checkbox_handler, I(CONF_frip_general_minimise_to_systray));
	ctrl_checkbox(s, "Cache passwords", 'p', WFP_HELP_CTX, conf_checkbox_handler, I(CONF_frip_cache_passwords));
	ctrl_text(s, "WARNING: If and while enabled, this will cache passwords in memory insecurely. Consider not using this on shared computers.", WFP_HELP_CTX);
	ctrl_checkbox(s, "Automatically restart session on disconnect", 'r', WFP_HELP_CTX, conf_checkbox_handler, I(CONF_frip_general_restart_session));
}

UINT
WffGeneralGetWmSysTray(
	void
	)
{
	return WM_SYSTRAY;
}

/*
 * Public subroutines
 */

WfReturn
WffGeneralOperation(
	WffGeneralOp	op,
	Conf *		conf,
	HINSTANCE	hinst,
	HWND		hwnd,
	LPARAM		lParam,
	int		reconfiguring,
	void *		wgs,
	WPARAM		wParam
	)
{
	WfsBackend	backend;
	char *		last_position = NULL, *last_size = NULL;
	size_t		last_position_size, last_size_size;
	int		last_x, last_y;
	int		last_w, last_h;
	RECT		rect;
	WfrStatus	status;


	switch (op) {
	case WFF_GENERAL_OP_CONFIG_DIALOG:
		if (conf_get_bool(conf, CONF_frip_general_always_on_top)) {
			SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		} else {
			break;
		}
		break;

	case WFF_GENERAL_OP_FOCUS_SET:
		if (reconfiguring) {
			break;
		}
		if (conf_get_bool(conf, CONF_frip_general_always_on_top)) {
			SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		} else {
			SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		break;

	case WFF_GENERAL_OP_SYSTRAY_INIT:
		WffgpSysTrayInit(hinst, hwnd); break;
	case WFF_GENERAL_OP_SYSTRAY_MINIMISE:
		WffgpSysTrayMinimise(conf, hwnd); break;
	case WFF_GENERAL_OP_SYSTRAY_WM_MENU:
		return WffgpSysTrayWmMenu(hwnd, wParam);
	case WFF_GENERAL_OP_SYSTRAY_WM_OTHER:
		WffgpSysTrayWmOther(hwnd, lParam); break;

	case WFF_GENERAL_OP_RESTART_SESSION:
		/* window.c:WndProc():IDM_RESTART */
		lp_eventlog(&((WinGuiSeat *)wgs)->logpolicy, "----- Session restarted -----");
		term_pwron(((WinGuiSeat *)wgs)->term, false);
		start_backend(wgs);
		break;

	case WFF_GENERAL_OP_POSITION_RESTORE:
		backend = WfsGetBackend();
		if (WFR_SUCCESS(status = WfsGetOption(
			backend, WFFGP_OPTION_CONFIG_LAST_POSITION, NULL,
			(void **)&last_position, NULL, NULL)))
		{
			if (sscanf(last_position, "%d,%d", &last_x, &last_y) == 2) {
				SetWindowPos(
					hwnd, NULL, last_x, last_y,
					0, 0, SWP_NOSIZE | SWP_NOZORDER);
			}
		}
		break;

	case WFF_GENERAL_OP_SIZE_RESTORE:
		backend = WfsGetBackend();
		if (WFR_SUCCESS(status = WfsGetOption(
			backend, WFFGP_OPTION_CONFIG_LAST_SIZE, NULL,
			(void **)&last_size, NULL, NULL)))
		{
			if (sscanf(last_size, "%dx%d", &last_w, &last_h) == 2) {
				SetWindowPos(
					hwnd, NULL, 0, 0, last_w, last_h,
					SWP_NOMOVE | SWP_NOZORDER);
			}
		}
		break;

	case WFF_GENERAL_OP_POSITION_SET:
		backend = WfsGetBackend();

		if (WFR_FAILURE_WINDOWS(status,
			(GetWindowRect(hwnd, &rect) > 0))
		||  WFR_FAILURE(status = WfrSnDuprintF(
			&last_position, &last_position_size, "%d,%d",
			rect.left, rect.top))
		||  WFR_FAILURE(status = WfsSetOption(
			backend, false, true,
			WFFGP_OPTION_CONFIG_LAST_POSITION,
			last_position, last_position_size,
			WFR_TREE_ITYPE_STRING)))
		{
			WFR_FREE_IF_NOTNULL(last_position);
		}
		break;

	case WFF_GENERAL_OP_SIZE_SET:
		backend = WfsGetBackend();

		if (WFR_FAILURE_WINDOWS(status,
			(GetWindowRect(hwnd, &rect) > 0))
		||  WFR_FAILURE(status = WfrSnDuprintF(
			&last_size, &last_size_size, "%dx%d",
			rect.right - rect.left, rect.bottom - rect.top))
		||  WFR_FAILURE(status = WfsSetOption(
			backend, false, true,
			WFFGP_OPTION_CONFIG_LAST_SIZE,
			last_size, last_size_size,
			WFR_TREE_ITYPE_STRING)))
		{
			WFR_FREE_IF_NOTNULL(last_size);
		}
		break;

	default:
		WFR_DEBUG_FAIL();
	}

	return WF_RETURN_CONTINUE;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
