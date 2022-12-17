/*
 * winfrip_feature_trans.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "dialog.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_feature_trans.h"
#include "PuTTie/winfrip_rtl.h"

/*
 * Private type definitions
 */

typedef enum WinFrippTransLevel {
	WINFRIPP_TRANS_LEVEL_OFF				= 255,
	WINFRIPP_TRANS_LEVEL_DEFAULT			= WINFRIPP_TRANS_LEVEL_OFF,
	WINFRIPP_TRANS_LEVEL_LOW				= 255 - 16,
	WINFRIPP_TRANS_LEVEL_MEDIUM				= 255 - 32,
	WINFRIPP_TRANS_LEVEL_HIGH				= 255 - 48,
} WinFrippTransLevel;

typedef enum WinFrippTransOpaqueOn {
	WINFRIPP_TRANS_OPAQUE_NEVER				= 0,
	WINFRIPP_TRANS_OPAQUE_DEFAULT			= WINFRIPP_TRANS_OPAQUE_NEVER,
	WINFRIPP_TRANS_OPAQUE_FOCUS_KILL		= 1,
	WINFRIPP_TRANS_OPAQUE_FOCUS_SET			= 2,
} WinFrippTransOpaqueOn;

typedef enum WinFrippTransSetting {
	WINFRIPP_TRANS_SETTING_OFF				= 0,
	WINFRIPP_TRANS_SETTING_DEFAULT			= WINFRIPP_TRANS_SETTING_OFF,
	WINFRIPP_TRANS_SETTING_LOW				= 1,
	WINFRIPP_TRANS_SETTING_MEDIUM			= 2,
	WINFRIPP_TRANS_SETTING_HIGH				= 3,
	WINFRIPP_TRANS_SETTING_CUSTOM			= 4,
} WinFrippTransSetting;

/*
 * Private subroutine prototypes
 */

static void winfripp_trans_config_panel_opaque(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static void winfripp_trans_config_panel_setting(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);

/*
 * Private subroutines
 */

static void
winfripp_trans_config_panel_opaque(
	dlgcontrol *	ctrl,
	dlgparam *		dlg,
	void *			data,
	int				event
	)
{
	Conf *	conf = (Conf *)data;


	switch (event) {
	case EVENT_REFRESH:
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);
		dlg_listbox_addwithid(ctrl, dlg, "Never", WINFRIPP_TRANS_OPAQUE_NEVER);
		dlg_listbox_addwithid(ctrl, dlg, "Focus loss", WINFRIPP_TRANS_OPAQUE_FOCUS_KILL);
		dlg_listbox_addwithid(ctrl, dlg, "Focus", WINFRIPP_TRANS_OPAQUE_FOCUS_SET);

		switch (conf_get_int(conf, CONF_frip_trans_opaque_on)) {
		case WINFRIPP_TRANS_OPAQUE_NEVER:
			dlg_listbox_select(ctrl, dlg, 0); break;
		case WINFRIPP_TRANS_OPAQUE_FOCUS_KILL:
			dlg_listbox_select(ctrl, dlg, 1); break;
		case WINFRIPP_TRANS_OPAQUE_FOCUS_SET:
			dlg_listbox_select(ctrl, dlg, 2); break;
		default:
			WFR_DEBUG_FAIL(); break;
		}
		dlg_update_done(ctrl, dlg);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		conf_set_int(conf, CONF_frip_trans_opaque_on,
					 dlg_listbox_getid(ctrl, dlg,
									   dlg_listbox_index(ctrl, dlg)));
		break;
	}
}

static void
winfripp_trans_config_panel_setting(
	dlgcontrol *	ctrl,
	dlgparam *		dlg,
	void *			data,
	int				event
	)
{
	Conf *	conf = (Conf *)data;


	switch (event) {
	case EVENT_REFRESH:
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);
		dlg_listbox_addwithid(ctrl, dlg, "Off", WINFRIPP_TRANS_SETTING_OFF);
		dlg_listbox_addwithid(ctrl, dlg, "Low", WINFRIPP_TRANS_SETTING_LOW);
		dlg_listbox_addwithid(ctrl, dlg, "Medium", WINFRIPP_TRANS_SETTING_MEDIUM);
		dlg_listbox_addwithid(ctrl, dlg, "High", WINFRIPP_TRANS_SETTING_HIGH);
		dlg_listbox_addwithid(ctrl, dlg, "Custom", WINFRIPP_TRANS_SETTING_CUSTOM);

		switch (conf_get_int(conf, CONF_frip_trans_setting)) {
		case WINFRIPP_TRANS_SETTING_OFF:
			dlg_listbox_select(ctrl, dlg, 0); break;
		case WINFRIPP_TRANS_SETTING_LOW:
			dlg_listbox_select(ctrl, dlg, 1); break;
		case WINFRIPP_TRANS_SETTING_MEDIUM:
			dlg_listbox_select(ctrl, dlg, 2); break;
		case WINFRIPP_TRANS_SETTING_HIGH:
			dlg_listbox_select(ctrl, dlg, 3); break;
		case WINFRIPP_TRANS_SETTING_CUSTOM:
			dlg_listbox_select(ctrl, dlg, 4); break;
		default:
			WFR_DEBUG_FAIL(); break;
		}
		dlg_update_done(ctrl, dlg);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		conf_set_int(conf, CONF_frip_trans_setting,
					 dlg_listbox_getid(ctrl, dlg,
									   dlg_listbox_index(ctrl, dlg)));
		break;
	}
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
winfripp_trans_config_panel(
	struct controlbox *		b
	)
{
	struct controlset *		s;


	WFR_DEBUG_ASSERT(b);

	/*
	 * The Frippery: trans-arency panel.
	 */

	ctrl_settitle(b, "Frippery/Trans-arency", "Configure pointless frippery: trans-arency");
	s = ctrl_getset(b, "Frippery/Trans-arency", "frip_trans", "Trans-arency settings");
	ctrl_droplist(s, "Setting:", 't', 35, WINFRIPP_HELP_CTX,
				  winfripp_trans_config_panel_setting, P(NULL));
	ctrl_editbox(s, "Custom (0-255):", 'u', 15, WINFRIPP_HELP_CTX,
				 conf_editbox_handler, I(CONF_frip_trans_custom), ED_INT);
	ctrl_droplist(s, "Opaque on:", 'q', 35, WINFRIPP_HELP_CTX,
				  winfripp_trans_config_panel_opaque, P(NULL));
}

/*
 * Public subroutines
 */

void
winfrip_trans_op(
	WinFripTransOp	op,
	Conf *			conf,
	HWND			hwnd
	)
{
	LONG_PTR	ex_style, rc;
	int			opacity;


	switch (conf_get_int(conf, CONF_frip_trans_setting)) {
	default:
		WFR_DEBUG_FAIL(); return;
	case WINFRIPP_TRANS_SETTING_OFF:
		opacity = WINFRIPP_TRANS_LEVEL_OFF; break;
	case WINFRIPP_TRANS_SETTING_LOW:
		opacity = WINFRIPP_TRANS_LEVEL_LOW; break;
	case WINFRIPP_TRANS_SETTING_MEDIUM:
		opacity = WINFRIPP_TRANS_LEVEL_MEDIUM; break;
	case WINFRIPP_TRANS_SETTING_HIGH:
		opacity = WINFRIPP_TRANS_LEVEL_HIGH; break;
	case WINFRIPP_TRANS_SETTING_CUSTOM:
		opacity = conf_get_int(conf, CONF_frip_trans_custom); break;
	}

	switch (op) {
	default:
		WFR_DEBUG_FAIL(); return;
	case WINFRIP_TRANS_OP_FOCUS_KILL:
		switch (conf_get_int(conf, CONF_frip_trans_opaque_on)) {
		default:
			WFR_DEBUG_FAIL(); return;
		case WINFRIPP_TRANS_OPAQUE_FOCUS_KILL:
			ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED;
			opacity = 255;
			break;
		case WINFRIPP_TRANS_OPAQUE_NEVER:
		case WINFRIPP_TRANS_OPAQUE_FOCUS_SET:
			ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED;
			break;
		}
		break;

	case WINFRIP_TRANS_OP_FOCUS_SET:
		switch (conf_get_int(conf, CONF_frip_trans_opaque_on)) {
		default:
			WFR_DEBUG_FAIL(); return;
		case WINFRIPP_TRANS_OPAQUE_FOCUS_SET:
			ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED;
			opacity = 255;
			break;
		case WINFRIPP_TRANS_OPAQUE_NEVER:
		case WINFRIPP_TRANS_OPAQUE_FOCUS_KILL:
			ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED;
			break;
		}
		break;
	}
	rc = SetWindowLongPtr(hwnd, GWL_EXSTYLE, ex_style);
	WFR_DEBUG_ASSERT(rc > 0);

	/*
	 * Ignore return value as the Windows API is fucking braindead.
	 */
	SetLayeredWindowAttributes(hwnd, 0, opacity, LWA_ALPHA);
}

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
