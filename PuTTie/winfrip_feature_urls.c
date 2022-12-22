/*
 * winfrip_feature_urls.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
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
#include "PuTTie/winfrip_rtl_pcre2.h"
#include "PuTTie/winfrip_rtl_putty.h"

/*
 * Private type definitions
 */

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
 * Private preprocessor macros
 */

/*
 * Terminal buffer position comparision macros
 */
#define winfripp_urls_posle(p1,px,py)	\
		((p1).y < (py) || ((p1).y == (py) && (p1).x <= (px)))
#define winfripp_urls_poslt(px,py,p2)	\
		((py) < (p2).y || ((py) == (p2).y && (px) < (p2).x))

/*
 * Private variables
 */

/*
 * UTF-16 encoded pathname to browser application or NULL (default)
 */
static wchar_t *			winfripp_urls_app_w = NULL;

/*
 * Zero-based inclusive beginning and exclusive end terminal buffer coordinates
 * of URL during URL operations
 */
static pos					winfripp_urls_begin = {0, 0};
static pos					winfripp_urls_end = {0, 0};

/*
 * UTF-16 encoded URL buffer and buffer size in bytes during URL operations
 * or NULL/0, resp.
 */
static wchar_t *			winfripp_urls_buf_w = NULL;
static size_t				winfripp_urls_buf_w_size = 0;

/*
 * Windows base API GetKeyState() virtual keys corresponding to the URL mouse
 * motion and LMB click modifier and shift modifier keys configured by the user.
 */
static int					winfripp_urls_modifier = 0;
static int					winfripp_urls_modifier_shift = 0;

/*
 * PCRE2 regular expression compiled code or NULL, error message buffer, and
 * match data block or NULL
 */
static pcre2_code *			winfripp_re_code = NULL;
static wchar_t				winfripp_re_error_message[256] = {0,};
static pcre2_match_data *	winfripp_re_md = NULL;

/*
 * URL operation state (WINFRIPP_URLS_STATE_{NONE,CLICK,SELECT})
 */
static WinFrippUrlsState	winfripp_urls_state = WINFRIPP_URLS_STATE_NONE;

/*
 * Private subroutine prototypes
 */

static bool winfripp_urls_get(Terminal *term, pos *pbegin, pos *pend, wchar_t **phover_url_w, size_t *phover_url_w_size, int x, int y);
static void winfripp_urls_config_panel_modifier_key_handler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static void winfripp_urls_config_panel_modifier_shift_handler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static WinFripReturn winfripp_urls_reconfig(Conf *conf);
static void winfripp_urls_reconfig_modifier_key(Conf *conf);
static void winfripp_urls_reconfig_modifier_shift(Conf *conf);
static bool winfripp_urls_state_match(int x, int y);
static void winfripp_urls_state_reset(Terminal *term, bool update_term);

/*
 * Private subroutines
 */

static bool
winfripp_urls_get(
	Terminal *	term,
	pos *		pbegin,
	pos *		pend,
	wchar_t **	phover_url_w,
	size_t *	phover_url_w_size,
	int			x,
	int			y
	)
{
	bool				breakfl = false;
	wchar_t *			line_w;
	size_t				line_w_len;
	size_t				match_begin, match_end, match_len;
	WinFrippP2MGState	pcre2_state;


	WFR_DEBUG_ASSERT(term);
	WFR_DEBUG_ASSERT(pbegin);
	WFR_DEBUG_ASSERT(pend);
	WFR_DEBUG_ASSERT(phover_url_w);
	WFR_DEBUG_ASSERT(phover_url_w_size);

	/*
	 * Fail given non-initialised pcre2 regular expression {code,match data
	 * block}, whether due to failure to initialise or an invalid regular
	 * expression provided by the user. Reject invalid {x,y} coordinates, be
	 * they negative or falling outside of term->{cols,rows}. Fail given
	 * failure to get the line from the terminal's buffer of text on real
	 * screen specified by the coordinate y.
	 */

	if (!winfripp_re_code || !winfripp_re_md) {
		WFR_DEBUG_FAIL();
		return FALSE;
	} else if ((x < 0) || (x >= term->cols) || (y < 0) || (y >= term->rows)) {
		WFR_DEBUG_FAIL();
		return FALSE;
	} else if (WFR_STATUS_FAILURE(WfrpGetTermLine(term, &line_w, &line_w_len, y))) {
		WFR_DEBUG_FAIL();
		return FALSE;
	} else {
		WinfrippPcre2Init(&pcre2_state, winfripp_re_code,
							line_w_len, winfripp_re_md, line_w);
	}

	/*
	 * Iteratively attempt to match the regular expression and UTF-16 encoded
	 * terminal buffer string described by pcre2_state until either a non-zero
	 * length match comprising a range intersected by the x coordinate parameter
	 * is found or until either WFR_STATUS_CONDITION_PCRE2_ERROR, WFR_STATUS_CONDITION_PCRE2_NO_MATCH, or
	 * WFR_STATUS_CONDITION_PCRE2_DONE is returned.
	 */

	do {
		switch (WFR_STATUS_CONDITION(
					WinfrippPcre2MatchGlobal(
						&pcre2_state, &match_begin, &match_end)))
		{
		case WFR_STATUS_CONDITION_PCRE2_ERROR:
			WFR_DEBUGF("error %d trying to match any URL(s) in line `%S'", pcre2_state.last_error, line_w);
			breakfl = true; break;
		case WFR_STATUS_CONDITION_PCRE2_NO_MATCH:
			breakfl = true; break;

		/*
		 * Given a non-zero length match the range of which is intersected by x,
		 * attempt to duplicate the corresponding substring and return it, its
		 * size in bytes, and the beginning and end positions thereof. Given
		 * failure, return failure.
		 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
		case WFR_STATUS_CONDITION_PCRE2_DONE:
			breakfl = true;
#pragma GCC diagnostic pop
		case WFR_STATUS_CONDITION_PCRE2_CONTINUE:
			if (((size_t)x >= match_begin) && ((size_t)x <= match_end)) {
				match_len = match_end - match_begin;
				if (match_len > 0) {
					WFR_DEBUGF("URL `%*.*S' matches regular expression", match_len, match_len, &line_w[match_begin]);
					if (!(*phover_url_w = WfrWcsNDup(&line_w[match_begin], match_len))) {
						sfree(line_w); return FALSE;
					} else {
						pbegin->x = match_begin, pend->x = match_end;
						pbegin->y = y, pend->y = y;
						*phover_url_w_size = match_len + 1; breakfl = true; return TRUE;
					}
				} else {
					breakfl = true;
				}
			}
			break;
		}
	} while (!breakfl);

	/*
	 * Free line_w and implictly return failure.
	 */

	sfree(line_w);
	return FALSE;
}

static void
winfripp_urls_config_panel_modifier_key_handler(
	dlgcontrol *	ctrl,
	dlgparam *		dlg,
	void *			data,
	int				event
	)
{
	Conf *	conf = (Conf *)data;
	int		id;


	switch (event) {
	case EVENT_REFRESH:
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);
		dlg_listbox_addwithid(ctrl, dlg, "Ctrl", WINFRIPP_URLS_MODIFIER_KEY_CTRL);
		dlg_listbox_addwithid(ctrl, dlg, "Alt", WINFRIPP_URLS_MODIFIER_KEY_ALT);
		dlg_listbox_addwithid(ctrl, dlg, "Right Ctrl", WINFRIPP_URLS_MODIFIER_KEY_RIGHT_CTRL);
		dlg_listbox_addwithid(ctrl, dlg, "Right Alt/AltGr", WINFRIPP_URLS_MODIFIER_KEY_RIGHT_ALT);

		switch (conf_get_int(conf, CONF_frip_urls_modifier_key)) {
		case WINFRIPP_URLS_MODIFIER_KEY_CTRL:
			dlg_listbox_select(ctrl, dlg, 0); break;
		case WINFRIPP_URLS_MODIFIER_KEY_ALT:
			dlg_listbox_select(ctrl, dlg, 1); break;
		case WINFRIPP_URLS_MODIFIER_KEY_RIGHT_CTRL:
			dlg_listbox_select(ctrl, dlg, 2); break;
		case WINFRIPP_URLS_MODIFIER_KEY_RIGHT_ALT:
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
		winfripp_urls_reconfig_modifier_key(conf);
		break;
	}
}

static void
winfripp_urls_config_panel_modifier_shift_handler(
	dlgcontrol *	ctrl,
	dlgparam *		dlg,
	void *			data,
	int				event
	)
{
	Conf *	conf = (Conf *)data;
	int		id;


	switch (event) {
	case EVENT_REFRESH:
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);
		dlg_listbox_addwithid(ctrl, dlg, "None", WINFRIPP_URLS_MODIFIER_SHIFT_NONE);
		dlg_listbox_addwithid(ctrl, dlg, "Shift", WINFRIPP_URLS_MODIFIER_SHIFT_LSHIFT);
		dlg_listbox_addwithid(ctrl, dlg, "Right Shift", WINFRIPP_URLS_MODIFIER_SHIFT_RSHIFT);

		switch (conf_get_int(conf, CONF_frip_urls_modifier_shift)) {
		case WINFRIPP_URLS_MODIFIER_SHIFT_NONE:
			dlg_listbox_select(ctrl, dlg, 0); break;
		case WINFRIPP_URLS_MODIFIER_SHIFT_LSHIFT:
			dlg_listbox_select(ctrl, dlg, 1); break;
		case WINFRIPP_URLS_MODIFIER_SHIFT_RSHIFT:
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
		winfripp_urls_reconfig_modifier_shift(conf);
		break;
	}
}

static WinFripReturn
winfripp_urls_reconfig(
	Conf *	conf
	)
{
	static char		dlg_caption[96], dlg_text[256];
	int				re_errorcode = 0;
	PCRE2_SIZE		re_erroroffset = 0;
	char *			spec;
	size_t			spec_len;
	wchar_t *		spec_w;


	/*
	 * Obtain/update new configuration values. Reject empty regular expression
	 * strings. Convert the regular expression string to UTF-16. Release
	 * previously allocated pcre2 code resources, if any, compile the new
	 * regular expression string into pcre2 code and create a match data block,
	 * if necessary. On failure to do so, attempt to obtain the error message
	 * corresponding to the error produced during compilation.
	 */

	spec = conf_get_str(conf, CONF_frip_urls_regex); spec_len = strlen(spec);
	winfripp_urls_reconfig_modifier_key(conf);
	winfripp_urls_reconfig_modifier_shift(conf);

	if (spec_len == 0) {
		WFR_SNPRINTF(dlg_caption, sizeof(dlg_caption), "Error compiling clickable URL regex");
		WFR_SNPRINTF(dlg_text, sizeof(dlg_caption), "Regular expressions must not be empty.");
		goto fail;
	} else if (WFR_STATUS_FAILURE(WfrToWcsDup(spec, spec_len + 1, &spec_w))) {
		WFR_DEBUG_FAIL();
		WFR_SNPRINTF(dlg_caption, sizeof(dlg_caption), "Error compiling clickable URL regex");
		WFR_SNPRINTF(dlg_text, sizeof(dlg_caption), "Internal memory allocation error on calling WfrToWcsDup()");
		goto fail;
	} else {
		if (winfripp_re_code) {
			pcre2_code_free(winfripp_re_code); winfripp_re_code = NULL;
		}
		winfripp_re_code = pcre2_compile(
			spec_w,
			PCRE2_ANCHORED | PCRE2_ZERO_TERMINATED,
			0, &re_errorcode, &re_erroroffset, NULL);

		if (winfripp_re_code) {
			sfree(spec_w);
			winfripp_re_md = pcre2_match_data_create(1, NULL);
			return WINFRIP_RETURN_CONTINUE;
		} else {
			ZeroMemory(winfripp_re_error_message, sizeof(winfripp_re_error_message));
			pcre2_get_error_message(
				re_errorcode,
				winfripp_re_error_message,
				sizeof(winfripp_re_error_message) / sizeof(winfripp_re_error_message[0]));

			WFR_SNPRINTF(dlg_caption, sizeof(dlg_caption), "Error compiling clickable URL regex");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat="
			WFR_SNPRINTF(dlg_text, sizeof(dlg_text),
					 "Error in regex %S at offset %llu: %S",
					 spec_w, re_erroroffset, winfripp_re_error_message);
#pragma GCC diagnostic pop

			goto fail;
		}
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
	switch (MessageBoxA(NULL, dlg_caption, dlg_text, MB_CANCELTRYCONTINUE | MB_ICONERROR)) {
	default:
	case IDCANCEL:
		sfree(spec_w);
		return WINFRIP_RETURN_CANCEL;
	case IDCONTINUE:
		sfree(spec_w);
		return WINFRIP_RETURN_CONTINUE;
	case IDTRYAGAIN:
		sfree(spec_w);
		return WINFRIP_RETURN_RETRY;
	}
}

static void
winfripp_urls_reconfig_modifier_key(
	Conf *	conf
	)
{
	switch (conf_get_int(conf, CONF_frip_urls_modifier_key)) {
	case WINFRIPP_URLS_MODIFIER_KEY_CTRL:
		winfripp_urls_modifier = VK_CONTROL; break;
	case WINFRIPP_URLS_MODIFIER_KEY_ALT:
		winfripp_urls_modifier = VK_MENU; break;
	case WINFRIPP_URLS_MODIFIER_KEY_RIGHT_CTRL:
		winfripp_urls_modifier = VK_RCONTROL; break;
	case WINFRIPP_URLS_MODIFIER_KEY_RIGHT_ALT:
		winfripp_urls_modifier = VK_RMENU; break;
	default:
		winfripp_urls_modifier = 0; WFR_DEBUG_FAIL(); break;
	}
}

static void
winfripp_urls_reconfig_modifier_shift(
	Conf *	conf
	)
{
	switch (conf_get_int(conf, CONF_frip_urls_modifier_shift)) {
	case WINFRIPP_URLS_MODIFIER_SHIFT_NONE:
		winfripp_urls_modifier_shift = 0; break;
	case WINFRIPP_URLS_MODIFIER_SHIFT_LSHIFT:
		winfripp_urls_modifier_shift = VK_LSHIFT; break;
	case WINFRIPP_URLS_MODIFIER_SHIFT_RSHIFT:
		winfripp_urls_modifier_shift = VK_RSHIFT; break;
	default:
		winfripp_urls_modifier_shift = 0; WFR_DEBUG_FAIL(); break;
	}
}

static bool
winfripp_urls_state_match(
	int		x,
	int		y
	)
{
	return (WfrpIsVKeyDown(winfripp_urls_modifier)
	&&    ((winfripp_urls_modifier_shift != 0)
	 ?     WfrpIsVKeyDown(winfripp_urls_modifier_shift)
	 :     true)
	&&  (y == winfripp_urls_begin.y)
	&&  (y == winfripp_urls_end.y)
	&&  (x >= winfripp_urls_begin.x)
	&&  (x  < winfripp_urls_end.x));
}

static void
winfripp_urls_state_reset(
	Terminal *	term,
	bool		update_term
	)
{
	winfripp_urls_begin.x = winfripp_urls_begin.y = 0;
	winfripp_urls_end.x = winfripp_urls_end.y = 0;
	if (winfripp_urls_buf_w) {
		sfree(winfripp_urls_buf_w);
		winfripp_urls_buf_w = NULL, winfripp_urls_buf_w_size = 0;
	}
	winfripp_urls_state = WINFRIPP_URLS_STATE_NONE;
	if (update_term) {
		term_update(term);
	}
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
winfripp_urls_config_panel(
	struct controlbox *		b
	)
{
	struct controlset *s_browser, *s_input, *s_re, *s_visual;


	WFR_DEBUG_ASSERT(b);

	/*
	 * The Frippery: URLs panel.
	 */

	ctrl_settitle(b, "Frippery/URLs", "Configure pointless frippery: clickable URLs");

	/*
	 * The Frippery: URLs: Visual behaviour controls box.
	 */

	s_visual = ctrl_getset(b, "Frippery/URLs", "frip_urls_visual", "Visual behaviour");
	ctrl_checkbox(s_visual, "Underline hyperlinks on highlight",
				  'u', WINFRIPP_HELP_CTX,
				  conf_checkbox_handler, I(CONF_frip_urls_underline_onhl));
	ctrl_checkbox(s_visual, "Apply reverse video to hyperlinks on click",
				  'v', WINFRIPP_HELP_CTX,
				  conf_checkbox_handler, I(CONF_frip_urls_revvideo_onclick));

	/*
	 * The Frippery: URLs: Input behaviour controls box.
	 */

	s_input = ctrl_getset(b, "Frippery/URLs", "frip_urls_input", "Input behaviour");
	ctrl_droplist(s_input, "Modifier key:",
				  'm', 35, WINFRIPP_HELP_CTX,
				  winfripp_urls_config_panel_modifier_key_handler, P(NULL));
	ctrl_droplist(s_input, "Shift key:",
				  's', 35, WINFRIPP_HELP_CTX,
				  winfripp_urls_config_panel_modifier_shift_handler, P(NULL));

	/*
	 * The Frippery: URLs: Regular expression controls box.
	 */

	s_re = ctrl_getset(b, "Frippery/URLs", "frip_urls_regex", "Regular expression");
	ctrl_editbox(s_re, NULL,
				 'r', 100, WINFRIPP_HELP_CTX,
				 conf_editbox_handler, I(CONF_frip_urls_regex), ED_STR);
	ctrl_text(s_re, "Regexes are matched against whole lines and may contain/match on whitespaces, etc.",
			  WINFRIPP_HELP_CTX);

	/*
	 * The Frippery: URLs: Browser controls box.
	 */

	s_browser = ctrl_getset(b, "Frippery/URLs", "frip_urls_browser", "Browser");
	ctrl_checkbox(s_browser, "Default application associated with \"open\" verb",
				  'd', WINFRIPP_HELP_CTX,
				  conf_checkbox_handler, I(CONF_frip_urls_browser_default));
	ctrl_text(s_browser, "Pathname to browser application:", WINFRIPP_HELP_CTX);
	ctrl_editbox(s_browser, NULL,
				 'p', 100, WINFRIPP_HELP_CTX,
				 conf_editbox_handler, I(CONF_frip_urls_browser_pname_verb), ED_STR);
}

/*
 * Public subroutines
 */

WinFripReturn
winfrip_urls_op(
	WinFripUrlsOp		op,
	Conf *				conf,
	HWND				hwnd,
	UINT				message,
	unsigned long *		tattr,
	Terminal *			term,
	WPARAM				wParam,
	int					x,
	int					y
	)
{
	WinFripReturn rc;


	(void)hwnd;
	(void)wParam;

	switch (op) {

	/*
	 * Given WINFRIP_URLS_OP_DRAW from terminal.c:do_paint(), apply either of
	 * the video reverse, given WINFRIPP_URLS_STATE_CLICK, or underline, given
	 * WINFRIPP_URLS_STATE_SELECT, attributes, unless configured not to do so
	 * by the user, to on-screen characters that fall into the range of the URL
	 * presently being processed, taking the scrolling displacement into
	 * account; note that this does not mutate the terminal's buffer of text on
	 * real screen.
	 */

	case WINFRIP_URLS_OP_DRAW:
		if (winfripp_urls_posle(winfripp_urls_begin, x, y - term->disptop) &&
			winfripp_urls_poslt(x, y - term->disptop, winfripp_urls_end))
		{
			switch (winfripp_urls_state) {
			case WINFRIPP_URLS_STATE_CLICK:
				if (conf_get_bool(conf, CONF_frip_urls_revvideo_onclick)) {
					*tattr |= ATTR_REVERSE;
				}
				break;

			case WINFRIPP_URLS_STATE_SELECT:
				if (conf_get_bool(conf, CONF_frip_urls_underline_onhl)) {
					*tattr |= ATTR_UNDER;
				}
				break;

			default:
				WFR_DEBUG_FAIL();
			}
		}
		return WINFRIP_RETURN_CONTINUE;

	/*
	 * Given WINFRIP_URLS_OP_FOCUS_KILL interrupting an URL operation, reset
	 * the URL operation state and update the terminal.
	 */

	case WINFRIP_URLS_OP_FOCUS_KILL:
		switch (winfripp_urls_state) {
		case WINFRIPP_URLS_STATE_SELECT:
			winfripp_urls_state_reset(term, true); break;
		default:
			break;
		}
		break;

	case WINFRIP_URLS_OP_MOUSE_BUTTON_EVENT:
		switch (winfripp_urls_state) {

		/*
		 * Given WINFRIPP_URLS_STATE_SELECT, a LMB click event, and
		 * satisfaction of the constraint comprised by {x,y} and the configured
		 * modifier, and optionally shift modifier, keys held down at click
		 * time, set state to WINFRIPP_URLS_STATE_CLICK, update the terminal
		 * to apply reverse video to the URL selected, unless configured not do
		 * so by the user, and execute the "open" verb or the browser
		 * application, as configured by the user on the URL selected.
		 * Subsequently, reset the URL operation state and update the terminal.
		 */

		case WINFRIPP_URLS_STATE_SELECT:
			if ((message == WM_LBUTTONDOWN) && winfripp_urls_state_match(x, y)) {
				winfripp_urls_state = WINFRIPP_URLS_STATE_CLICK; term_update(term);
				if (conf_get_bool(conf, CONF_frip_urls_browser_default)) {
					WFR_DEBUGF("ShellExecuteW(\"open\", `%S')", winfripp_urls_buf_w);
					ShellExecuteW(NULL, L"open", winfripp_urls_buf_w, NULL, NULL, SW_SHOWNORMAL);
				} else {
					WFR_DEBUG_ASSERT(winfripp_urls_app_w);
					WFR_DEBUGF("ShellExecuteW(`%S', `%S')", winfripp_urls_app_w, winfripp_urls_buf_w);
					ShellExecuteW(NULL, NULL, winfripp_urls_app_w, winfripp_urls_buf_w, NULL, SW_SHOWNORMAL);
				}
				rc = WINFRIP_RETURN_BREAK;
			} else {
				rc = WINFRIP_RETURN_CONTINUE;
			}
			winfripp_urls_state_reset(term, true);
			return rc;

		default:
			break;
		}
		return WINFRIP_RETURN_CONTINUE;

	case WINFRIP_URLS_OP_MOUSE_MOTION_EVENT:
		switch (winfripp_urls_state) {

		/*
		 * Given WINFRIPP_URLS_STATE_NONE and satisfaction of the constraint
		 * comprised by the configured modifier, and optionally shift modifier,
		 * keys held down at mouse motion event time, attempt to match the URL
		 * pointed into by the mouse cursor and, given a match, set state to
		 * WINFRIPP_URLS_STATE_SELECT and update the terminal.
		 */

		case WINFRIPP_URLS_STATE_NONE:
			if (WfrpIsVKeyDown(winfripp_urls_modifier)
			&& ((winfripp_urls_modifier_shift != 0)
			 ? WfrpIsVKeyDown(winfripp_urls_modifier_shift)
			 : true))
			{
				if (winfripp_urls_get(
						term, &winfripp_urls_begin, &winfripp_urls_end,
						&winfripp_urls_buf_w, &winfripp_urls_buf_w_size,
						x, y)) {
					winfripp_urls_state = WINFRIPP_URLS_STATE_SELECT;
					term_update(term);
				}
			}
			break;

		/*
		 * Given WINFRIPP_URLS_STATE_SELECT, enforce the constraint that the
		 * configured modifier, and optionally shift modifier, keys must remain
		 * held down and that the mouse pointer must remain pointing into the
		 * region described by winfripp_urls_{begin,end}.{x,y}. If this
		 * constraint is violated, reset the URL operation state and update the
		 * terminal.
		 */

		case WINFRIPP_URLS_STATE_SELECT:
			if (!winfripp_urls_state_match(x, y)) {
				winfripp_urls_state_reset(term, true);
			}
			break;

		default:
			break;
		}
		return WINFRIP_RETURN_CONTINUE;

	case WINFRIP_URLS_OP_RECONFIG:
		return winfripp_urls_reconfig(conf);

	default:
		WFR_DEBUG_FAIL();
		return WINFRIP_RETURN_CONTINUE;
	}

	return WINFRIP_RETURN_CONTINUE;
}

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
