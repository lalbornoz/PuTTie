/*
 * winfrip_feature_mouse.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "dialog.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_feature_mouse.h"
#include "PuTTie/winfrip_rtl.h"

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
winfripp_mouse_config_panel(
	struct controlbox *		b
	)
{
	struct controlset *	s;


	WFR_DEBUG_ASSERT(b);

	/*
	 * The Frippery: mouse panel.
	 */

	ctrl_settitle(b, "Frippery/Mouse", "Configure pointless frippery: mouse behaviour");
	s = ctrl_getset(b, "Frippery/Mouse", "frip_mouse", "Mouse behaviour");

	ctrl_checkbox(s, "Change font size with mouse wheel", 'n', WINFRIPP_HELP_CTX,
				  conf_checkbox_handler, I(CONF_frip_mouse_font_size_wheel));
	ctrl_text(s, "This only affects mouse wheel actions with the CTRL modifier.", WINFRIPP_HELP_CTX);
}

/*
 * Public subroutines
 */

WinFripReturn
winfrip_mouse_op(
	WinFripMouseOp	op,
	Conf *			conf,
	UINT			message,
	WPARAM			wParam
	)
{
	FontSpec *	font;
	short		wheel_distance;


	if (op == WINFRIP_MOUSE_OP_MOUSE_EVENT) {
		if (message == WM_MOUSEWHEEL) {
			op = WINFRIP_MOUSE_OP_WHEEL;
		} else {
			return WINFRIP_RETURN_CONTINUE;
		}
	}

	switch (op) {
	default:
		WFR_DEBUG_FAIL();
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
					WFR_DEBUG_FAIL();
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

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
