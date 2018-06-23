/*
 * winfrip_mouse.c - pointless frippery & tremendous amounts of bloat
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
 * Public subroutine private to FySTY/winfrip*.c prototypes
 */

void winfripp_mouse_config_panel(struct controlbox *b)
{
    struct controlset *s;


    /*
     * The Window/Frippery: mouse panel.
     */

    ctrl_settitle(b, "Window/Frippery: mouse", "Configure pointless frippery: mouse");
    s = ctrl_getset(b, "Window/Frippery: mouse", "frip", "Click actions");
    ctrl_radiobuttons(s, "Right mouse button:", NO_SHORTCUT, 2, HELPCTX(appearance_frippery),
		      conf_radiobutton_handler, I(CONF_frip_mouse_rmb),
		      "Normal",		NO_SHORTCUT,	I(WINFRIP_MOUSE_RMB_NORMAL),
		      "Inhibit",	NO_SHORTCUT,	I(WINFRIP_MOUSE_RMB_INHIBIT), NULL);
    ctrl_text(s, "This only affects click actions with no modifiers, e.g. CTRL, ALT, and/or SHIFT.",
	      HELPCTX(appearance_frippery));
}

/*
 * Public subroutines
 */

BOOL winfrip_mouse_op(WinFripMouseOp op, UINT message, WPARAM wParam)
{
    BYTE keystate[256];
    int rc;


    /*
     * XXX document
     */
    if (op == WINFRIP_MOUSE_OP_MOUSE_EVENT) {
	if (message == WM_RBUTTONDOWN) {
	    op = WINFRIP_MOUSE_OP_RMB_DOWN;
	} else {
	    return FALSE;
	}
    }

    /*
     * XXX document
     */
    switch (op) {
    default:
	break;

    case WINFRIP_MOUSE_OP_RMB_DOWN:
	switch (conf_get_int(conf, CONF_frip_mouse_rmb)) {
	case WINFRIP_MOUSE_RMB_NORMAL:
	    return FALSE;
	case WINFRIP_MOUSE_RMB_INHIBIT:
	    if (wParam & (MK_CONTROL | MK_SHIFT)) {
		return FALSE;
	    } else {
		rc = GetKeyboardState(keystate);
		if (!rc) {
		    WINFRIPP_DEBUG_FAIL();
		    return FALSE;
		} else if ((keystate[VK_MENU] & 0x80) || (keystate[VK_RMENU] & 0x80)) {
		    return FALSE;
		} else {
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}
