/*
 * winfrip_feature_urls.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_FEATURE_URLS_H
#define PUTTY_WINFRIP_FEATURE_URLS_H

/*
 * Public defaults
 */

#define	WFF_URLS_DEFAULT_BROWSER_DEFAULT		true
#define	WFF_URLS_DEFAULT_BROWSER_PNAME_VERB		""
#define	WFF_URLS_DEFAULT_MODIFIER_KEY			0
#define	WFF_URLS_DEFAULT_MODIFIER_EXTEND_SHRINK_KEY	1
#define	WFF_URLS_DEFAULT_MODIFIER_SHIFT			0
#define	WFF_URLS_DEFAULT_REGEX				"((https?|ftp)://|www\\.).(([^ ]*\\([^ ]*\\))([^ ()]*[^ ,;.:\"')>])?|([^ ()]*[^ ,;.:\"')>]))"
#define	WFF_URLS_DEFAULT_REVERSE_VIDEO_ON_CLICK		true
#define	WFF_URLS_DEFAULT_UNDERLINE_ON_HIGHLIGHT		true

/*
 * Public subroutine prototypes used by/in:
 * winfrip_putty_config.c
 */

void		WffUrlsConfigPanel(struct controlbox *b);

/*
 * Public type definitions and subroutine prototypes used by/in:
 * terminal/terminal.c:do_paint()
 * windows/window.c:WndProc()
 */

typedef enum WffUrlsOp {
	WFF_URLS_OP_INIT		= 1,
	WFF_URLS_OP_DRAW		= 2,
	WFF_URLS_OP_FOCUS_KILL		= 3,
	WFF_URLS_OP_MOUSE_BUTTON_EVENT	= 4,
	WFF_URLS_OP_MOUSE_MOTION_EVENT	= 5,
	WFF_URLS_OP_MOUSE_WHEEL_EVENT	= 6,
	WFF_URLS_OP_RECONFIG		= 7
} WffUrlsOp;

WfReturn	WffUrlsOperation(WffUrlsOp op, Conf *conf, HWND hwnd, UINT message, unsigned long *tattr, Terminal *term, WPARAM wParam, int x, int y);

#endif // !PUTTY_WINFRIP_FEATURE_URLS_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
