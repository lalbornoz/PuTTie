/*
 * winfrip_mouse.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
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
    s = ctrl_getset(b, "Frippery/Mouse", "frip_mouse", "Mouse behaviour");

    ctrl_checkbox(s, "Change font size with mouse wheel:", 'n', P(WINFRIPP_HELP_CTX),
		  conf_checkbox_handler, I(CONF_frip_mouse_font_size_wheel));
    ctrl_text(s, "This only affects mouse wheel actions with the CTRL modifier.", P(WINFRIPP_HELP_CTX));
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
	if (message == WM_MOUSEWHEEL) {
	    op = WINFRIP_MOUSE_OP_WHEEL;
	} else {
	    return WINFRIP_RETURN_CONTINUE;
	}
    }

    switch (op) {
    default:
	WINFRIPP_DEBUG_FAIL();
	return WINFRIP_RETURN_FAILURE;

    case WINFRIP_MOUSE_OP_WHEEL:
	if ((LOWORD(wParam) & MK_CONTROL) && !(LOWORD(wParam) & MK_SHIFT)) {
	    if (conf_get_bool(conf, CONF_frip_mouse_font_size_wheel)) {
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
	    } else {
		return WINFRIP_RETURN_CONTINUE;
	    }
	} else {
	    return WINFRIP_RETURN_FAILURE;
	}
	break;
    }
}
