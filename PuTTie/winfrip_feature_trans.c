/*
 * winfrip_feature_trans.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
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

typedef enum WfftpLevel {
	WFFTP_LEVEL_OFF				= 255,
	WFFTP_LEVEL_DEFAULT			= WFFTP_LEVEL_OFF,
	WFFTP_LEVEL_LOW				= 255 - 16,
	WFFTP_LEVEL_MEDIUM			= 255 - 32,
	WFFTP_LEVEL_HIGH			= 255 - 48,
} WfftpLevel;

typedef enum WfftpOpaqueOn {
	WFFTP_OPAQUE_NEVER			= 0,
	WFFTP_OPAQUE_DEFAULT		= WFFTP_OPAQUE_NEVER,
	WFFTP_OPAQUE_FOCUS_KILL		= 1,
	WFFTP_OPAQUE_FOCUS_SET		= 2,
} WfftpOpaqueOn;

typedef enum WfftpSetting {
	WFFTP_SETTING_OFF			= 0,
	WFFTP_SETTING_DEFAULT		= WFFTP_SETTING_OFF,
	WFFTP_SETTING_LOW			= 1,
	WFFTP_SETTING_MEDIUM		= 2,
	WFFTP_SETTING_HIGH			= 3,
	WFFTP_SETTING_CUSTOM		= 4,
} WfftpSetting;

/*
 * Private subroutine prototypes
 */

static void WfftpConfigPanelOpaque(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static void WfftpConfigPanelSetting(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);

/*
 * Private subroutines
 */

static void
WfftpConfigPanelOpaque(
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
		dlg_listbox_addwithid(ctrl, dlg, "Never", WFFTP_OPAQUE_NEVER);
		dlg_listbox_addwithid(ctrl, dlg, "Focus loss", WFFTP_OPAQUE_FOCUS_KILL);
		dlg_listbox_addwithid(ctrl, dlg, "Focus", WFFTP_OPAQUE_FOCUS_SET);

		switch (conf_get_int(conf, CONF_frip_trans_opaque_on)) {
		case WFFTP_OPAQUE_NEVER:
			dlg_listbox_select(ctrl, dlg, 0); break;
		case WFFTP_OPAQUE_FOCUS_KILL:
			dlg_listbox_select(ctrl, dlg, 1); break;
		case WFFTP_OPAQUE_FOCUS_SET:
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
WfftpConfigPanelSetting(
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
		dlg_listbox_addwithid(ctrl, dlg, "Off", WFFTP_SETTING_OFF);
		dlg_listbox_addwithid(ctrl, dlg, "Low", WFFTP_SETTING_LOW);
		dlg_listbox_addwithid(ctrl, dlg, "Medium", WFFTP_SETTING_MEDIUM);
		dlg_listbox_addwithid(ctrl, dlg, "High", WFFTP_SETTING_HIGH);
		dlg_listbox_addwithid(ctrl, dlg, "Custom", WFFTP_SETTING_CUSTOM);

		switch (conf_get_int(conf, CONF_frip_trans_setting)) {
		case WFFTP_SETTING_OFF:
			dlg_listbox_select(ctrl, dlg, 0); break;
		case WFFTP_SETTING_LOW:
			dlg_listbox_select(ctrl, dlg, 1); break;
		case WFFTP_SETTING_MEDIUM:
			dlg_listbox_select(ctrl, dlg, 2); break;
		case WFFTP_SETTING_HIGH:
			dlg_listbox_select(ctrl, dlg, 3); break;
		case WFFTP_SETTING_CUSTOM:
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
WffTransConfigPanel(
	struct controlbox *		b
	)
{
	struct controlset *		s;


	/*
	 * The Frippery: trans-arency panel.
	 */

	ctrl_settitle(b, "Frippery/Trans-arency", "Configure pointless frippery: trans-arency");
	s = ctrl_getset(b, "Frippery/Trans-arency", "frip_trans", "Trans-arency settings");
	ctrl_droplist(s, "Setting:", 't', 35, WFP_HELP_CTX,
				  WfftpConfigPanelSetting, P(NULL));
	ctrl_editbox(s, "Custom (0-255):", 'u', 15, WFP_HELP_CTX,
				 conf_editbox_handler, I(CONF_frip_trans_custom), ED_INT);
	ctrl_droplist(s, "Opaque on:", 'q', 35, WFP_HELP_CTX,
				  WfftpConfigPanelOpaque, P(NULL));
}

/*
 * Public subroutines
 */

void
WffTransOperation(
	WffTransOp	op,
	Conf *		conf,
	HWND		hwnd
	)
{
	LONG_PTR	ex_style;
	int			opacity;


	switch (conf_get_int(conf, CONF_frip_trans_setting)) {
	default:
		WFR_DEBUG_FAIL(); return;
	case WFFTP_SETTING_OFF:
		opacity = WFFTP_LEVEL_OFF; break;
	case WFFTP_SETTING_LOW:
		opacity = WFFTP_LEVEL_LOW; break;
	case WFFTP_SETTING_MEDIUM:
		opacity = WFFTP_LEVEL_MEDIUM; break;
	case WFFTP_SETTING_HIGH:
		opacity = WFFTP_LEVEL_HIGH; break;
	case WFFTP_SETTING_CUSTOM:
		opacity = conf_get_int(conf, CONF_frip_trans_custom); break;
	}

	switch (op) {
	default:
		WFR_DEBUG_FAIL(); return;
	case WFF_TRANS_OP_FOCUS_KILL:
		switch (conf_get_int(conf, CONF_frip_trans_opaque_on)) {
		default:
			WFR_DEBUG_FAIL(); return;
		case WFFTP_OPAQUE_FOCUS_KILL:
			ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED;
			opacity = 255;
			break;
		case WFFTP_OPAQUE_NEVER:
		case WFFTP_OPAQUE_FOCUS_SET:
			ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED;
			break;
		}
		break;

	case WFF_TRANS_OP_FOCUS_SET:
		switch (conf_get_int(conf, CONF_frip_trans_opaque_on)) {
		default:
			WFR_DEBUG_FAIL(); return;
		case WFFTP_OPAQUE_FOCUS_SET:
			ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED;
			opacity = 255;
			break;
		case WFFTP_OPAQUE_NEVER:
		case WFFTP_OPAQUE_FOCUS_KILL:
			ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED;
			break;
		}
		break;
	}
	(void)SetWindowLongPtr(hwnd, GWL_EXSTYLE, ex_style);

	/*
	 * Ignore return value as the Windows API is fucking braindead.
	 */
	(void)SetLayeredWindowAttributes(hwnd, 0, opacity, LWA_ALPHA);
}

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
