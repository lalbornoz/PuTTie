/*
 * winfrip_transp.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "FySTY/winfrip.h"
#include "FySTY/winfrip_priv.h"

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
    ctrl_radiobuttons(s, "Setting", NO_SHORTCUT, 3, P(WINFRIPP_HELP_CTX),
		      conf_radiobutton_handler, I(CONF_frip_transp_setting),
		      "Off",		NO_SHORTCUT,	I(WINFRIPP_TRANSP_SETTING_OFF),
		      "Low",		NO_SHORTCUT,	I(WINFRIPP_TRANSP_SETTING_LOW),
		      "Medium",		NO_SHORTCUT,	I(WINFRIPP_TRANSP_SETTING_MEDIUM),
		      "High",		NO_SHORTCUT,	I(WINFRIPP_TRANSP_SETTING_HIGH),
		      "Custom",		NO_SHORTCUT,	I(WINFRIPP_TRANSP_SETTING_CUSTOM), NULL);
    ctrl_editbox(s, "Custom (0-255):", NO_SHORTCUT, 20, P(WINFRIPP_HELP_CTX),
		 conf_editbox_handler, I(CONF_frip_transp_custom), I(-1));
    ctrl_radiobuttons(s, "Opaque on", NO_SHORTCUT, 3, P(WINFRIPP_HELP_CTX),
		      conf_radiobutton_handler, I(CONF_frip_transp_opaque_on),
		      "Never",		NO_SHORTCUT,	I(WINFRIPP_TRANSP_OPAQUE_NEVER),
		      "Focus loss",	NO_SHORTCUT,	I(WINFRIPP_TRANSP_OPAQUE_FOCUS_KILL),
		      "Focus",		NO_SHORTCUT,	I(WINFRIPP_TRANSP_OPAQUE_FOCUS_SET), NULL);
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
