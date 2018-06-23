/*
 * winfrip_transp.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "winfrip.h"
#include "winfrip_priv.h"

#include <assert.h>

/*
 * {External,Static} variables
 */

/*
 * XXX document
 */
extern Conf *conf;

/*
 * Public subroutines private to FySTY/winfrip*.c
 */

void winfripp_transp_config_panel(struct controlbox *b)
{
    struct controlset *s;


    /*
     * The Window/Frippery: transparency panel.
     */

    ctrl_settitle(b, "Window/Frippery: transparency", "Configure pointless frippery: transparency");
    s = ctrl_getset(b, "Window/Frippery: transparency", "frip_transp", "Transparency settings");
    ctrl_radiobuttons(s, "Setting", NO_SHORTCUT, 3, HELPCTX(appearance_frippery),
		      conf_radiobutton_handler, I(CONF_frip_transp_setting),
		      "Off",		NO_SHORTCUT,	I(WINFRIP_TRANSP_SETTING_OFF),
		      "Low",		NO_SHORTCUT,	I(WINFRIP_TRANSP_SETTING_LOW),
		      "Medium",		NO_SHORTCUT,	I(WINFRIP_TRANSP_SETTING_MEDIUM),
		      "High",		NO_SHORTCUT,	I(WINFRIP_TRANSP_SETTING_HIGH),
		      "Custom",		NO_SHORTCUT,	I(WINFRIP_TRANSP_SETTING_CUSTOM), NULL);
    ctrl_editbox(s, "Custom (0-255):", NO_SHORTCUT, 20, HELPCTX(appearance_frippery),
		 conf_editbox_handler, I(CONF_frip_transp_custom), I(-1));
    ctrl_radiobuttons(s, "Opaque on", NO_SHORTCUT, 3, HELPCTX(appearance_frippery),
		      conf_radiobutton_handler, I(CONF_frip_transp_opaque_on),
		      "Never",		NO_SHORTCUT,	I(WINFRIP_TRANSP_OPAQUE_NEVER),
		      "Focus loss",	NO_SHORTCUT,	I(WINFRIP_TRANSP_OPAQUE_FOCUS_KILL),
		      "Focus",		NO_SHORTCUT,	I(WINFRIP_TRANSP_OPAQUE_FOCUS_SET), NULL);
}

/*
 * Public subroutines
 */

void winfrip_transp_op(WinFripTranspOp op, HWND hwnd)
{
    LONG_PTR ex_style;
    int opacity;


    /*
     * XXX document
     */
    switch (conf_get_int(conf, CONF_frip_transp_setting)) {
    case WINFRIP_TRANSP_SETTING_OFF:
	opacity = WINFRIP_TRANSP_LEVEL_OFF; break;
    case WINFRIP_TRANSP_SETTING_LOW:
	opacity = WINFRIP_TRANSP_LEVEL_LOW; break;
    case WINFRIP_TRANSP_SETTING_MEDIUM:
	opacity = WINFRIP_TRANSP_LEVEL_MEDIUM; break;
    case WINFRIP_TRANSP_SETTING_HIGH:
	opacity = WINFRIP_TRANSP_LEVEL_HIGH; break;
    case WINFRIP_TRANSP_SETTING_CUSTOM:
	opacity = conf_get_int(conf, CONF_frip_transp_custom); break;
    default:
	WINFRIPP_DEBUG_FAIL();
	return;
    }

    /*
     * XXX document
     */
    switch (op) {
    case WINFRIP_TRANSP_OP_FOCUS_KILL:
	switch (conf_get_int(conf, CONF_frip_transp_opaque_on)) {
	case WINFRIP_TRANSP_OPAQUE_FOCUS_KILL:
	    ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED;
	    opacity = 255;
	    break;
	case WINFRIP_TRANSP_OPAQUE_NEVER:
	case WINFRIP_TRANSP_OPAQUE_FOCUS_SET:
	    ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED;
	    break;
	}
	break;
    case WINFRIP_TRANSP_OP_FOCUS_SET:
	switch (conf_get_int(conf, CONF_frip_transp_opaque_on)) {
	case WINFRIP_TRANSP_OPAQUE_FOCUS_SET:
	    ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED;
	    opacity = 255;
	    break;
	case WINFRIP_TRANSP_OPAQUE_NEVER:
	case WINFRIP_TRANSP_OPAQUE_FOCUS_KILL:
	    ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED;
	    break;
	}
	break;
    }
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, ex_style);
    SetLayeredWindowAttributes(hwnd, 0, opacity, LWA_ALPHA);
}
