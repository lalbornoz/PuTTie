/*
 * winfrip_putty_conf.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_PUTTY_CONF_H
#define PUTTY_WINFRIP_PUTTY_CONF_H

/*
 * Public macros used by/in:
 * conf.h
 */

#define WF_CONFIG_OPTIONS(CONF_OPTION)				\
CONF_OPTION(frip_bgimg_filename,				\
    VALUE_TYPE(FILENAME),					\
    SAVE_KEYWORD("FripBgImgFile"),				\
)								\
CONF_OPTION(frip_bgimg_opacity,					\
    VALUE_TYPE(INT),						\
    DEFAULT_INT(WFF_BGIMG_DEFAULT_OPACITY),			\
    SAVE_KEYWORD("FripBgImgOpacity"),				\
)								\
CONF_OPTION(frip_bgimg_padding,					\
    VALUE_TYPE(INT),						\
    DEFAULT_INT(WFF_BGIMG_DEFAULT_PADDING),			\
    SAVE_KEYWORD("FripBgImgPadding"),				\
)								\
CONF_OPTION(frip_bgimg_slideshow,				\
    VALUE_TYPE(INT),						\
    DEFAULT_INT(WFF_BGIMG_DEFAULT_SLIDESHOW),			\
    SAVE_KEYWORD("FripBgImgSlideshow"),				\
)								\
CONF_OPTION(frip_bgimg_slideshow_freq,				\
    VALUE_TYPE(INT),						\
    DEFAULT_INT(WFF_BGIMG_DEFAULT_SLIDESHOW_FREQ),		\
    SAVE_KEYWORD("FripBgImgSlideshowFreq"),			\
)								\
CONF_OPTION(frip_bgimg_style,					\
    VALUE_TYPE(INT),						\
    DEFAULT_INT(WFF_BGIMG_DEFAULT_STYLE),			\
    SAVE_KEYWORD("FripBgImgStyle"),				\
)								\
CONF_OPTION(frip_bgimg_type,					\
    VALUE_TYPE(INT),						\
    DEFAULT_INT(WFF_BGIMG_DEFAULT_TYPE),			\
    SAVE_KEYWORD("FripBgImgType"),				\
)								\
CONF_OPTION(frip_cache_passwords,				\
    VALUE_TYPE(BOOL),						\
    DEFAULT_BOOL(WFF_CACHEPASSWORD_DEFAULT_CACHE_PASSWORDS),	\
    SAVE_KEYWORD("FripCachePasswords"),				\
)								\
CONF_OPTION(frip_general_always_on_top,				\
    VALUE_TYPE(BOOL),						\
    DEFAULT_BOOL(WFF_GENERAL_DEFAULT_DUPSESS_SHORTCUT),		\
    SAVE_KEYWORD("FripGeneralDupSessShortcut"),			\
)								\
CONF_OPTION(frip_general_dupsess_shortcut,			\
    VALUE_TYPE(BOOL),						\
    DEFAULT_BOOL(WFF_GENERAL_DEFAULT_ALWAYS_ON_TOP),		\
    SAVE_KEYWORD("FripGeneralAlwaysOnTop"),			\
)								\
CONF_OPTION(frip_general_minimise_to_systray,			\
    VALUE_TYPE(BOOL),						\
    DEFAULT_BOOL(WFF_GENERAL_DEFAULT_MINIMISE_TO_SYSTRAY),	\
    SAVE_KEYWORD("FripGeneralMinimiseToSystray"),		\
)								\
CONF_OPTION(frip_general_restart_session,			\
    VALUE_TYPE(BOOL),						\
    DEFAULT_BOOL(WFF_GENERAL_DEFAULT_RESTART_SESSION),		\
    SAVE_KEYWORD("FripGeneralRestartSession"),			\
)								\
CONF_OPTION(frip_mouse_font_size_wheel,				\
    VALUE_TYPE(BOOL),						\
    DEFAULT_BOOL(WFF_MOUSE_DEFAULT_FONT_SIZE_WHEEL),		\
    SAVE_KEYWORD("FripMouseFontSizeWheel"),			\
)								\
CONF_OPTION(frip_mouse_font_size_wheel_shortcut,		\
    VALUE_TYPE(BOOL),						\
    DEFAULT_BOOL(WFF_MOUSE_DEFAULT_FONT_SIZE_WHEEL_SHORTCUT),	\
    SAVE_KEYWORD("FripMouseFontSizeWheelShortcut"),		\
)								\
CONF_OPTION(frip_trans_custom,					\
    VALUE_TYPE(INT),						\
    DEFAULT_INT(WFF_TRANS_DEFAULT_CUSTOM),			\
    SAVE_KEYWORD("FripTranspCustom"),				\
)								\
CONF_OPTION(frip_trans_opaque_on,				\
    VALUE_TYPE(INT),						\
    DEFAULT_INT(WFF_TRANS_DEFAULT_OPAQUE_ON),			\
    SAVE_KEYWORD("FripTranspOpaqueOn"),				\
)								\
CONF_OPTION(frip_trans_setting,					\
    VALUE_TYPE(INT),						\
    DEFAULT_INT(WFF_TRANS_DEFAULT_SETTING),			\
    SAVE_KEYWORD("FripTranspSetting"),				\
)								\
CONF_OPTION(frip_urls_browser_default,				\
    VALUE_TYPE(BOOL),						\
    DEFAULT_BOOL(WFF_URLS_DEFAULT_BROWSER_DEFAULT),		\
    SAVE_KEYWORD("FripUrlsBrowserDefault"),			\
)								\
CONF_OPTION(frip_urls_browser_pname_verb,			\
    VALUE_TYPE(STR),						\
    DEFAULT_STR(WFF_URLS_DEFAULT_BROWSER_PNAME_VERB),		\
    SAVE_KEYWORD("FripUrlsBrowserPNameVerb"),			\
)								\
CONF_OPTION(frip_urls_modifier_key,				\
    VALUE_TYPE(INT),						\
    DEFAULT_INT(WFF_URLS_DEFAULT_MODIFIER_KEY),			\
    SAVE_KEYWORD("FripUrlsModifierKey"),			\
)								\
CONF_OPTION(frip_urls_modifier_extendshrink_key,		\
    VALUE_TYPE(INT),						\
    DEFAULT_INT(WFF_URLS_DEFAULT_MODIFIER_EXTEND_SHRINK_KEY),	\
    SAVE_KEYWORD("FripUrlsModifierExtendShrinkKey"),		\
)								\
CONF_OPTION(frip_urls_modifier_shift,				\
    VALUE_TYPE(INT),						\
    DEFAULT_INT(WFF_URLS_DEFAULT_MODIFIER_SHIFT),		\
    SAVE_KEYWORD("FripUrlsModifierShift"),			\
)								\
CONF_OPTION(frip_urls_regex,					\
    VALUE_TYPE(STR),						\
    DEFAULT_STR(WFF_URLS_DEFAULT_REGEX),			\
    SAVE_KEYWORD("FripUrlsRegex"),				\
)								\
CONF_OPTION(frip_urls_revvideo_onclick,				\
    VALUE_TYPE(BOOL),						\
    DEFAULT_BOOL(WFF_URLS_DEFAULT_REVERSE_VIDEO_ON_CLICK),	\
    SAVE_KEYWORD("FripUrlsReverseVideoOnClick"),		\
)								\
CONF_OPTION(frip_urls_underline_onhl,				\
    VALUE_TYPE(BOOL),						\
    DEFAULT_BOOL(WFF_URLS_DEFAULT_UNDERLINE_ON_HIGHLIGHT),	\
    SAVE_KEYWORD("FripUrlsUnderlineOnHighlight"),		\
)								\

#endif // !PUTTY_WINFRIP_PUTTY_CONF_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
