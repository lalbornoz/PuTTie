/*
 * winfrip_general.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "FySTY/winfrip.h"
#include "FySTY/winfrip_priv.h"

/*
 * Private subroutine prototypes
 */

static void winfripp_general_config_panel_store_backend_handler(union control *ctrl, dlgparam *dlg, void *data, int event);

/*
 * Private subroutines
 */

static void winfripp_general_config_panel_store_backend_handler(union control *ctrl, dlgparam *dlg, void *data, int event)
{
    Conf *conf = (Conf *)data;

    switch (event) {
    case EVENT_REFRESH:
	dlg_update_start(ctrl, dlg);
	dlg_listbox_clear(ctrl, dlg);
	dlg_listbox_addwithid(ctrl, dlg, "Registry", WINFRIP_GENERAL_STORE_BACKEND_REGISTRY);
	dlg_listbox_addwithid(ctrl, dlg, "Ephemeral", WINFRIP_GENERAL_STORE_BACKEND_EPHEMERAL);
	dlg_listbox_addwithid(ctrl, dlg, "File", WINFRIP_GENERAL_STORE_BACKEND_FILE);
	dlg_listbox_select(ctrl, dlg, conf_get_int(conf, CONF_frip_general_store_backend));
	dlg_update_done(ctrl, dlg);
	break;

    case EVENT_SELCHANGE:
    case EVENT_VALCHANGE:
	conf_set_int(conf, CONF_frip_general_store_backend, dlg_listbox_index(ctrl, dlg));
	break;
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
    ctrl_droplist(s, "Storage backend:", 's', 35, P(WINFRIPP_HELP_CTX),
		  winfripp_general_config_panel_store_backend_handler, P(NULL));
}

/*
 * Public subroutines
 */

void winfrip_general_op(WinFripGeneralOp op, Conf *conf, HWND hwnd, int reconfiguring)
{
    switch (op) {
    default:
	WINFRIPP_DEBUG_FAIL(); break;

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
    }
}
