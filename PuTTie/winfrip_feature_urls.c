/*
 * winfrip_feature_urls.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "dialog.h"
#include "terminal.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_feature_urls.h"
#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_debug.h"
#include "PuTTie/winfrip_rtl_pcre2.h"
#include "PuTTie/winfrip_rtl_terminal.h"
#include "PuTTie/winfrip_rtl_windows.h"

/*
 * Private type definitions
 */

typedef enum WffupModifierKey {
	WFFUP_MODIFIER_KEY_CTRL		= 0,
	WFFUP_MODIFIER_KEY_DEFAULT	= WFFUP_MODIFIER_KEY_CTRL,
	WFFUP_MODIFIER_KEY_ALT		= 1,
	WFFUP_MODIFIER_KEY_RIGHT_CTRL	= 2,
	WFFUP_MODIFIER_KEY_RIGHT_ALT	= 3,
} WffupModifierKey;

typedef enum WffupModifierShift {
	WFFUP_MODIFIER_SHIFT_NONE	= 0,
	WFFUP_MODIFIER_SHIFT_DEFAULT	= WFFUP_MODIFIER_SHIFT_NONE,
	WFFUP_MODIFIER_SHIFT_LSHIFT	= 1,
	WFFUP_MODIFIER_SHIFT_RSHIFT	= 2,
} WffupModifierShift;

typedef enum WffupState {
	WFFUP_STATE_NONE		= 0,
	WFFUP_STATE_CLICK		= 1,
	WFFUP_STATE_SELECT		= 2,
} WffupState;

/*
 * Private preprocessor macros
 */

/*
 * Terminal buffer position comparision macros
 */
#define WffupPosLe(p1,px,py)	\
	((p1).y < (py) || ((p1).y == (py) && (p1).x <= (px)))
#define WffupPosLt(px,py,p2)	\
	((py) < (p2).y || ((py) == (p2).y && (px) < (p2).x))

/*
 * Private variables
 */

/*
 * UTF-16 encoded pathname to browser application or NULL (default)
 */
static wchar_t *	WffupAppW = NULL;

/*
 * Zero-based inclusive beginning and exclusive end terminal buffer coordinates
 * of URL during URL matching and selecting operations
 */
static pos		WffupMatchBegin = {0, 0};
static pos		WffupMatchEnd = {0, 0};
static pos		WffupSelectBegin = {0, 0};
static pos		WffupSelectEnd = {0, 0};

/*
 * UTF-16 encoded URL buffer and buffer size in bytes during URL operations
 * or NULL/0, resp.
 */
static wchar_t *	WffupBufW = NULL;
static size_t		WffupBufW_size = 0;

/*
 * Windows base API GetKeyState() virtual keys corresponding to the URL mouse
 * motion, URL extending/shrinking, and LMB click modifier and shift modifier
 * keys configured by the user.
 */
static int		WffupModifier = 0;
static int		WffupModifierExtendShrink = 0;
static int		WffupModifierShiftState = 0;

/*
 * PCRE2 error message buffer
 */
static wchar_t		WffupReErrorMessage[256] = {0,};

/*
 * URL operation state (WFFUP_STATE_{NONE,CLICK,SELECT})
 */
static WffupState	WffupStateCurrent = WFFUP_STATE_NONE;

/*
 * Private subroutine prototypes
 */

static void		WffupConfigPanelBrowserVerbHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static void		WffupConfigPanelModifierKeyHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static void		WffupConfigPanelModifierExtendShrinkHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static void		WffupConfigPanelModifierShiftHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static WfReturn		WffupReconfig(Conf *conf);
static int		WffupReconfigModifierKey(int modifier);
static int		WffupReconfigModifierShift(int modifier);

static bool		WffupMatchState(int x, int y);
static void		WffupResetState(Terminal *term, bool update_term);

static int		WffupOpClick(Conf *conf, UINT message, Terminal *term, int x, int y);
static int		WffupOpDraw(Conf *conf, unsigned long *tattr, Terminal *term, int x, int y);
static int		WffupOpSelect(Terminal *term, int x, int );
static int		WffupOpSelectExtendShrink(Terminal *term, WPARAM wParam);

/*
 * Private subroutines
 */

static void
WffupConfigPanelBrowserVerbHandler(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	void *		data,
	int		event
	)
{
	char *		browser_verb;
	size_t		browser_verb_len;
	wchar_t *	browser_verb_w;
	Conf *		conf = (Conf *)data;
	WfrStatus	status;


	switch (event) {
	case EVENT_REFRESH:
		dlg_editbox_set(
			ctrl, dlg,
			conf_get_str(conf, CONF_frip_urls_browser_pname_verb));
		break;

	case EVENT_VALCHANGE:
		browser_verb = dlg_editbox_get(ctrl, dlg);
		browser_verb_len = strlen(browser_verb);
		conf_set_str(conf, CONF_frip_urls_browser_pname_verb, browser_verb);

		if (WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(
				browser_verb, browser_verb_len, &browser_verb_w)))
		{
			WFR_FREE(browser_verb);
			WFR_FREE_IF_NOTNULL(WffupAppW);
			WffupAppW = browser_verb_w;
		} else {
			WFR_FREE(browser_verb);
		}
		break;
	}
}

static void
WffupConfigPanelModifierKeyHandler(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	void *		data,
	int		event
	)
{
	Conf *	conf = (Conf *)data;
	int	id;


	switch (event) {
	case EVENT_REFRESH:
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);
		dlg_listbox_addwithid(ctrl, dlg, "Ctrl", WFFUP_MODIFIER_KEY_CTRL);
		dlg_listbox_addwithid(ctrl, dlg, "Alt", WFFUP_MODIFIER_KEY_ALT);
		dlg_listbox_addwithid(ctrl, dlg, "Right Ctrl", WFFUP_MODIFIER_KEY_RIGHT_CTRL);
		dlg_listbox_addwithid(ctrl, dlg, "Right Alt/AltGr", WFFUP_MODIFIER_KEY_RIGHT_ALT);

		switch (conf_get_int(conf, CONF_frip_urls_modifier_key)) {
		case WFFUP_MODIFIER_KEY_CTRL:
			dlg_listbox_select(ctrl, dlg, 0); break;
		case WFFUP_MODIFIER_KEY_ALT:
			dlg_listbox_select(ctrl, dlg, 1); break;
		case WFFUP_MODIFIER_KEY_RIGHT_CTRL:
			dlg_listbox_select(ctrl, dlg, 2); break;
		case WFFUP_MODIFIER_KEY_RIGHT_ALT:
			dlg_listbox_select(ctrl, dlg, 3); break;
		default:
			WFR_DEBUG_FAIL(); break;
		}
		dlg_update_done(ctrl, dlg);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		id = dlg_listbox_getid(ctrl, dlg, dlg_listbox_index(ctrl, dlg));
		conf_set_int(conf, CONF_frip_urls_modifier_key, id);
		WffupModifier = WffupReconfigModifierKey(conf_get_int(conf, CONF_frip_urls_modifier_key));
		break;
	}
}

static void
WffupConfigPanelModifierExtendShrinkHandler(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	void *		data,
	int		event
	)
{
	Conf *	conf = (Conf *)data;
	int	id;


	switch (event) {
	case EVENT_REFRESH:
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);
		dlg_listbox_addwithid(ctrl, dlg, "Ctrl", WFFUP_MODIFIER_KEY_CTRL);
		dlg_listbox_addwithid(ctrl, dlg, "Alt", WFFUP_MODIFIER_KEY_ALT);
		dlg_listbox_addwithid(ctrl, dlg, "Right Ctrl", WFFUP_MODIFIER_KEY_RIGHT_CTRL);
		dlg_listbox_addwithid(ctrl, dlg, "Right Alt/AltGr", WFFUP_MODIFIER_KEY_RIGHT_ALT);

		switch (conf_get_int(conf, CONF_frip_urls_modifier_extendshrink_key)) {
		case WFFUP_MODIFIER_KEY_CTRL:
			dlg_listbox_select(ctrl, dlg, 0); break;
		case WFFUP_MODIFIER_KEY_ALT:
			dlg_listbox_select(ctrl, dlg, 1); break;
		case WFFUP_MODIFIER_KEY_RIGHT_CTRL:
			dlg_listbox_select(ctrl, dlg, 2); break;
		case WFFUP_MODIFIER_KEY_RIGHT_ALT:
			dlg_listbox_select(ctrl, dlg, 3); break;
		default:
			WFR_DEBUG_FAIL(); break;
		}
		dlg_update_done(ctrl, dlg);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		id = dlg_listbox_getid(ctrl, dlg, dlg_listbox_index(ctrl, dlg));
		conf_set_int(conf, CONF_frip_urls_modifier_extendshrink_key, id);
		WffupModifierExtendShrink = WffupReconfigModifierKey(conf_get_int(conf, CONF_frip_urls_modifier_extendshrink_key));
		break;
	}
}

static void
WffupConfigPanelModifierShiftHandler(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	void *		data,
	int		event
	)
{
	Conf *	conf = (Conf *)data;
	int	id;


	switch (event) {
	case EVENT_REFRESH:
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);
		dlg_listbox_addwithid(ctrl, dlg, "None", WFFUP_MODIFIER_SHIFT_NONE);
		dlg_listbox_addwithid(ctrl, dlg, "Shift", WFFUP_MODIFIER_SHIFT_LSHIFT);
		dlg_listbox_addwithid(ctrl, dlg, "Right Shift", WFFUP_MODIFIER_SHIFT_RSHIFT);

		switch (conf_get_int(conf, CONF_frip_urls_modifier_shift)) {
		case WFFUP_MODIFIER_SHIFT_NONE:
			dlg_listbox_select(ctrl, dlg, 0); break;
		case WFFUP_MODIFIER_SHIFT_LSHIFT:
			dlg_listbox_select(ctrl, dlg, 1); break;
		case WFFUP_MODIFIER_SHIFT_RSHIFT:
			dlg_listbox_select(ctrl, dlg, 2); break;
		default:
			WFR_DEBUG_FAIL(); break;
		}
		dlg_update_done(ctrl, dlg);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		id = dlg_listbox_getid(ctrl, dlg, dlg_listbox_index(ctrl, dlg));
		conf_set_int(conf, CONF_frip_urls_modifier_shift, id);
		WffupModifierShiftState = WffupReconfigModifierShift(conf_get_int(conf, CONF_frip_urls_modifier_shift));
		break;
	}
}

static WfReturn
WffupReconfig(
	Conf *	conf
	)
{
	static char	dlg_caption[96], dlg_text[256];
	int		re_errorcode = 0;
	PCRE2_SIZE	re_erroroffset = 0;
	char *		spec;
	size_t		spec_len;
	wchar_t *	spec_w;


	/*
	 * Obtain/update new configuration values. Reject empty regular expression
	 * strings. Convert the regular expression string to UTF-16. Release
	 * previously allocated pcre2 code resources, if any, compile the new
	 * regular expression string into pcre2 code and create a match data block,
	 * if necessary. On failure to do so, attempt to obtain the error message
	 * corresponding to the error produced during compilation.
	 */

	spec = conf_get_str(conf, CONF_frip_urls_regex); spec_len = strlen(spec);
	WffupModifier = WffupReconfigModifierKey(conf_get_int(conf, CONF_frip_urls_modifier_key));
	WffupModifierExtendShrink = WffupReconfigModifierKey(conf_get_int(conf, CONF_frip_urls_modifier_extendshrink_key));
	WffupModifierShiftState = WffupReconfigModifierShift(conf_get_int(conf, CONF_frip_urls_modifier_shift));

	if (spec_len == 0) {
		snprintf(dlg_caption, sizeof(dlg_caption), "Error compiling clickable URL regex");
		snprintf(dlg_text, sizeof(dlg_caption), "Regular expressions must not be empty.");
		goto fail;
	} else if (WFR_FAILURE(WfrConvertUtf8ToUtf16String(spec, spec_len, &spec_w))) {
		WFR_DEBUG_FAIL();
		snprintf(dlg_caption, sizeof(dlg_caption), "Error compiling clickable URL regex");
		snprintf(dlg_text, sizeof(dlg_caption), "Internal memory allocation error on calling WfrConvertUtf8ToUtf16String()");
		goto fail;
	} else if (WFR_SUCCESS(WfrInitTermLinesURLWRegex(
			spec_w, &re_errorcode, &re_erroroffset)))
	{
		WFR_FREE(spec_w);
		return WF_RETURN_CONTINUE;
	} else {
		ZeroMemory(WffupReErrorMessage, sizeof(WffupReErrorMessage));
		pcre2_get_error_message(
			re_errorcode, WffupReErrorMessage,
			sizeof(WffupReErrorMessage) / sizeof(WffupReErrorMessage[0]));

		snprintf(dlg_caption, sizeof(dlg_caption), "Error compiling clickable URL regex");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat="
		snprintf(
			dlg_text, sizeof(dlg_text),
			"Error in regex %S at offset %llu: %S",
			spec_w, re_erroroffset, WffupReErrorMessage);
#pragma GCC diagnostic pop

		goto fail;
	}

	/*
	 * On failure, create and show a modal dialogue box describing the
	 * specific error and offer the user the choice of:
	 *	1) cancelling the (re)configuration operation altogether, discarding
	 *	   the entirety of the new configuration values, or
	 *	2) continuing the (re)configuration operation w/o a compiled regular
	 *	   expression and URL matching effectively disabled, or
	 *	3) retrying the (re)configuration operation, discarding the entirety
	 *	   of the new configuration values, re-entering the configuration
	 *	   dialogue w/ focus set to the Frippery/URLs category.
	 */

fail:
	switch (WfrMessageBox(
			NULL, dlg_caption, dlg_text,
			MB_CANCELTRYCONTINUE | MB_ICONERROR))
	{
	default:
	case IDCANCEL:
		WFR_FREE(spec_w);
		return WF_RETURN_CANCEL;
	case IDCONTINUE:
		WFR_FREE(spec_w);
		return WF_RETURN_CONTINUE;
	case IDTRYAGAIN:
		WFR_FREE(spec_w);
		return WF_RETURN_RETRY;
	}
}

static int
WffupReconfigModifierKey(
	int	modifier
	)
{
	switch (modifier) {
	default:
		WFR_DEBUG_FAIL();
		return VK_CONTROL;
	case WFFUP_MODIFIER_KEY_CTRL:
		return VK_CONTROL;
	case WFFUP_MODIFIER_KEY_ALT:
		return VK_MENU;
	case WFFUP_MODIFIER_KEY_RIGHT_CTRL:
		return VK_RCONTROL;
	case WFFUP_MODIFIER_KEY_RIGHT_ALT:
		return VK_RMENU;
	}
}

static int
WffupReconfigModifierShift(
	int	modifier
	)
{
	switch (modifier) {
	default:
		WFR_DEBUG_FAIL();
		return 0;
	case WFFUP_MODIFIER_SHIFT_NONE:
		return 0;
	case WFFUP_MODIFIER_SHIFT_LSHIFT:
		return VK_LSHIFT;
	case WFFUP_MODIFIER_SHIFT_RSHIFT:
		return VK_RSHIFT;
	}
}


static bool
WffupMatchState(
	int	x,
	int	y
	)
{
	return (
		   WfrIsVKeyDown(WffupModifier)
		&& ((WffupModifierShiftState != 0) ? WfrIsVKeyDown(WffupModifierShiftState) : true)
		&& (y >= WffupMatchBegin.y)
		&& ((y == WffupMatchBegin.y) ? (x >= WffupMatchBegin.x) : true)
		&& (y <= WffupMatchEnd.y)
		&& ((y == WffupMatchEnd.y) ? (x < WffupMatchEnd.x) : true)
	);
}

static void
WffupResetState(
	Terminal *	term,
	bool		update_term
	)
{
	WffupMatchBegin.x = WffupMatchBegin.y = 0;
	WffupMatchEnd.x = WffupMatchEnd.y = 0;
	if (WffupBufW) {
		WFR_FREE(WffupBufW);
		WffupBufW = NULL, WffupBufW_size = 0;
	}
	WffupStateCurrent = WFFUP_STATE_NONE;
	if (update_term) {
		term_update(term);
	}
}


static int
WffupOpClick(
	Conf *		conf,
	UINT		message,
	Terminal *	term,
	int		x,
	int		y
	)
{
	int		rc;
	HINSTANCE	rc_shexec;


	if ((message == WM_LBUTTONDOWN)
	&&  WffupMatchState(x, y))
	{
		WffupStateCurrent = WFFUP_STATE_CLICK;
		term_update(term);

		if (conf_get_bool(conf, CONF_frip_urls_browser_default)) {
			WFR_DEBUGF("ShellExecuteW(\"open\", `%S')", WffupBufW);
			rc_shexec = ShellExecuteW(NULL, L"open", WffupBufW, NULL, NULL, SW_SHOWNORMAL);
		} else {
			WFR_DEBUGF("ShellExecuteW(`%S', `%S')", WffupAppW, WffupBufW);
			rc_shexec = ShellExecuteW(NULL, NULL, WffupAppW, WffupBufW, NULL, SW_SHOWNORMAL);
		}

		if ((INT_PTR)rc_shexec <= 32) {
			WFR_IF_STATUS_FAILURE_MESSAGEBOX(
				WFR_STATUS_FROM_WINDOWS(), NULL,
				"opening URL %S with %S",
				WffupBufW, WffupAppW ? WffupAppW : L"open");
		}

		rc = WF_RETURN_BREAK;
	} else {
		rc = WF_RETURN_CONTINUE;
	}

	WffupResetState(term, true);

	return rc;
}

static int
WffupOpDraw(
	Conf *			conf,
	unsigned long *		tattr,
	Terminal *		term,
	int			x,
	int			y
	)
{
	if (WffupPosLe(
			WffupMatchBegin, x, y - term->disptop) &&
			WffupPosLt(x, y - term->disptop, WffupMatchEnd))
	{
		switch (WffupStateCurrent) {
		case WFFUP_STATE_CLICK:
			if (conf_get_bool(conf, CONF_frip_urls_revvideo_onclick)) {
				*tattr |= ATTR_REVERSE;
			}
			break;

		case WFFUP_STATE_SELECT:
			if (conf_get_bool(conf, CONF_frip_urls_underline_onhl)) {
				*tattr |= ATTR_UNDER;
			}
			break;

		default:
			WFR_DEBUG_FAIL();
		}
	}

	return WF_RETURN_CONTINUE;
}

static int
WffupOpSelect(
	Terminal *	term,
	int		x,
	int		y
	)
{
	WfrStatus	status;


	if (WfrIsVKeyDown(WffupModifier)
	&& ((WffupModifierShiftState != 0)
	 ? WfrIsVKeyDown(WffupModifierShiftState)
	 : true))
	{
		if (WFR_SUCCESS(status = WfrGetTermLinesURLW(
				term, &WffupMatchBegin, &WffupMatchEnd,
				&WffupBufW, &WffupBufW_size,
				(pos){.x=x, .y=y}, (pos){.x=x, .y=y})))
		{
			WffupSelectBegin = (pos){.x=x, .y=y};
			WffupSelectEnd = (pos){.x=x, .y=y};
			WffupStateCurrent = WFFUP_STATE_SELECT;
			term_update(term);
		}
	}

	return WF_RETURN_CONTINUE;
}

static int
WffupOpSelectExtendShrink(
	Terminal *	term,
	WPARAM		wParam
	)
{
	wchar_t *	buf_w_new;
	WfrStatus	status;
	short		wheel_distance;


	if (WfrIsVKeyDown(WffupModifier)
	&&  WfrIsVKeyDown(WffupModifierExtendShrink)
	&&  ((WffupModifierShiftState != 0)
	 ?   WfrIsVKeyDown(WffupModifierShiftState)
	 :   true))
	{
		wheel_distance = (short)HIWORD(wParam);
		if ((wheel_distance > 0) && (WffupSelectEnd.y < term->rows)) {
			WffupSelectEnd.y++;
		} else if ((wheel_distance < 0) && (WffupSelectEnd.y > WffupSelectBegin.y)) {
			WffupSelectEnd.y--;
		}

		if (WFR_SUCCESS(status = WfrGetTermLinesURLW(
				term, &WffupMatchBegin, &WffupMatchEnd,
				&buf_w_new, &WffupBufW_size,
				WffupSelectBegin, WffupSelectEnd)))
		{
			if (WffupBufW) {
				WFR_FREE(WffupBufW);
			}
			WffupBufW = buf_w_new;
			term_update(term);
		}
		return WF_RETURN_BREAK;
	} else {
		return WF_RETURN_CONTINUE;
	}
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
WffUrlsConfigPanel(
	struct controlbox *	b
	)
{
	struct controlset *	s_browser, *s_input, *s_re, *s_visual;


	/*
	 * The Frippery: URLs panel.
	 */

	ctrl_settitle(b, "Frippery/URLs", "Configure pointless frippery: clickable URLs");

	/*
	 * The Frippery: URLs: Visual behaviour controls box.
	 */

	s_visual = ctrl_getset(b, "Frippery/URLs", "frip_urls_visual", "Visual behaviour");
	ctrl_checkbox(s_visual, "Underline hyperlinks on highlight", 'u', WFP_HELP_CTX, conf_checkbox_handler, I(CONF_frip_urls_underline_onhl));
	ctrl_checkbox(s_visual, "Apply reverse video to hyperlinks on click", 'v', WFP_HELP_CTX, conf_checkbox_handler, I(CONF_frip_urls_revvideo_onclick));

	/*
	 * The Frippery: URLs: Input behaviour controls box.
	 */

	s_input = ctrl_getset(b, "Frippery/URLs", "frip_urls_input", "Input behaviour");
	ctrl_droplist(s_input, "Modifier key:", 'm', 35, WFP_HELP_CTX, WffupConfigPanelModifierKeyHandler, P(NULL));
	ctrl_droplist(s_input, "Extend/shrink modifier key:", 'x', 35, WFP_HELP_CTX, WffupConfigPanelModifierExtendShrinkHandler, P(NULL));
	ctrl_droplist(s_input, "Shift key:", 's', 35, WFP_HELP_CTX, WffupConfigPanelModifierShiftHandler, P(NULL));

	/*
	 * The Frippery: URLs: Regular expression controls box.
	 */

	s_re = ctrl_getset(b, "Frippery/URLs", "frip_urls_regex", "Regular expression");
	ctrl_editbox(s_re, NULL, 'r', 100, WFP_HELP_CTX, conf_editbox_handler, I(CONF_frip_urls_regex), ED_STR);
	ctrl_text(s_re, "Regexes are matched against whole lines and may contain/match on whitespaces, etc.", WFP_HELP_CTX);

	/*
	 * The Frippery: URLs: Browser controls box.
	 */

	s_browser = ctrl_getset(b, "Frippery/URLs", "frip_urls_browser", "Browser");
	ctrl_checkbox(s_browser, "Default application associated with \"open\" verb", 'd', WFP_HELP_CTX, conf_checkbox_handler, I(CONF_frip_urls_browser_default));
	ctrl_text(s_browser, "Pathname to browser application:", WFP_HELP_CTX);
	ctrl_editbox(s_browser, NULL, 'p', 100, WFP_HELP_CTX, WffupConfigPanelBrowserVerbHandler, P(NULL), P(NULL));
}

/*
 * Public subroutines
 */

WfReturn
WffUrlsOperation(
	WffUrlsOp		op,
	Conf *			conf,
	HWND			hwnd,
	UINT			message,
	unsigned long *		tattr,
	Terminal *		term,
	WPARAM			wParam,
	int			x,
	int			y
	)
{
	(void)hwnd;

	switch (op) {
	case WFF_URLS_OP_INIT:
		return WffupReconfig(conf);

	/*
	 * Given WFF_URLS_OP_DRAW from terminal.c:do_paint(), apply either of
	 * the video reverse, given WFFUP_STATE_CLICK, or underline, given
	 * WFFUP_STATE_SELECT, attributes, unless configured not to do so
	 * by the user, to on-screen characters that fall into the range of the URL
	 * presently being processed, taking the scrolling displacement into
	 * account; note that this does not mutate the terminal's buffer of text on
	 * real screen.
	 */

	case WFF_URLS_OP_DRAW:
		return WffupOpDraw(conf, tattr, term, x, y);

	/*
	 * Given WFF_URLS_OP_FOCUS_KILL interrupting an URL operation, reset
	 * the URL operation state and update the terminal.
	 */

	case WFF_URLS_OP_FOCUS_KILL:
		switch (WffupStateCurrent) {
		default:
			return WF_RETURN_CONTINUE;
		case WFFUP_STATE_SELECT:
			WffupResetState(term, true);
			return WF_RETURN_CONTINUE;
		}

	case WFF_URLS_OP_MOUSE_BUTTON_EVENT:
		switch (WffupStateCurrent) {
		default:
			return WF_RETURN_CONTINUE;

		/*
		 * Given WFFUP_STATE_SELECT, a LMB click event, and
		 * satisfaction of the constraint comprised by {x,y} and the configured
		 * modifier, and optionally shift modifier, keys held down at click
		 * time, set state to WFFUP_STATE_CLICK, update the terminal
		 * to apply reverse video to the URL selected, unless configured not do
		 * so by the user, and execute the "open" verb or the browser
		 * application, as configured by the user on the URL selected.
		 * Subsequently, reset the URL operation state and update the terminal.
		 */

		case WFFUP_STATE_SELECT:
			return WffupOpClick(conf, message, term, x, y);
		}

	case WFF_URLS_OP_MOUSE_MOTION_EVENT:
		switch (WffupStateCurrent) {
		default:
			return WF_RETURN_CONTINUE;

		/*
		 * Given WFFUP_STATE_NONE and satisfaction of the constraint
		 * comprised by the configured modifier, and optionally shift modifier,
		 * keys held down at mouse motion event time, attempt to match the URL
		 * pointed into by the mouse cursor and, given a match, set state to
		 * WFFUP_STATE_SELECT and update the terminal.
		 */

		case WFFUP_STATE_NONE:
			return WffupOpSelect(term, x, y);

		/*
		 * Given WFFUP_STATE_SELECT, enforce the constraint that the
		 * configured modifier, and optionally shift modifier, keys must remain
		 * held down and that the mouse pointer must remain pointing into the
		 * region described by WffupMatch{Begin,End}.{x,y}. If this
		 * constraint is violated, reset the URL operation state and update the
		 * terminal.
		 */

		case WFFUP_STATE_SELECT:
			if (!WffupMatchState(x, y)) {
				WffupResetState(term, true);
			}
			return WF_RETURN_CONTINUE;
		}

	case WFF_URLS_OP_MOUSE_WHEEL_EVENT:
		switch (WffupStateCurrent) {
		default:
			return WF_RETURN_CONTINUE;
		case WFFUP_STATE_SELECT:
			return WffupOpSelectExtendShrink(term, wParam);
		}

	case WFF_URLS_OP_RECONFIG:
		return WffupReconfig(conf);

	default:
		WFR_DEBUG_FAIL();
		return WF_RETURN_CONTINUE;
	}
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
