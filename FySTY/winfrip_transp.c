/*
 * winfrip_transp.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "FySTY/winfrip.h"
#include "FySTY/winfrip_priv.h"

/*
 * Private subroutine prototypes
 */

static void winfripp_transp_config_panel_opaque(union control *ctrl, dlgparam *dlg, void *data, int event);
static void winfripp_transp_config_panel_setting(union control *ctrl, dlgparam *dlg, void *data, int event);

/*
 * Private subroutines
 */

static void winfripp_transp_config_panel_opaque(union control *ctrl, dlgparam *dlg, void *data, int event)
{
    Conf *conf = (Conf *)data;

    switch (event) {
    case EVENT_REFRESH:
	dlg_update_start(ctrl, dlg);
	dlg_listbox_clear(ctrl, dlg);
	dlg_listbox_addwithid(ctrl, dlg, "Never", WINFRIPP_TRANSP_OPAQUE_NEVER);
	dlg_listbox_addwithid(ctrl, dlg, "Focus loss", WINFRIPP_TRANSP_OPAQUE_FOCUS_KILL);
	dlg_listbox_addwithid(ctrl, dlg, "Focus", WINFRIPP_TRANSP_OPAQUE_FOCUS_SET);
	dlg_listbox_select(ctrl, dlg, conf_get_int(conf, CONF_frip_transp_opaque_on));
	dlg_update_done(ctrl, dlg);
	break;

    case EVENT_SELCHANGE:
    case EVENT_VALCHANGE:
	conf_set_int(conf, CONF_frip_transp_opaque_on, dlg_listbox_index(ctrl, dlg));
	break;
    }
}

static void winfripp_transp_config_panel_setting(union control *ctrl, dlgparam *dlg, void *data, int event)
{
    Conf *conf = (Conf *)data;

    switch (event) {
    case EVENT_REFRESH:
	dlg_update_start(ctrl, dlg);
	dlg_listbox_clear(ctrl, dlg);
	dlg_listbox_addwithid(ctrl, dlg, "Off", WINFRIPP_TRANSP_SETTING_OFF);
	dlg_listbox_addwithid(ctrl, dlg, "Low", WINFRIPP_TRANSP_SETTING_LOW);
	dlg_listbox_addwithid(ctrl, dlg, "Medium", WINFRIPP_TRANSP_SETTING_MEDIUM);
	dlg_listbox_addwithid(ctrl, dlg, "High", WINFRIPP_TRANSP_SETTING_HIGH);
	dlg_listbox_addwithid(ctrl, dlg, "Custom", WINFRIPP_TRANSP_SETTING_CUSTOM);
	dlg_listbox_select(ctrl, dlg, conf_get_int(conf, CONF_frip_transp_setting));
	dlg_update_done(ctrl, dlg);
	break;

    case EVENT_SELCHANGE:
    case EVENT_VALCHANGE:
	conf_set_int(conf, CONF_frip_transp_setting, dlg_listbox_index(ctrl, dlg));
	break;
    }
}

/*
 * Public subroutines private to FySTY/winfrip*.c
 */

void winfripp_transp_config_panel(struct controlbox *b)
{
    struct controlset *s;


    WINFRIPP_DEBUG_ASSERT(b);

    /*
     * The Frippery: transparency panel.
     */

    ctrl_settitle(b, "Frippery/Transparency", "Configure pointless frippery: transparency");
    s = ctrl_getset(b, "Frippery/Transparency", "frip_transp", "Transparency settings");
    ctrl_droplist(s, "Setting:", 't', 35, P(WINFRIPP_HELP_CTX),
		  winfripp_transp_config_panel_setting, P(NULL));
    ctrl_editbox(s, "Custom (0-255):", 'u', 15, P(WINFRIPP_HELP_CTX),
		 conf_editbox_handler, I(CONF_frip_transp_custom), I(-1));
    ctrl_droplist(s, "Opaque on:", 'q', 35, P(WINFRIPP_HELP_CTX),
		  winfripp_transp_config_panel_opaque, P(NULL));
}

/*
 * Public subroutines
 */

void winfrip_transp_op(WinFripTranspOp op, Conf *conf, HWND hwnd)
{
    LONG_PTR ex_style, rc;
    int opacity;


    switch (conf_get_int(conf, CONF_frip_transp_setting)) {
    default:
	WINFRIPP_DEBUG_FAIL(); return;
    case WINFRIPP_TRANSP_SETTING_OFF:
	opacity = WINFRIPP_TRANSP_LEVEL_OFF; break;
    case WINFRIPP_TRANSP_SETTING_LOW:
	opacity = WINFRIPP_TRANSP_LEVEL_LOW; break;
    case WINFRIPP_TRANSP_SETTING_MEDIUM:
	opacity = WINFRIPP_TRANSP_LEVEL_MEDIUM; break;
    case WINFRIPP_TRANSP_SETTING_HIGH:
	opacity = WINFRIPP_TRANSP_LEVEL_HIGH; break;
    case WINFRIPP_TRANSP_SETTING_CUSTOM:
	opacity = conf_get_int(conf, CONF_frip_transp_custom); break;
    }

    switch (op) {
    default:
	WINFRIPP_DEBUG_FAIL(); return;
    case WINFRIP_TRANSP_OP_FOCUS_KILL:
	switch (conf_get_int(conf, CONF_frip_transp_opaque_on)) {
	default:
	    WINFRIPP_DEBUG_FAIL(); return;
	case WINFRIPP_TRANSP_OPAQUE_FOCUS_KILL:
	    ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED;
	    WINFRIPP_DEBUG_ASSERT(ex_style > 0);
	    opacity = 255;
	    break;
	case WINFRIPP_TRANSP_OPAQUE_NEVER:
	case WINFRIPP_TRANSP_OPAQUE_FOCUS_SET:
	    ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED;
	    WINFRIPP_DEBUG_ASSERT(ex_style > 0);
	    break;
	}
	break;
    case WINFRIP_TRANSP_OP_FOCUS_SET:
	switch (conf_get_int(conf, CONF_frip_transp_opaque_on)) {
	default:
	    WINFRIPP_DEBUG_FAIL(); return;
	case WINFRIPP_TRANSP_OPAQUE_FOCUS_SET:
	    ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED;
	    WINFRIPP_DEBUG_ASSERT(ex_style > 0);
	    opacity = 255;
	    break;
	case WINFRIPP_TRANSP_OPAQUE_NEVER:
	case WINFRIPP_TRANSP_OPAQUE_FOCUS_KILL:
	    ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED;
	    WINFRIPP_DEBUG_ASSERT(ex_style > 0);
	    break;
	}
	break;
    }
    rc = SetWindowLongPtr(hwnd, GWL_EXSTYLE, ex_style);
    WINFRIPP_DEBUG_ASSERT(rc > 0);

    /*
     * Ignore return value as the Windows API is fucking braindead.
     */
    SetLayeredWindowAttributes(hwnd, 0, opacity, LWA_ALPHA);
}
