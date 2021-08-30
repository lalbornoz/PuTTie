/*
 * winfrip_general.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "windows/putty-rc.h"
#include "FySTY/winfrip.h"
#include "FySTY/winfrip_priv.h"

/*
 * Preprocessor macros
 */

/*
 * XXX
 */
#define IDM_TRAY_FIRST		0x6010
#define IDM_TRAY_LAST		0x6020
#define IDM_TRAY_QUIT		0x6020
#define IDM_TRAY_RESTORE	0x6010

/*
 * XXX
 */
#define WM_SYSTRAY		(WM_USER + (0x7FFF - WM_USER))

/*
 * Static variables
 */

/*
 * XXX
 */
static NOTIFYICONDATAA winfripp_general_notifyicon;

/*
 * Private subroutine prototypes
 */

static void winfripp_general_config_panel_store_backend_handler(union control *ctrl, dlgparam *dlg, void *data, int event);
static void winfripp_general_systray_init(HINSTANCE hinst, HWND hwnd);
static void winfripp_general_systray_minimise(Conf *conf, HWND hwnd);
static WinFripReturn winfripp_general_systray_wm_menu(HWND hwnd, WPARAM wParam);
static void winfripp_general_systray_wm_other(HWND hwnd, LPARAM lParam);

/*
 * Private subroutines
 */

static void winfripp_general_config_panel_store_backend_handler(union control *ctrl, dlgparam *dlg, void *data, int event)
{
    Conf *conf = (Conf *)data;
    int id;


    switch (event) {
    case EVENT_REFRESH:
	dlg_update_start(ctrl, dlg);
	dlg_listbox_clear(ctrl, dlg);
	dlg_listbox_addwithid(ctrl, dlg, "Registry", WINFRIP_GENERAL_STORE_BACKEND_REGISTRY);
	dlg_listbox_addwithid(ctrl, dlg, "Ephemeral", WINFRIP_GENERAL_STORE_BACKEND_EPHEMERAL);
	dlg_listbox_addwithid(ctrl, dlg, "File", WINFRIP_GENERAL_STORE_BACKEND_FILE);

	switch (winfrip_confstore_backend_get()) {
	case WINFRIP_GENERAL_STORE_BACKEND_REGISTRY:
	    dlg_listbox_select(ctrl, dlg, 0); break;
	case WINFRIP_GENERAL_STORE_BACKEND_EPHEMERAL:
	    dlg_listbox_select(ctrl, dlg, 1); break;
	case WINFRIP_GENERAL_STORE_BACKEND_FILE:
	    dlg_listbox_select(ctrl, dlg, 2); break;
	default:
	    WINFRIPP_DEBUG_FAIL(); break;
	}
	dlg_update_done(ctrl, dlg);
	break;

    case EVENT_SELCHANGE:
    case EVENT_VALCHANGE:
	id = dlg_listbox_getid(ctrl, dlg, dlg_listbox_index(ctrl, dlg));
	winfrip_confstore_backend_set(id);
	break;
    }
}

static void winfripp_general_systray_init(HINSTANCE hinst, HWND hwnd)
{
    ZeroMemory(&winfripp_general_notifyicon, sizeof(winfripp_general_notifyicon));
    winfripp_general_notifyicon.cbSize = sizeof(winfripp_general_notifyicon);
    winfripp_general_notifyicon.hIcon = LoadIcon(hinst, MAKEINTRESOURCE(IDI_MAINICON));
    winfripp_general_notifyicon.hWnd = hwnd;
    winfripp_general_notifyicon.uCallbackMessage = WM_SYSTRAY;
    winfripp_general_notifyicon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    winfripp_general_notifyicon.uVersion = NOTIFYICON_VERSION;
}

static void winfripp_general_systray_minimise(Conf *conf, HWND hwnd)
{
    if (conf_get_bool(conf, CONF_frip_general_minimise_to_systray)) {
	GetWindowTextA(hwnd, winfripp_general_notifyicon.szTip, sizeof(winfripp_general_notifyicon.szTip));
	Shell_NotifyIconA(NIM_ADD, &winfripp_general_notifyicon);
	ShowWindow(hwnd, SW_HIDE);
    }
}

static WinFripReturn winfripp_general_systray_wm_menu(HWND hwnd, WPARAM wParam)
{
    switch (wParam & ~0xF) {
    case IDM_TRAY_RESTORE:
	ShowWindow(hwnd, SW_RESTORE);
	SetForegroundWindow(hwnd);
	Shell_NotifyIconA(NIM_DELETE, &winfripp_general_notifyicon);
	return WINFRIP_RETURN_BREAK;

    case IDM_TRAY_QUIT:
	Shell_NotifyIconA(NIM_DELETE, &winfripp_general_notifyicon);
	PostQuitMessage(1);
	return WINFRIP_RETURN_BREAK;

    default:
    }

    return WINFRIP_RETURN_CONTINUE;
}

static void winfripp_general_systray_wm_other(HWND hwnd, LPARAM lParam)
{
    HMENU systray_menu;
    POINT pt;


    switch (lParam) {
    case WM_LBUTTONDBLCLK:
	ShowWindow(hwnd, SW_RESTORE);
	SetForegroundWindow(hwnd);
	Shell_NotifyIconA(NIM_DELETE, &winfripp_general_notifyicon);
	break;

    case WM_RBUTTONUP:
	systray_menu = CreatePopupMenu();
	AppendMenu(systray_menu, MF_ENABLED, IDM_TRAY_RESTORE, "&Restore");
	AppendMenu(systray_menu, MF_ENABLED, IDM_TRAY_QUIT, "&Quit");

	SetForegroundWindow(hwnd); GetCursorPos(&pt);
	TrackPopupMenu(systray_menu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
	break;

    default:
    }
}

/*
 * Public subroutines private to FySTY/winfrip*.c
 */

void winfripp_general_config_panel(struct controlbox *b)
{
    struct controlset *s;

    WINFRIPP_DEBUG_ASSERT(b);

    /*
     * The Frippery: general panel.
     */

    ctrl_settitle(b, "Frippery", "Configure pointless frippery: general frippery");
    s = ctrl_getset(b, "Frippery", "frip_general", "General pointless frippery");
    ctrl_checkbox(s, "Always on top", 'l', P(WINFRIPP_HELP_CTX),
		  conf_checkbox_handler, I(CONF_frip_general_always_on_top));
    ctrl_checkbox(s, "Minimise to system tray", 'y', P(WINFRIPP_HELP_CTX),
		  conf_checkbox_handler, I(CONF_frip_general_minimise_to_systray));
    ctrl_droplist(s, "Storage backend:", 's', 35, P(WINFRIPP_HELP_CTX),
		  winfripp_general_config_panel_store_backend_handler, P(NULL));
}

UINT winfripp_general_get_wm_systray(void)
{
    return WM_SYSTRAY;
}

/*
 * Public subroutines
 */

WinFripReturn winfrip_general_op(WinFripGeneralOp op, Conf *conf, HINSTANCE hinst, HWND hwnd, LPARAM lParam, int reconfiguring, WPARAM wParam)
{
    switch (op) {
    case WINFRIP_GENERAL_OP_CONFIG_DIALOG:
	if (conf_get_bool(conf, CONF_frip_general_always_on_top)) {
	    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	} else {
	    break;
	}
	break;

    case WINFRIP_GENERAL_OP_FOCUS_SET:
	if (reconfiguring) {
	    break;
	}
	if (conf_get_bool(conf, CONF_frip_general_always_on_top)) {
	    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	} else {
	    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	break;

    case WINFRIP_GENERAL_OP_SYSTRAY_INIT:
	winfripp_general_systray_init(hinst, hwnd); break;
    case WINFRIP_GENERAL_OP_SYSTRAY_MINIMISE:
	winfripp_general_systray_minimise(conf, hwnd); break;
    case WINFRIP_GENERAL_OP_SYSTRAY_WM_MENU:
	return winfripp_general_systray_wm_menu(hwnd, wParam);
    case WINFRIP_GENERAL_OP_SYSTRAY_WM_OTHER:
	winfripp_general_systray_wm_other(hwnd, lParam); break;

    default:
	WINFRIPP_DEBUG_FAIL();
    }

    return WINFRIP_RETURN_CONTINUE;
}
