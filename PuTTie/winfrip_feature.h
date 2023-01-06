/*
 * winfrip_feature.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_FEATURE_H
#define PUTTY_WINFRIP_FEATURE_H

/*
 * Public preprocessor macros used by/in:
 * PuTTie/winfrip_feature_*.c
 */

#define WFP_HELP_CTX	\
		"appearance.frippery:config-WFPery"

/*
 * Public type definitions used by/in:
 * PuTTie/winfrip_feature_*.[ch]
 */

typedef enum WfReturn {
	WF_RETURN_BREAK					= 1,
	WF_RETURN_BREAK_RESET_WINDOW	= 2,
	WF_RETURN_CANCEL				= 3,
	WF_RETURN_CONTINUE				= 4,
	WF_RETURN_FAILURE				= 5,
	WF_RETURN_NOOP					= 6,
	WF_RETURN_RETRY					= 7,
} WfReturn;

#endif // !PUTTY_WINFRIP_FEATURE_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
