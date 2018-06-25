/*
 * winfrip_priv.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#ifndef PUTTY_WINFRIP_PRIV_H
#define PUTTY_WINFRIP_PRIV_H

/*
 * Preprocessor macros
 */

/*
 * XXX document
 */
#define WINFRIPP_HELP_CTX					\
	"appearance.frippery:config-winfrippery"

/*
 * Private enumerations
 */

/*
 * XXX document
 */
typedef enum WinFrippGeneralAlwaysOnTop {
    WINFRIPP_GENERAL_ALWAYS_ON_TOP_NEVER	= 0,
    WINFRIPP_GENERAL_ALWAYS_ON_TOP_ALWAYS	= 1,
} WinFrippGeneralAlwaysOnTop;

/*
 * XXX document
 */
typedef enum WinFrippBgImgState {
    WINFRIPP_BGIMG_STATE_NONE			= 0,
    WINFRIPP_BGIMG_STATE_FAILED			= 1,
    WINFRIPP_BGIMG_STATE_INIT			= 2,
} WinFrippBgImgState;

/*
 * XXX document
 */
typedef enum WinFrippBgImgStyle {
    WINFRIPP_BGIMG_STYLE_ABSOLUTE		= 0,
    WINFRIPP_BGIMG_STYLE_CENTER			= 1,
    WINFRIPP_BGIMG_STYLE_STRETCH		= 2,
    WINFRIPP_BGIMG_STYLE_TILE			= 3,
} WinFrippBgImgStyle;

/*
 * XXX document
 */
typedef enum WinFrippBgImgType {
    WINFRIPP_BGIMG_TYPE_SOLID			= 0,
    WINFRIPP_BGIMG_TYPE_IMAGE			= 1,
} WinFrippBgImgType;

/*
 * XXX document
 */
typedef enum WinFrippMouseRmb {
    WINFRIPP_MOUSE_RMB_NORMAL			= 0,
    WINFRIPP_MOUSE_RMB_INHIBIT			= 1,
} WinFrippMouseRmb;

/*
 * XXX document
 */
typedef enum WinFrippMouseWheel {
    WINFRIPP_MOUSE_WHEEL_NORMAL			= 0,
    WINFRIPP_MOUSE_WHEEL_FONT_SIZE		= 1,
} WinFrippMouseWheel;

/*
 * XXX document
 */
typedef enum WinFrippTranspLevel {
    WINFRIPP_TRANSP_LEVEL_OFF			= 255,
    WINFRIPP_TRANSP_LEVEL_LOW			= 255 - 16,
    WINFRIPP_TRANSP_LEVEL_MEDIUM		= 255 - 32,
    WINFRIPP_TRANSP_LEVEL_HIGH			= 255 - 48,
} WinFrippTranspLevel;

/*
 * XXX document
 */
typedef enum WinFrippTranspOpaqueOn {
    WINFRIPP_TRANSP_OPAQUE_NEVER		= 0,
    WINFRIPP_TRANSP_OPAQUE_FOCUS_KILL		= 1,
    WINFRIPP_TRANSP_OPAQUE_FOCUS_SET		= 2,
} WinFrippTranspOpaqueOn;

/*
 * XXX document
 */
typedef enum WinFrippTranspSetting {
    WINFRIPP_TRANSP_SETTING_OFF			= 0,
    WINFRIPP_TRANSP_SETTING_LOW			= 1,
    WINFRIPP_TRANSP_SETTING_MEDIUM		= 2,
    WINFRIPP_TRANSP_SETTING_HIGH		= 3,
    WINFRIPP_TRANSP_SETTING_CUSTOM		= 4,
} WinFrippTranspSetting;

/*
 * XXX document
 */
typedef enum WinFrippUrlsState {
    WINFRIPP_URLS_STATE_NONE			= 0,
    WINFRIPP_URLS_STATE_SELECT			= 1,
    WINFRIPP_URLS_STATE_CLICK			= 2,
    WINFRIPP_URLS_STATE_CLEAR			= 3,
} WinFrippUrlsState;

/*
 * Public subroutines private to FySTY/winfrip*.c prototypes
 */

/*
 * XXX document
 */
void winfripp_bgimg_config_panel(struct controlbox *b);
void winfripp_general_config_panel(struct controlbox *b);
void winfripp_mouse_config_panel(struct controlbox *b);
void winfripp_transp_config_panel(struct controlbox *b);
void winfripp_urls_config_panel(struct controlbox *b);

/*
 * XXX document
 */
#ifdef WINFRIP_DEBUG
#define WINFRIPP_DEBUG_ASSERT(expr) do {			\
	if (!(expr)) {						\
	    WINFRIPP_DEBUGF("assertion failure: %s\n"		\
			    "GetLastError(): 0x%08x",		\
			    #expr, GetLastError());		\
	}							\
} while (0)
#define WINFRIPP_DEBUG_ASSERTF(expr, fmt, ...) do {		\
	if (!(expr)) {						\
	    WINFRIPP_DEBUGF(fmt "\n" "assertion failure: %s\n"	\
			    "GetLastError(): 0x%08x",		\
			    ##__VA_ARGS__, #expr,		\
			    GetLastError());			\
	}							\
} while (0)
#define WINFRIPP_DEBUG_FAIL()					\
	WINFRIPP_DEBUGF("failure condition", __FILE__, __func__, __LINE__)
#define WINFRIPP_DEBUGF(fmt, ...)				\
	winfripp_debugf(fmt, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
void winfripp_debugf(const char *fmt, const char *file, const char *func, int line, ...);
#else
#define WINFRIPP_DEBUG_ASSERT(expr)				\
	(void)(expr)
#define WINFRIPP_DEBUG_ASSERTF(expr, fmt, ...)			\
	(void)(expr)
#define WINFRIPP_DEBUG_FAIL()
#define WINFRIPP_DEBUGF(fmt, ...)
#endif

/*
 * XXX document
 */
BOOL winfripp_towcsdup(char *in, size_t in_size, wchar_t **pout_w);

#endif
