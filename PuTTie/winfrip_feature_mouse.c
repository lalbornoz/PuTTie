/*
 * winfrip_feature_mouse.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "dialog.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_feature_mouse.h"
#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_debug.h"

/*
 * Private variables
 */

static bool WffpMouseControlState = false;

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
WffMouseConfigPanel(
	struct controlbox *	b
	)
{
	struct controlset *	s;


	/*
	 * The Frippery: mouse panel.
	 */

	ctrl_settitle(b, "Frippery/Mouse", "Configure pointless frippery: mouse behaviour");
	s = ctrl_getset(b, "Frippery/Mouse", "frip_mouse", "Mouse behaviour");

	ctrl_checkbox(s, "Change font size with mouse wheel", 'n', WFP_HELP_CTX, conf_checkbox_handler, I(CONF_frip_mouse_font_size_wheel));
	ctrl_text(s, "This only affects mouse wheel actions with the CTRL modifier.", WFP_HELP_CTX);

	ctrl_checkbox(s, "Change font size with CTRL 0/+/-:", 'k', WFP_HELP_CTX, conf_checkbox_handler, I(CONF_frip_mouse_font_size_wheel_shortcut));
}

/*
 * Public subroutines
 */

WfReturn
WffMouseOperation(
	WffMouseOp	op,
	Conf *		conf,
	UINT		message,
	WPARAM		wParam
	)
{
	FontSpec *	font;
	short		wheel_distance;


	if (op == WFF_MOUSE_OP_MOUSE_EVENT) {
		if (message == WM_MOUSEWHEEL) {
			op = WFF_MOUSE_OP_WHEEL;
		} else {
			return WF_RETURN_CONTINUE;
		}
	}

	switch (op) {
	default:
		WFR_DEBUG_FAIL();
		return WF_RETURN_FAILURE;

	case WFF_MOUSE_OP_WHEEL:
		if ((LOWORD(wParam) & MK_CONTROL) && !(LOWORD(wParam) & MK_SHIFT)) {
			if (conf_get_bool(conf, CONF_frip_mouse_font_size_wheel)) {
				font = conf_get_fontspec(conf, CONF_font);
				wheel_distance = (short)HIWORD(wParam);
				if ((wheel_distance > 0) && (font->height < 32)) {
					font->height++;
					return WF_RETURN_BREAK_RESET_WINDOW;
				} else if ((wheel_distance < 0) && (font->height > 1)) {
					font->height--;
					return WF_RETURN_BREAK_RESET_WINDOW;
				} else {
					WFR_DEBUG_FAIL();
					return WF_RETURN_FAILURE;
				}
			} else {
				return WF_RETURN_CONTINUE;
			}
		} else {
			return WF_RETURN_FAILURE;
		}
		break;

	case WFF_MOUSE_OP_KEY_MESSAGE:
		if (WffpMouseControlState) {
			if (wParam == '0') {
				if (conf_get_bool(conf, CONF_frip_mouse_font_size_wheel_shortcut)) {
					font = conf_get_fontspec(conf, CONF_font);
					font->height = 10;
					return WF_RETURN_BREAK_RESET_WINDOW;
				} else {
					return WF_RETURN_CONTINUE;
				}
			} else if (wParam == VK_OEM_MINUS) {
				if (conf_get_bool(conf, CONF_frip_mouse_font_size_wheel_shortcut)) {
					font = conf_get_fontspec(conf, CONF_font);
					if (font->height > 1) {
						font->height--;
					}
					return WF_RETURN_BREAK_RESET_WINDOW;
				} else {
					return WF_RETURN_CONTINUE;
				}
			} else if (wParam == VK_OEM_PLUS) {
				if (conf_get_bool(conf, CONF_frip_mouse_font_size_wheel_shortcut)) {
					font = conf_get_fontspec(conf, CONF_font);
					if (font->height < 32) {
						font->height++;
					}
					return WF_RETURN_BREAK_RESET_WINDOW;
				} else {
					return WF_RETURN_CONTINUE;
				}
			} else {
				return WF_RETURN_CONTINUE;
			}
		} else {
			if ((message == WM_KEYDOWN) && (wParam == VK_CONTROL)) {
				WffpMouseControlState = true;
			} else if ((message == WM_KEYUP) && (wParam == VK_CONTROL)) {
				WffpMouseControlState = false;
			}
			return WF_RETURN_CONTINUE;
		}
	}
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
