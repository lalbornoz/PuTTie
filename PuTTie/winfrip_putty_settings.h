/*
 * winfrip_putty_settings.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_PUTTY_SETTINGS_H
#define PUTTY_WINFRIP_PUTTY_SETTINGS_H

/*
 * Public macros used by/in:
 * settings.c:load_open_settings()
 */

#define WF_LOAD_OPEN_SETTINGS(sesskey, conf) do {														\
	gppfile((sesskey), "FripBgImgFile", (conf), CONF_frip_bgimg_filename);											\
	gppi((sesskey), "FripBgImgOpacity", WFF_BGIMG_DEFAULT_OPACITY, (conf), CONF_frip_bgimg_opacity);							\
	gppi((sesskey), "FripBgImgPadding", WFF_BGIMG_DEFAULT_PADDING, (conf), CONF_frip_bgimg_padding);							\
	gppi((sesskey), "FripBgImgSlideshow", WFF_BGIMG_DEFAULT_SLIDESHOW, (conf), CONF_frip_bgimg_slideshow);							\
	gppi((sesskey), "FripBgImgSlideshowFreq", WFF_BGIMG_DEFAULT_SLIDESHOW_FREQ, (conf), CONF_frip_bgimg_slideshow_freq);					\
	gppi((sesskey), "FripBgImgStyle", WFF_BGIMG_DEFAULT_STYLE, (conf), CONF_frip_bgimg_style);								\
	gppi((sesskey), "FripBgImgType", WFF_BGIMG_DEFAULT_TYPE, (conf), CONF_frip_bgimg_type);									\
	gppb((sesskey), "FripCachePasswords", WFF_CACHEPASSWORD_DEFAULT_CACHE_PASSWORDS, (conf), CONF_frip_cache_passwords);					\
	gppb((sesskey), "FripGeneralAlwaysOnTop", WFF_GENERAL_DEFAULT_ALWAYS_ON_TOP, (conf), CONF_frip_general_always_on_top);					\
	gppb((sesskey), "FripGeneralMinimiseToSysTray", WFF_GENERAL_DEFAULT_MINIMISE_TO_SYSTRAY, (conf), CONF_frip_general_minimise_to_systray);		\
	gppb((sesskey), "FripGeneralRestartSession", WFF_GENERAL_DEFAULT_RESTART_SESSION, (conf), CONF_frip_general_restart_session);				\
	gppb((sesskey), "FripMouseDupSessShortcut", WFF_MOUSE_DEFAULT_DUPSESS_SHORTCUT, (conf), CONF_frip_mouse_dupsess_shortcut);				\
	gppb((sesskey), "FripMouseFontSizeWheel", WFF_MOUSE_DEFAULT_FONT_SIZE_WHEEL, (conf), CONF_frip_mouse_font_size_wheel);					\
	gppb((sesskey), "FripMouseFontSizeWheelShortcut", WFF_MOUSE_DEFAULT_FONT_SIZE_WHEEL_SHORTCUT, (conf), CONF_frip_mouse_font_size_wheel_shortcut);	\
	gppi((sesskey), "FripTranspCustom", WFF_TRANS_DEFAULT_CUSTOM, (conf), CONF_frip_trans_custom);								\
	gppi((sesskey), "FripTranspOpaqueOn", WFF_TRANS_DEFAULT_OPAQUE_ON, (conf), CONF_frip_trans_opaque_on);							\
	gppi((sesskey), "FripTranspSetting", WFF_TRANS_DEFAULT_SETTING, (conf), CONF_frip_trans_setting);							\
	gppb((sesskey), "FripUrlsBrowserDefault", WFF_URLS_DEFAULT_BROWSER_DEFAULT, (conf), CONF_frip_urls_browser_default);					\
	gpps((sesskey), "FripUrlsBrowserPNameVerb", WFF_URLS_DEFAULT_BROWSER_PNAME_VERB, (conf), CONF_frip_urls_browser_pname_verb);				\
	gppi((sesskey), "FripUrlsModifierKey", WFF_URLS_DEFAULT_MODIFIER_KEY, (conf), CONF_frip_urls_modifier_key);						\
	gppi((sesskey), "FripUrlsModifierExtendShrinkKey", WFF_URLS_DEFAULT_MODIFIER_EXTEND_SHRINK_KEY, (conf), CONF_frip_urls_modifier_extendshrink_key);	\
	gppi((sesskey), "FripUrlsModifierShift", WFF_URLS_DEFAULT_MODIFIER_SHIFT, (conf), CONF_frip_urls_modifier_shift);					\
	gpps((sesskey), "FripUrlsRegex", WFF_URLS_DEFAULT_REGEX, (conf), CONF_frip_urls_regex);									\
	gppb((sesskey), "FripUrlsReverseVideoOnClick", WFF_URLS_DEFAULT_REVERSE_VIDEO_ON_CLICK, (conf), CONF_frip_urls_revvideo_onclick);			\
	gppb((sesskey), "FripUrlsUnderlineOnHighlight", WFF_URLS_DEFAULT_UNDERLINE_ON_HIGHLIGHT, (conf), CONF_frip_urls_underline_onhl);			\
} while (0)

/*
 * Public macros used by/in:
 * settings.c:save_open_settings()
 */

#define WF_SAVE_OPEN_SETTINGS(sesskey, conf) do {											\
	write_setting_filename((sesskey), "FripBgImgFile", conf_get_filename((conf), CONF_frip_bgimg_filename));			\
	write_setting_i((sesskey), "FripBgImgOpacity", conf_get_int((conf), CONF_frip_bgimg_opacity));					\
	write_setting_i((sesskey), "FripBgImgPadding", conf_get_int((conf), CONF_frip_bgimg_padding));					\
	write_setting_i((sesskey), "FripBgImgSlideshow", conf_get_int((conf), CONF_frip_bgimg_slideshow));				\
	write_setting_i((sesskey), "FripBgImgSlideshowFreq", conf_get_int((conf), CONF_frip_bgimg_slideshow_freq));			\
	write_setting_i((sesskey), "FripBgImgStyle", conf_get_int((conf), CONF_frip_bgimg_style));					\
	write_setting_i((sesskey), "FripBgImgType", conf_get_int((conf), CONF_frip_bgimg_type));					\
	write_setting_b((sesskey), "FripCachePasswords", conf_get_bool((conf), CONF_frip_cache_passwords));				\
	write_setting_b((sesskey), "FripGeneralAlwaysOnTop", conf_get_bool((conf), CONF_frip_general_always_on_top));			\
	write_setting_b((sesskey), "FripGeneralMinimiseToSysTray", conf_get_bool((conf), CONF_frip_general_minimise_to_systray));	\
	write_setting_b((sesskey), "FripGeneralRestartSession", conf_get_bool((conf), CONF_frip_general_restart_session));		\
	write_setting_b((sesskey), "FripMouseDupSessShortcut", conf_get_bool((conf), CONF_frip_mouse_dupsess_shortcut));		\
	write_setting_b((sesskey), "FripMouseFontSizeWheel", conf_get_bool((conf), CONF_frip_mouse_font_size_wheel));			\
	write_setting_b((sesskey), "FripMouseFontSizeWheelShortcut", conf_get_bool((conf), CONF_frip_mouse_font_size_wheel_shortcut));	\
	write_setting_i((sesskey), "FripTranspCustom", conf_get_int((conf), CONF_frip_trans_custom));					\
	write_setting_i((sesskey), "FripTranspOpaqueOn", conf_get_int((conf), CONF_frip_trans_opaque_on));				\
	write_setting_i((sesskey), "FripTranspSetting", conf_get_int((conf), CONF_frip_trans_setting));					\
	write_setting_b((sesskey), "FripUrlsBrowserDefault", conf_get_bool((conf), CONF_frip_urls_browser_default));			\
	write_setting_s((sesskey), "FripUrlsBrowserPNameVerb", conf_get_str((conf), CONF_frip_urls_browser_pname_verb));		\
	write_setting_i((sesskey), "FripUrlsModifierKey", conf_get_int((conf), CONF_frip_urls_modifier_key));				\
	write_setting_i((sesskey), "FripUrlsModifierExtendShrinkKey", conf_get_int((conf), CONF_frip_urls_modifier_extendshrink_key));	\
	write_setting_i((sesskey), "FripUrlsModifierShift", conf_get_int((conf), CONF_frip_urls_modifier_shift));			\
	write_setting_s((sesskey), "FripUrlsRegex", conf_get_str((conf), CONF_frip_urls_regex));					\
	write_setting_b((sesskey), "FripUrlsReverseVideoOnClick", conf_get_bool((conf), CONF_frip_urls_revvideo_onclick));		\
	write_setting_b((sesskey), "FripUrlsUnderlineOnHighlight", conf_get_bool((conf), CONF_frip_urls_underline_onhl));		\
} while (0)

#endif // !PUTTY_WINFRIP_PUTTY_SETTINGS_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
