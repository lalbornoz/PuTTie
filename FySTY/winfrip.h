/*
 * winfrip.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#ifndef PUTTY_WINFRIP_H
#define PUTTY_WINFRIP_H
#ifndef PUTTY_UNIX_H

typedef enum WinFripReturn {
    WINFRIP_RETURN_BREAK			= 1,
    WINFRIP_RETURN_BREAK_RESET_WINDOW		= 2,
    WINFRIP_RETURN_CANCEL			= 3,	// winfrip_urls_op(WINFRIP_URLS_OP_RECONFIG, ...)
    WINFRIP_RETURN_CONTINUE			= 4,
    WINFRIP_RETURN_FAILURE			= 5,
    WINFRIP_RETURN_NOOP				= 6,
    WINFRIP_RETURN_RETRY			= 7,	// winfrip_urls_op(WINFRIP_URLS_OP_RECONFIG, ...)
} WinFripReturn;

/*
 * config.c:setup_config_box()
 */

void winfrip_config_panel(struct controlbox *b);

/*
 * putty.h:CONFIG_OPTIONS()
 */

#define WINFRIP_CONFIG_OPTIONS(X)											\
    X(FILENAME, NONE, frip_bgimg_filename)										\
    X(INT, NONE, frip_bgimg_opacity)											\
    X(INT, NONE, frip_bgimg_style)											\
    X(INT, NONE, frip_bgimg_type)											\
    X(BOOL, NONE, frip_general_always_on_top)										\
    X(BOOL, NONE, frip_general_minimise_to_systray)									\
    X(INT, NONE, frip_general_store_backend)										\
    X(BOOL, NONE, frip_mouse_font_size_wheel)										\
    X(INT, NONE, frip_transp_custom)											\
    X(INT, NONE, frip_transp_opaque_on)											\
    X(INT, NONE, frip_transp_setting)											\
    X(BOOL, NONE, frip_urls_browser_default)										\
    X(STR, NONE, frip_urls_browser_pname_verb)										\
    X(INT, NONE, frip_urls_modifier_key)										\
    X(INT, NONE, frip_urls_modifier_shift)										\
    X(STR, NONE, frip_urls_regex)											\
    X(BOOL, NONE, frip_urls_revvideo_onclick)										\
    X(BOOL, NONE, frip_urls_underline_onhl)

/*
 * settings.c:load_open_settings()
 */

#define WINFRIP_URLS_REGEX												\
    "((https?|ftp)://|www\\.).(([^ ]*\\([^ ]*\\))([^ ()]*[^ ,;.:\"')>])?|([^ ()]*[^ ,;.:\"')>]))"
#define WINFRIP_LOAD_OPEN_SETTINGS(sesskey, conf) do {									\
    gppfile((sesskey), "FripBgImgFile", (conf), CONF_frip_bgimg_filename);						\
    gppi((sesskey), "FripBgImgOpacity", 75, (conf), CONF_frip_bgimg_opacity);						\
    gppi((sesskey), "FripBgImgStyle", 0, (conf), CONF_frip_bgimg_style);						\
    gppi((sesskey), "FripBgImgType", 0, (conf), CONF_frip_bgimg_type);							\
    gppb((sesskey), "FripGeneralAlwaysOnTop", false, (conf), CONF_frip_general_always_on_top);				\
    gppb((sesskey), "FripGeneralMinimiseToSysTray", true, (conf), CONF_frip_general_minimise_to_systray);		\
    gppi((sesskey), "FripGeneralStoreBackEnd", 0, (conf), CONF_frip_general_store_backend);				\
    gppb((sesskey), "FripMouseFontSizeWheel", true, (conf), CONF_frip_mouse_font_size_wheel);				\
    gppi((sesskey), "FripTranspCustom", 0, (conf), CONF_frip_transp_custom);						\
    gppi((sesskey), "FripTranspOpaqueOn", 1, (conf), CONF_frip_transp_opaque_on);					\
    gppi((sesskey), "FripTranspSetting", 0, (conf), CONF_frip_transp_setting);						\
    gppb((sesskey), "FripUrlsBrowserDefault", true, (conf), CONF_frip_urls_browser_default);				\
    gpps((sesskey), "FripUrlsBrowserPNameVerb", "", (conf), CONF_frip_urls_browser_pname_verb);				\
    gppi((sesskey), "FripUrlsModifierKey", 0, (conf), CONF_frip_urls_modifier_key);					\
    gppi((sesskey), "FripUrlsModifierShift", 0, (conf), CONF_frip_urls_modifier_shift);					\
    gpps((sesskey), "FripUrlsRegex", WINFRIP_URLS_REGEX, (conf), CONF_frip_urls_regex);					\
    gppb((sesskey), "FripUrlsReverseVideoOnClick", true, (conf), CONF_frip_urls_revvideo_onclick);			\
    gppb((sesskey), "FripUrlsUnderlineOnHighlight", true, (conf), CONF_frip_urls_underline_onhl);			\
} while (0)

/*
 * settings.c:save_open_settings()
 */

#define WINFRIP_SAVE_OPEN_SETTINGS(sesskey, conf) do {									\
    write_setting_filename((sesskey), "FripBgImgFile", conf_get_filename((conf), CONF_frip_bgimg_filename));		\
    write_setting_i((sesskey), "FripBgImgOpacity", conf_get_int((conf), CONF_frip_bgimg_opacity));			\
    write_setting_i((sesskey), "FripBgImgStyle", conf_get_int((conf), CONF_frip_bgimg_style));				\
    write_setting_i((sesskey), "FripBgImgType", conf_get_int((conf), CONF_frip_bgimg_type));				\
    write_setting_b((sesskey), "FripGeneralAlwaysOnTop", conf_get_bool((conf), CONF_frip_general_always_on_top));	\
    write_setting_b((sesskey), "FripGeneralMinimiseToSysTray", conf_get_bool((conf), CONF_frip_general_minimise_to_systray));\
    write_setting_i((sesskey), "FripGeneralStoreBackEnd", conf_get_int((conf), CONF_frip_general_store_backend));	\
    write_setting_b((sesskey), "FripMouseFontSizeWheel", conf_get_bool((conf), CONF_frip_mouse_font_size_wheel));	\
    write_setting_i((sesskey), "FripTranspCustom", conf_get_int((conf), CONF_frip_transp_custom));			\
    write_setting_i((sesskey), "FripTranspOpaqueOn", conf_get_int((conf), CONF_frip_transp_opaque_on));			\
    write_setting_i((sesskey), "FripTranspSetting", conf_get_int((conf), CONF_frip_transp_setting));			\
    write_setting_b((sesskey), "FripUrlsBrowserDefault", conf_get_bool((conf), CONF_frip_urls_browser_default));	\
    write_setting_s((sesskey), "FripUrlsBrowserPNameVerb", conf_get_str((conf), CONF_frip_urls_browser_pname_verb));	\
    write_setting_i((sesskey), "FripUrlsModifierKey", conf_get_int((conf), CONF_frip_urls_modifier_key));		\
    write_setting_i((sesskey), "FripUrlsModifierShift", conf_get_int((conf), CONF_frip_urls_modifier_shift));		\
    write_setting_s((sesskey), "FripUrlsRegex", conf_get_str((conf), CONF_frip_urls_regex));				\
    write_setting_b((sesskey), "FripUrlsReverseVideoOnClick", conf_get_bool((conf), CONF_frip_urls_revvideo_onclick));	\
    write_setting_b((sesskey), "FripUrlsUnderlineOnHighlight", conf_get_bool((conf), CONF_frip_urls_underline_onhl));	\
} while (0)

/*
 * terminal.c:do_paint()
 * windows/window.c:WndProc()
 */

typedef enum WinFripUrlsOp {
    WINFRIP_URLS_OP_DRAW			= 1,
    WINFRIP_URLS_OP_FOCUS_KILL			= 2,
    WINFRIP_URLS_OP_MOUSE_BUTTON_EVENT		= 3,
    WINFRIP_URLS_OP_MOUSE_MOTION_EVENT		= 4,
    WINFRIP_URLS_OP_RECONFIG			= 5,
} WinFripUrlsOp;
WinFripReturn winfrip_urls_op(WinFripUrlsOp op, Conf *conf, HWND hwnd, UINT message, unsigned long *tattr, Terminal *term, WPARAM wParam, int x, int y);

/*
 * windows/window.c:{do_text_internal,WndProc}()
 */

typedef enum WinFripBgImgOp {
    WINFRIP_BGIMG_OP_DRAW			= 1,
    WINFRIP_BGIMG_OP_RECONF			= 2,
    WINFRIP_BGIMG_OP_SIZE			= 3,
} WinFripBgImgOp;
WinFripReturn winfrip_bgimg_op(WinFripBgImgOp op, BOOL *pbgfl, Conf *conf, HDC hdc_in, HWND hwnd, int char_width, int font_height, int len, int nbg, int rc_width, int x, int y);

/*
 * windows/window.c:{WinMain,WndProc}()
 */

typedef enum WinFripTranspOp {
    WINFRIP_TRANSP_OP_FOCUS_KILL		= 1,
    WINFRIP_TRANSP_OP_FOCUS_SET			= 2,
} WinFripTranspOp;
void winfrip_transp_op(WinFripTranspOp op, Conf *conf, HWND hwnd);

/*
 * windows/window.c:WinMain()
 */

void winfrip_debug_init(void);

/*
 * windows/window.c:WndProc()
 */

UINT winfripp_general_get_wm_systray(void);

typedef enum WinFripGeneralStoreBackEnd {
    WINFRIP_GENERAL_STORE_BACKEND_EPHEMERAL	= 0,
    WINFRIP_GENERAL_STORE_BACKEND_DEFAULT	= WINFRIP_GENERAL_STORE_BACKEND_EPHEMERAL,
    WINFRIP_GENERAL_STORE_BACKEND_FILE		= 1,
    WINFRIP_GENERAL_STORE_BACKEND_REGISTRY	= 2,
} WinFripGeneralStoreBackEnd;

typedef enum WinFripGeneralOp {
    WINFRIP_GENERAL_OP_CONFIG_DIALOG		= 1,
    WINFRIP_GENERAL_OP_FOCUS_SET		= 2,
    WINFRIP_GENERAL_OP_SYSTRAY_INIT		= 3,
    WINFRIP_GENERAL_OP_SYSTRAY_MINIMISE		= 4,
    WINFRIP_GENERAL_OP_SYSTRAY_WM_MENU		= 5,
    WINFRIP_GENERAL_OP_SYSTRAY_WM_OTHER		= 6,
} WinFripGeneralOp;
WinFripReturn winfrip_general_op(WinFripGeneralOp op, Conf *conf, HINSTANCE hinst, HWND hwnd, LPARAM lParam, int reconfiguring, WPARAM wParam);

/*
 * windows/window.c:WndProc()
 */

typedef enum WinFripMouseOp {
    WINFRIP_MOUSE_OP_MOUSE_EVENT		= 1,
    WINFRIP_MOUSE_OP_WHEEL			= 2,
} WinFripMouseOp;
WinFripReturn winfrip_mouse_op(WinFripMouseOp op, Conf *conf, UINT message, WPARAM wParam);

/*
 * windows/win{gss,store}.c
 */

WinFripGeneralStoreBackEnd winfrip_confstore_backend_get(void);
void winfrip_confstore_backend_set(WinFripGeneralStoreBackEnd new_backend);

/* Key {creation,open} methods */
LONG winfrip_confstore_RegCreateKey(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult);
LONG winfrip_confstore_RegCreateKeyEx(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
LONG winfrip_confstore_RegOpenKey(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult);

/* Key {closing,deletion,enumeration} methods */
LONG winfrip_confstore_RegCloseKey(HKEY hKey);
LONG winfrip_confstore_RegDeleteKey(HKEY hKey, LPCSTR lpSubKey);
LONG winfrip_confstore_RegEnumKey(HKEY hKey, DWORD dwIndex, LPSTR lpName, DWORD cchName);

/* Value {deletion,querying,setting} methods */
LONG winfrip_confstore_RegDeleteValue(HKEY hKey, LPCSTR lpValueName);
LONG winfrip_confstore_RegQueryValueEx(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
LONG winfrip_confstore_RegSetValueEx(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE *lpData, DWORD cbData);

#else
#define WINFRIP_CONFIG_OPTIONS(X)
#define WINFRIP_LOAD_OPEN_SETTINGS(sesskey, conf)
#define WINFRIP_SAVE_OPEN_SETTINGS(sesskey, conf)
#endif
#endif
