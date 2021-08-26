/*
 * winfrip_mouse.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "FySTY/winfrip.h"
#include "FySTY/winfrip_priv.h"

/*
 * Public subroutines private to FySTY/winfrip*.c
 */

void winfripp_mouse_config_panel(struct controlbox *b)
{
    struct controlset *s;


    WINFRIPP_DEBUG_ASSERT(b);

    /*
     * The Frippery: mouse panel.
     */

    ctrl_settitle(b, "Frippery/Mouse", "Configure pointless frippery: mouse behaviour");
    s = ctrl_getset(b, "Frippery/Mouse", "frip", "Mouse behaviour");

    ctrl_radiobuttons(s, "Right mouse button:", NO_SHORTCUT, 2, P(WINFRIPP_HELP_CTX),
		      conf_radiobutton_handler, I(CONF_frip_mouse_rmb),
		      "Normal",		NO_SHORTCUT,	I(WINFRIPP_MOUSE_RMB_NORMAL),
		      "Inhibit",	NO_SHORTCUT,	I(WINFRIPP_MOUSE_RMB_INHIBIT), NULL);
    ctrl_text(s, "This only affects click actions with no modifiers, e.g. CTRL, ALT, and/or SHIFT.",
	      P(WINFRIPP_HELP_CTX));

    ctrl_radiobuttons(s, "Mouse wheel:", NO_SHORTCUT, 2, P(WINFRIPP_HELP_CTX),
		      conf_radiobutton_handler, I(CONF_frip_mouse_wheel),
		      "Normal",			NO_SHORTCUT,	I(WINFRIPP_MOUSE_WHEEL_NORMAL),
		      "Change font size",	NO_SHORTCUT,	I(WINFRIPP_MOUSE_WHEEL_FONT_SIZE), NULL);
    ctrl_text(s, "This only affects mouse wheel actions with the CTRL modifier.",
	      P(WINFRIPP_HELP_CTX));
}

/*
 * Public subroutines
 */

WinFripReturn winfrip_mouse_op(WinFripMouseOp op, Conf *conf, UINT message, WPARAM wParam)
{
    FontSpec *font;
    BYTE keystate[256];
    short wheel_distance;
    int rc;


    if (op == WINFRIP_MOUSE_OP_MOUSE_EVENT) {
	if (message == WM_RBUTTONDOWN) {
	    op = WINFRIP_MOUSE_OP_RMB_DOWN;
	} else if (message == WM_MOUSEWHEEL) {
	    op = WINFRIP_MOUSE_OP_WHEEL;
	} else {
	    return WINFRIP_RETURN_CONTINUE;
	}
    }

    switch (op) {
    default:
	WINFRIPP_DEBUG_FAIL();
	return WINFRIP_RETURN_FAILURE;

    case WINFRIP_MOUSE_OP_RMB_DOWN:
	switch (conf_get_int(conf, CONF_frip_mouse_rmb)) {
	default:
	    WINFRIPP_DEBUG_FAIL();
	    return WINFRIP_RETURN_FAILURE;
	case WINFRIPP_MOUSE_RMB_NORMAL:
	    return WINFRIP_RETURN_CONTINUE;
	case WINFRIPP_MOUSE_RMB_INHIBIT:
	    if (wParam & (MK_CONTROL | MK_SHIFT)) {
		return WINFRIP_RETURN_CONTINUE;
	    } else {
		rc = GetKeyboardState(keystate);
		if (!rc) {
		    WINFRIPP_DEBUG_FAIL();
		    return WINFRIP_RETURN_FAILURE;
		} else if ((keystate[VK_MENU] & 0x80) || (keystate[VK_RMENU] & 0x80)) {
		    return WINFRIP_RETURN_CONTINUE;
		} else {
		    return WINFRIP_RETURN_BREAK;
		}
	    }
	}

    case WINFRIP_MOUSE_OP_WHEEL:
	if ((LOWORD(wParam) & MK_CONTROL) && !(LOWORD(wParam) & MK_SHIFT)) {
	    switch (conf_get_int(conf, CONF_frip_mouse_wheel)) {
	    default:
		WINFRIPP_DEBUG_FAIL();
		return WINFRIP_RETURN_FAILURE;
	    case WINFRIPP_MOUSE_WHEEL_NORMAL:
		return WINFRIP_RETURN_CONTINUE;
	    case WINFRIPP_MOUSE_WHEEL_FONT_SIZE:
		font = conf_get_fontspec(conf, CONF_font);
		wheel_distance = (short)HIWORD(wParam);
		if ((wheel_distance > 0) && (font->height < 32)) {
		    font->height++;
		    return WINFRIP_RETURN_BREAK_RESET_WINDOW;
		} else if ((wheel_distance < 0) && (font->height > 1)) {
		    font->height--;
		    return WINFRIP_RETURN_BREAK_RESET_WINDOW;
		} else {
		    WINFRIPP_DEBUG_FAIL();
		    return WINFRIP_RETURN_FAILURE;
		}
	    }
	} else {
	    return WINFRIP_RETURN_FAILURE;
	}
    }
}
