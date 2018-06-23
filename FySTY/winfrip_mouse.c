/*
 * winfrip_mouse.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
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
