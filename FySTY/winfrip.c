/*
 * winfrip.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "winfrip.h"
#include "winfrip_priv.h"

#include <assert.h>

/*
 * Preprocessor macros
 */

/*
 * XXX document
 */
#define WINHELP_CTX_appearance_frippery "appearance.frippery:config-winfrippery"

/*
 * XXX document
 */
#define WINFRIP_BGIMG_FILTER_IMAGE_FILES (									\
	"All Picture Files\0*.bmp;*.emf;*.gif;*.ico;*.jpg;*.jpeg;*.jpe;*.jfif;*.png;*.tif;*.tiff;*.wmf\0"	\
	"Bitmap Files (*.bmp)\0*.bmp\0"										\
	"EMF (*.emf)\0*.emf\0"											\
	"GIF (*.gif)\0*.gif\0"											\
	"ICO (*.ico)\0*.ico\0"											\
	"JPEG (*.jpg;*.jpeg;*.jpe;*.jfif)\0*.jpg;*.jpeg;*.jpe;*.jfif\0"						\
	"PNG (*.png)\0*.png\0"											\
	"TIFF (*.tif;*.tiff)\0*.tif;*.tiff\0"									\
	"WMF (*.wmf)\0*.wmf\0"											\
	"All Files (*.*)\0*\0"											\
	"\0\0")

/*
 * Public subroutines private to FySTY/winfrip*.c
 */

BOOL winfrip_towcsdup(char *in, size_t in_size, wchar_t **pout_w)
{
    size_t out_w_len, out_w_size;
    wchar_t *out_w;


    /*
     * XXX document
     */
    out_w_len = MultiByteToWideChar(CP_ACP, 0, in, in_size, NULL, 0);
    if (out_w_len > 0) {
	out_w_size = out_w_len * sizeof(*out_w);
	out_w = snewn(out_w_size, wchar_t);
	ZeroMemory(out_w, out_w_size);
	if (MultiByteToWideChar(CP_ACP, 0, in, in_size, out_w, out_w_size) == out_w_len) {
	    *pout_w = out_w;
	    return TRUE;
	}
    }
    return FALSE;
}

/*
 * Public subroutines
 */

void winfrip_config_panel(struct controlbox *b)
{
    struct controlset *s;


    /*
     * The Window/Frippery panel.
     */

    ctrl_settitle(b, "Window/Frippery", "Configure pointless frippery");
    s = ctrl_getset(b, "Window/Frippery", "frip", "Click actions");
    ctrl_radiobuttons(s, "Right mouse button:", NO_SHORTCUT, 2, HELPCTX(appearance_frippery),
		      conf_radiobutton_handler, I(CONF_frip_mouse_rmb),
		      "Normal",		NO_SHORTCUT,	I(WINFRIP_MOUSE_RMB_NORMAL),
		      "Inhibit",	NO_SHORTCUT,	I(WINFRIP_MOUSE_RMB_INHIBIT), NULL);
    ctrl_text(s, "This only affects click actions with no modifiers, e.g. CTRL, ALT, and/or SHIFT.",
	      HELPCTX(appearance_frippery));

    /*
     * The Window/Frippery: background panel.
     */

    ctrl_settitle(b, "Window/Frippery: background image", "Configure pointless frippery: background image");
    s = ctrl_getset(b, "Window/Frippery: background image", "frip_bgimg", "Background image settings");
    ctrl_filesel(s, "Image file:", NO_SHORTCUT,
		 WINFRIP_BGIMG_FILTER_IMAGE_FILES, FALSE, "Select background image file",
		 HELPCTX(appearance_frippery), conf_filesel_handler, I(CONF_frip_bgimg_filename));
    ctrl_editbox(s, "Opacity (0-100):", NO_SHORTCUT, 20, HELPCTX(appearance_frippery),
		 conf_editbox_handler, I(CONF_frip_bgimg_opacity), I(-1));
    ctrl_radiobuttons(s, "Style:", NO_SHORTCUT, 4, HELPCTX(appearance_frippery),
		      conf_radiobutton_handler, I(CONF_frip_bgimg_style),
		      "Absolute",	NO_SHORTCUT,	I(WINFRIP_BGIMG_STYLE_ABSOLUTE),
		      "Center",		NO_SHORTCUT,	I(WINFRIP_BGIMG_STYLE_CENTER),
		      "Stretch",	NO_SHORTCUT,	I(WINFRIP_BGIMG_STYLE_STRETCH),
		      "Tile",		NO_SHORTCUT,	I(WINFRIP_BGIMG_STYLE_TILE), NULL);
    ctrl_radiobuttons(s, "Type:", NO_SHORTCUT, 2, HELPCTX(appearance_frippery),
		      conf_radiobutton_handler, I(CONF_frip_bgimg_type),
		      "Solid",		NO_SHORTCUT,	I(WINFRIP_BGIMG_TYPE_SOLID),
		      "Image",		NO_SHORTCUT,	I(WINFRIP_BGIMG_TYPE_IMAGE), NULL);

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

    /*
     * The Window/Frippery: URLs panel.
     */

    ctrl_settitle(b, "Window/Frippery: URLs", "Configure pointless frippery: clickable URLs");
    s = ctrl_getset(b, "Window/Frippery: URLs", "frip_urls", "Clickable URLs settings");
    ctrl_editbox(s, "Match string", NO_SHORTCUT, 75, HELPCTX(appearance_frippery),
		 conf_editbox_handler, I(CONF_frip_urls_match_spec), I(1));
    ctrl_text(s, "Specify multiple match strings by separating them with "
	      "a single semicolon.", HELPCTX(appearance_frippery));
}

void winfrip_debug_init(void)
{
    /*
     * XXX document
     */
#ifdef WINFRIP_DEBUG
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
#endif
}
