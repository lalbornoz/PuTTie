/*
 * winfrip_priv.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_PRIV_H
#define PUTTY_WINFRIP_PRIV_H

/*
 * Preprocessor macros
 */

#define WINFRIPP_HELP_CTX	\
		"appearance.frippery:config-winfrippery"

/*
 * Private enumerations
 */

typedef enum WinFrippBgImgState {
	WINFRIPP_BGIMG_STATE_NONE				= 0,
	WINFRIPP_BGIMG_STATE_FAILED				= 1,
	WINFRIPP_BGIMG_STATE_INIT				= 2,
} WinFrippBgImgState;

typedef enum WinFrippBgImgSlideshow {
	WINFRIPP_BGIMG_SLIDESHOW_SINGLE_IMAGE	= 0,
	WINFRIPP_BGIMG_SLIDESHOW_DEFAULT		= WINFRIPP_BGIMG_SLIDESHOW_SINGLE_IMAGE,
	WINFRIPP_BGIMG_SLIDESHOW_SHUFFLE		= 1,
} WinFrippBgImgSlideshow;

typedef enum WinFrippBgImgStyle {
	WINFRIPP_BGIMG_STYLE_ABSOLUTE			= 0,
	WINFRIPP_BGIMG_STYLE_DEFAULT			= WINFRIPP_BGIMG_STYLE_ABSOLUTE,
	WINFRIPP_BGIMG_STYLE_CENTER				= 1,
	WINFRIPP_BGIMG_STYLE_FIT				= 2,
	WINFRIPP_BGIMG_STYLE_STRETCH			= 3,
	WINFRIPP_BGIMG_STYLE_TILE				= 4,
} WinFrippBgImgStyle;

typedef enum WinFrippBgImgType {
	WINFRIPP_BGIMG_TYPE_SOLID				= 0,
	WINFRIPP_BGIMG_TYPE_DEFAULT				= WINFRIPP_BGIMG_TYPE_SOLID,
	WINFRIPP_BGIMG_TYPE_IMAGE				= 1,
} WinFrippBgImgType;

typedef enum WinFrippTranspLevel {
	WINFRIPP_TRANSP_LEVEL_OFF				= 255,
	WINFRIPP_TRANSP_LEVEL_DEFAULT			= WINFRIPP_TRANSP_LEVEL_OFF,
	WINFRIPP_TRANSP_LEVEL_LOW				= 255 - 16,
	WINFRIPP_TRANSP_LEVEL_MEDIUM			= 255 - 32,
	WINFRIPP_TRANSP_LEVEL_HIGH				= 255 - 48,
} WinFrippTranspLevel;

typedef enum WinFrippTranspOpaqueOn {
	WINFRIPP_TRANSP_OPAQUE_NEVER			= 0,
	WINFRIPP_TRANSP_OPAQUE_DEFAULT			= WINFRIPP_TRANSP_OPAQUE_NEVER,
	WINFRIPP_TRANSP_OPAQUE_FOCUS_KILL		= 1,
	WINFRIPP_TRANSP_OPAQUE_FOCUS_SET		= 2,
} WinFrippTranspOpaqueOn;

typedef enum WinFrippTranspSetting {
	WINFRIPP_TRANSP_SETTING_OFF				= 0,
	WINFRIPP_TRANSP_SETTING_DEFAULT			= WINFRIPP_TRANSP_SETTING_OFF,
	WINFRIPP_TRANSP_SETTING_LOW				= 1,
	WINFRIPP_TRANSP_SETTING_MEDIUM			= 2,
	WINFRIPP_TRANSP_SETTING_HIGH			= 3,
	WINFRIPP_TRANSP_SETTING_CUSTOM			= 4,
} WinFrippTranspSetting;

typedef enum WinFrippUrlsModifierKey {
	WINFRIPP_URLS_MODIFIER_KEY_CTRL			= 0,
	WINFRIPP_URLS_MODIFIER_KEY_DEFAULT		= WINFRIPP_URLS_MODIFIER_KEY_CTRL,
	WINFRIPP_URLS_MODIFIER_KEY_ALT			= 1,
	WINFRIPP_URLS_MODIFIER_KEY_RIGHT_CTRL	= 2,
	WINFRIPP_URLS_MODIFIER_KEY_RIGHT_ALT	= 3,
} WinFrippUrlsModifierKey;

typedef enum WinFrippUrlsModifierShift {
	WINFRIPP_URLS_MODIFIER_SHIFT_NONE		= 0,
	WINFRIPP_URLS_MODIFIER_SHIFT_DEFAULT	= WINFRIPP_URLS_MODIFIER_SHIFT_NONE,
	WINFRIPP_URLS_MODIFIER_SHIFT_LSHIFT		= 1,
	WINFRIPP_URLS_MODIFIER_SHIFT_RSHIFT		= 2,
} WinFrippUrlsModifierShift;

typedef enum WinFrippUrlsState {
	WINFRIPP_URLS_STATE_NONE				= 0,
	WINFRIPP_URLS_STATE_CLICK				= 1,
	WINFRIPP_URLS_STATE_SELECT				= 2,
} WinFrippUrlsState;

/*
 * Public subroutines private to PuTTie/winfrip*.c prototypes
 */

void winfripp_bgimg_config_panel(struct controlbox *b);
void winfripp_general_config_panel(struct controlbox *b);
void winfripp_mouse_config_panel(struct controlbox *b);
void winfripp_transp_config_panel(struct controlbox *b);
void winfripp_urls_config_panel(struct controlbox *b);

#ifdef WINFRIP_DEBUG
#define WINFRIPP_DEBUG_ASSERT(expr) do {					\
	if (!(expr)) {											\
		WINFRIPP_DEBUGF("assertion failure: %s\n"			\
			    "GetLastError(): 0x%08x",					\
			    #expr, GetLastError());						\
	}														\
} while (0)
#define WINFRIPP_DEBUG_ASSERTF(expr, fmt, ...) do {			\
	if (!(expr)) {											\
		WINFRIPP_DEBUGF(fmt "\n" "assertion failure: %s\n"	\
			    "GetLastError(): 0x%08x",					\
			    ##__VA_ARGS__, #expr,						\
			    GetLastError());							\
	}														\
} while (0)
#define WINFRIPP_DEBUG_FAIL()								\
	WINFRIPP_DEBUGF("failure condition", __FILE__, __func__, __LINE__)
#define WINFRIPP_DEBUGF(fmt, ...)							\
	winfripp_debugf(fmt, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
void winfripp_debugf(const char *fmt, const char *file, const char *func, int line, ...);
#else
#define WINFRIPP_DEBUG_ASSERT(expr)							\
	(void)(expr)
#define WINFRIPP_DEBUG_ASSERTF(expr, fmt, ...)				\
	(void)(expr)
#define WINFRIPP_DEBUG_FAIL()
#define WINFRIPP_DEBUGF(fmt, ...)
#endif

BOOL winfripp_get_term_line(Terminal *term, wchar_t **pline_w, size_t *pline_w_len, int y);
BOOL winfripp_is_vkey_down(int nVirtKey);
BOOL winfripp_towcsdup(char *in, size_t in_size, wchar_t **pout_w);
wchar_t *winfripp_wcsndup(const wchar_t *in_w, size_t in_w_len);

#endif

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
