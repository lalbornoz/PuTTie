/*
 * winfrip_feature.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_FEATURE_H
#define PUTTY_WINFRIP_FEATURE_H

/*
 * Public preprocessor macros used by/in:
 * PuTTie/winfrip_feature_*.c
 */

#define WINFRIPP_HELP_CTX	\
		"appearance.frippery:config-winfrippery"

/*
 * Public type definitions used by/in:
 * PuTTie/winfrip_feature_*.[ch]
 */

typedef enum WinFripReturn {
	WINFRIP_RETURN_BREAK				= 1,
	WINFRIP_RETURN_BREAK_RESET_WINDOW	= 2,
	WINFRIP_RETURN_CANCEL				= 3,
	WINFRIP_RETURN_CONTINUE				= 4,
	WINFRIP_RETURN_FAILURE				= 5,
	WINFRIP_RETURN_NOOP					= 6,
	WINFRIP_RETURN_RETRY				= 7,
} WinFripReturn;

/*
 * Public subroutine prototypes used by/in:
 * winfrip_putty_config.c
 */

void winfripp_bgimg_config_panel(struct controlbox *b);
void winfripp_general_config_panel(struct controlbox *b);
void winfripp_mouse_config_panel(struct controlbox *b);
void winfripp_trans_config_panel(struct controlbox *b);
void winfripp_urls_config_panel(struct controlbox *b);

#endif // !PUTTY_WINFRIP_FEATURE_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
