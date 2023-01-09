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

#define WF_URLS_REGEX																													\
		"((https?|ftp)://|www\\.).(([^ ]*\\([^ ]*\\))([^ ()]*[^ ,;.:\"')>])?|([^ ()]*[^ ,;.:\"')>]))"

#define WF_LOAD_OPEN_SETTINGS(sesskey, conf) do {																						\
		gppfile((sesskey), "FripBgImgFile", (conf), CONF_frip_bgimg_filename);															\
		gppi((sesskey), "FripBgImgOpacity", 75, (conf), CONF_frip_bgimg_opacity);														\
		gppi((sesskey), "FripBgImgPadding", 10, (conf), CONF_frip_bgimg_padding);														\
		gppi((sesskey), "FripBgImgSlideshow", 0, (conf), CONF_frip_bgimg_slideshow);													\
		gppi((sesskey), "FripBgImgSlideshowFreq", 3600, (conf), CONF_frip_bgimg_slideshow_freq);										\
		gppi((sesskey), "FripBgImgStyle", 0, (conf), CONF_frip_bgimg_style);															\
		gppi((sesskey), "FripBgImgType", 0, (conf), CONF_frip_bgimg_type);																\
		gppb((sesskey), "FripGeneralAlwaysOnTop", false, (conf), CONF_frip_general_always_on_top);										\
		gppb((sesskey), "FripGeneralMinimiseToSysTray", true, (conf), CONF_frip_general_minimise_to_systray);							\
		gppb((sesskey), "FripMouseFontSizeWheel", true, (conf), CONF_frip_mouse_font_size_wheel);										\
		gppi((sesskey), "FripTranspCustom", 0, (conf), CONF_frip_trans_custom);															\
		gppi((sesskey), "FripTranspOpaqueOn", 1, (conf), CONF_frip_trans_opaque_on);													\
		gppi((sesskey), "FripTranspSetting", 0, (conf), CONF_frip_trans_setting);														\
		gppb((sesskey), "FripUrlsBrowserDefault", true, (conf), CONF_frip_urls_browser_default);										\
		gpps((sesskey), "FripUrlsBrowserPNameVerb", "", (conf), CONF_frip_urls_browser_pname_verb);										\
		gppi((sesskey), "FripUrlsModifierKey", 0, (conf), CONF_frip_urls_modifier_key);													\
		gppi((sesskey), "FripUrlsModifierExtendShrinkKey", 1, (conf), CONF_frip_urls_modifier_extendshrink_key);						\
		gppi((sesskey), "FripUrlsModifierShift", 0, (conf), CONF_frip_urls_modifier_shift);												\
		gpps((sesskey), "FripUrlsRegex", WF_URLS_REGEX, (conf), CONF_frip_urls_regex);													\
		gppb((sesskey), "FripUrlsReverseVideoOnClick", true, (conf), CONF_frip_urls_revvideo_onclick);									\
		gppb((sesskey), "FripUrlsUnderlineOnHighlight", true, (conf), CONF_frip_urls_underline_onhl);									\
	} while (0)

/*
 * Public macros used by/in:
 * settings.c:save_open_settings()
 */

#define WF_SAVE_OPEN_SETTINGS(sesskey, conf) do {																						\
		write_setting_filename((sesskey), "FripBgImgFile", conf_get_filename((conf), CONF_frip_bgimg_filename));						\
		write_setting_i((sesskey), "FripBgImgOpacity", conf_get_int((conf), CONF_frip_bgimg_opacity));									\
		write_setting_i((sesskey), "FripBgImgPadding", conf_get_int((conf), CONF_frip_bgimg_padding));									\
		write_setting_i((sesskey), "FripBgImgSlideshow", conf_get_int((conf), CONF_frip_bgimg_slideshow));								\
		write_setting_i((sesskey), "FripBgImgSlideshowFreq", conf_get_int((conf), CONF_frip_bgimg_slideshow_freq));						\
		write_setting_i((sesskey), "FripBgImgStyle", conf_get_int((conf), CONF_frip_bgimg_style));										\
		write_setting_i((sesskey), "FripBgImgType", conf_get_int((conf), CONF_frip_bgimg_type));										\
		write_setting_b((sesskey), "FripGeneralAlwaysOnTop", conf_get_bool((conf), CONF_frip_general_always_on_top));					\
		write_setting_b((sesskey), "FripGeneralMinimiseToSysTray", conf_get_bool((conf), CONF_frip_general_minimise_to_systray));		\
		write_setting_b((sesskey), "FripMouseFontSizeWheel", conf_get_bool((conf), CONF_frip_mouse_font_size_wheel));					\
		write_setting_i((sesskey), "FripTranspCustom", conf_get_int((conf), CONF_frip_trans_custom));									\
		write_setting_i((sesskey), "FripTranspOpaqueOn", conf_get_int((conf), CONF_frip_trans_opaque_on));								\
		write_setting_i((sesskey), "FripTranspSetting", conf_get_int((conf), CONF_frip_trans_setting));									\
		write_setting_b((sesskey), "FripUrlsBrowserDefault", conf_get_bool((conf), CONF_frip_urls_browser_default));					\
		write_setting_s((sesskey), "FripUrlsBrowserPNameVerb", conf_get_str((conf), CONF_frip_urls_browser_pname_verb));				\
		write_setting_i((sesskey), "FripUrlsModifierKey", conf_get_int((conf), CONF_frip_urls_modifier_key));							\
		write_setting_i((sesskey), "FripUrlsModifierExtendShrinkKey", conf_get_int((conf), CONF_frip_urls_modifier_extendshrink_key));	\
		write_setting_i((sesskey), "FripUrlsModifierShift", conf_get_int((conf), CONF_frip_urls_modifier_shift));						\
		write_setting_s((sesskey), "FripUrlsRegex", conf_get_str((conf), CONF_frip_urls_regex));										\
		write_setting_b((sesskey), "FripUrlsReverseVideoOnClick", conf_get_bool((conf), CONF_frip_urls_revvideo_onclick));				\
		write_setting_b((sesskey), "FripUrlsUnderlineOnHighlight", conf_get_bool((conf), CONF_frip_urls_underline_onhl));				\
	} while (0)

#endif // !PUTTY_WINFRIP_PUTTY_SETTINGS_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
