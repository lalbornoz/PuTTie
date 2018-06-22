/*
 * winfrip.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#ifndef PUTTY_WINFRIP_H
#define PUTTY_WINFRIP_H

/*
 * config.c:setup_config_box()
 */
void winfrip_config_panel(struct controlbox *b);

/*
 * settings.c:load_open_settings()
 */
#define WINFRIP_LOAD_OPEN_SETTINGS(sesskey, conf) do {								\
    gppfile((sesskey), "FripBgImgFile", (conf), CONF_frip_bgimg_filename);					\
    gppi((sesskey), "FripBgImgOpacity", 75, (conf), CONF_frip_bgimg_opacity);					\
    gppi((sesskey), "FripBgImgStyle", 0, (conf), CONF_frip_bgimg_style);					\
    gppi((sesskey), "FripBgImgType", 0, (conf), CONF_frip_bgimg_type);						\
    gpps((sesskey), "FripHoverMatchSpec", "*://*", (conf), CONF_frip_hover_match_spec);				\
    gppi((sesskey), "FripTranspCustom", 0, (conf), CONF_frip_transp_custom);					\
    gppi((sesskey), "FripTranspOpaqueOn", 1, (conf), CONF_frip_transp_opaque_on);				\
    gppi((sesskey), "FripTranspSetting", 0, (conf), CONF_frip_transp_setting);					\
} while (0)

/*
 * settings.c:save_open_settings()
 */
#define WINFRIP_SAVE_OPEN_SETTINGS(sesskey, conf) do {								\
    write_setting_filename((sesskey), "FripBgImgFile", conf_get_filename((conf), CONF_frip_bgimg_filename));	\
    write_setting_i((sesskey), "FripBgImgOpacity", conf_get_int((conf), CONF_frip_bgimg_opacity));		\
    write_setting_i((sesskey), "FripBgImgStyle", conf_get_int((conf), CONF_frip_bgimg_style));			\
    write_setting_i((sesskey), "FripBgImgType", conf_get_int((conf), CONF_frip_bgimg_type));			\
    write_setting_s((sesskey), "FripHoverMatchSpec", conf_get_str((conf), CONF_frip_hover_match_spec));		\
    write_setting_i((sesskey), "FripTranspCustom", conf_get_int((conf), CONF_frip_transp_custom));		\
    write_setting_i((sesskey), "FripTranspOpaqueOn", conf_get_int((conf), CONF_frip_transp_opaque_on));		\
    write_setting_i((sesskey), "FripTranspSetting", conf_get_int((conf), CONF_frip_transp_setting));		\
} while (0)

/*
 * terminal.c:do_paint()
 */
typedef enum WinFripHoverOp {
    WINFRIP_HOVER_OP_CLEAR		= 0,
    WINFRIP_HOVER_OP_CTRL_DOWN		= 1,
    WINFRIP_HOVER_OP_CTRL_EVENT		= 2,
    WINFRIP_HOVER_OP_CTRL_UP		= 3,
    WINFRIP_HOVER_OP_DRAW		= 4,
    WINFRIP_HOVER_OP_MOUSE_DOWN		= 5,
    WINFRIP_HOVER_OP_MOUSE_EVENT	= 6,
    WINFRIP_HOVER_OP_MOUSE_UP		= 7,
} WinFripHoverOp;
BOOL winfrip_hover_op(WinFripHoverOp op, HWND hwnd, UINT message, unsigned long *tattr, Terminal *term, WPARAM wParam, int x, int y);

/*
 * windows/window.c:{do_text_internal,WndProc}()
 */
typedef enum WinFripBgImgOp {
    WINFRIP_BGIMG_OP_DRAW		= 0,
    WINFRIP_BGIMG_OP_RECONF		= 1,
    WINFRIP_BGIMG_OP_SIZE		= 2,
} WinFripBgImgOp;
BOOL winfrip_bgimg_op(WinFripBgImgOp op, HDC hdc_in, HWND hwnd, int char_width, int font_height, int len, int nbg, int rc_width, int x, int y);

/*
 * windows/window.c:WinMain()
 */
void winfrip_debug_init(void);

/*
 * windows/window.c:{WinMain,WndProc}()
 */
typedef enum WinFripTranspOp {
    WINFRIP_TRANSP_OP_FOCUS_KILL		= 0,
    WINFRIP_TRANSP_OP_FOCUS_SET		= 1,
} WinFripTranspOp;
void winfrip_transp_op(WinFripTranspOp op, HWND hwnd);

#endif
