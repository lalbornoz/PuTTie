/*
 * winfrip_putty_putty.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_PUTTY_PUTTY_H
#define PUTTY_WINFRIP_PUTTY_PUTTY_H

/*
 * Public macros used by/in:
 * putty.h:CONFIG_OPTIONS()
 */

#define WF_CONFIG_OPTIONS(X)					\
	X(FILENAME, NONE, frip_bgimg_filename)			\
	X(INT, NONE, frip_bgimg_opacity)			\
	X(INT, NONE, frip_bgimg_padding)			\
	X(INT, NONE, frip_bgimg_slideshow)			\
	X(INT, NONE, frip_bgimg_slideshow_freq)			\
	X(INT, NONE, frip_bgimg_style)				\
	X(INT, NONE, frip_bgimg_type)				\
	X(BOOL, NONE, frip_general_always_on_top)		\
	X(BOOL, NONE, frip_general_minimise_to_systray)		\
	X(BOOL, NONE, frip_mouse_font_size_wheel)		\
	X(INT, NONE, frip_trans_custom)				\
	X(INT, NONE, frip_trans_opaque_on)			\
	X(INT, NONE, frip_trans_setting)			\
	X(BOOL, NONE, frip_urls_browser_default)		\
	X(STR, NONE, frip_urls_browser_pname_verb)		\
	X(INT, NONE, frip_urls_modifier_key)			\
	X(INT, NONE, frip_urls_modifier_extendshrink_key)	\
	X(INT, NONE, frip_urls_modifier_shift)			\
	X(STR, NONE, frip_urls_regex)				\
	X(BOOL, NONE, frip_urls_revvideo_onclick)		\
	X(BOOL, NONE, frip_urls_underline_onhl)

#endif // !PUTTY_WINFRIP_PUTTY_PUTTY_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
