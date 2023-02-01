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
static wchar_t *		WffupAppW = NULL;

/*
 * Zero-based inclusive beginning and exclusive end terminal buffer coordinates
 * of URL during URL matching and selecting operations
 */
static pos			WffupMatchBegin = {0, 0};
static pos			WffupMatchEnd = {0, 0};
static pos			WffupSelectBegin = {0, 0};
static pos			WffupSelectEnd = {0, 0};

/*
 * UTF-16 encoded URL buffer and buffer size in bytes during URL operations
 * or NULL/0, resp.
 */
static wchar_t *		WffupBufW = NULL;
static size_t			WffupBufW_size = 0;

/*
 * Windows base API GetKeyState() virtual keys corresponding to the URL mouse
 * motion, URL extending/shrinking, and LMB click modifier and shift modifier
 * keys configured by the user.
 */
static int			WffupModifier = 0;
static int			WffupModifierExtendShrink = 0;
static int			WffupModifierShiftState = 0;

/*
 * PCRE2 regular expression compiled code or NULL, error message buffer, and
 * match data block or NULL
 */
static pcre2_code *		WffupReCode = NULL;
static wchar_t			WffupReErrorMessage[256] = {0,};
static pcre2_match_data *	WffupReMd = NULL;

/*
 * URL operation state (WFFUP_STATE_{NONE,CLICK,SELECT})
 */
static WffupState		WffupStateCurrent = WFFUP_STATE_NONE;

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

static WfrStatus	WffupGet(Terminal *term, pos *pbegin, pos *pend, wchar_t **phover_url_w, size_t *phover_url_w_size, pos begin, pos end);
static WfrStatus	WffupGetTermLine(Terminal *term, int y, wchar_t **pline_w, size_t *pline_w_len);
static WfrStatus	WffupGetTermLines(Terminal *term, pos begin, pos end, wchar_t **pline_w);

static bool		WffupMatchState(int x, int y);
static void		WffupResetState(Terminal *term, bool update_term);

static int		WffOpClick(Conf *conf, UINT message, Terminal *term, int x, int y);
static int		WffOpDraw(Conf *conf, unsigned long *tattr, Terminal *term, int x, int y);
static int		WffOpSelect(Terminal *term, int x, int );
static int		WffOpSelectExtendShrink(Terminal *term, WPARAM wParam);

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

		if (WFR_STATUS_SUCCESS(status = WfrToWcsDup(
				browser_verb, browser_verb_len + 1, &browser_verb_w)))
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
	} else if (WFR_STATUS_FAILURE(WfrToWcsDup(spec, spec_len + 1, &spec_w))) {
		WFR_DEBUG_FAIL();
		snprintf(dlg_caption, sizeof(dlg_caption), "Error compiling clickable URL regex");
		snprintf(dlg_text, sizeof(dlg_caption), "Internal memory allocation error on calling WfrToWcsDup()");
		goto fail;
	} else {
		if (WffupReCode) {
			pcre2_code_free(WffupReCode); WffupReCode = NULL;
		}
		WffupReCode = pcre2_compile(
			spec_w,
			PCRE2_ANCHORED | PCRE2_ZERO_TERMINATED,
			0, &re_errorcode, &re_erroroffset, NULL);

		if (WffupReCode) {
			WFR_FREE(spec_w);
			WffupReMd = pcre2_match_data_create(1, NULL);
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


static WfrStatus
WffupGet(
	Terminal *	term,
	pos *		pbegin,
	pos *		pend,
	wchar_t **	phover_url_w,
	size_t *	phover_url_w_size,
	pos		begin,
	pos		end
	)
{
	bool		donefl;
	wchar_t *	line_w = NULL;
	size_t		match_begin, match_end, match_len;
	Wfp2MGState	pcre2_state;
	WfrStatus	status;


	/*
	 * Fail given non-initialised pcre2 regular expression {code,match data
	 * block}, whether due to failure to initialise or an invalid regular
	 * expression provided by the user.
	 */

	if (!WffupReCode || !WffupReMd) {
		return WFR_STATUS_FROM_ERRNO1(EINVAL);
	} else if (WFR_STATUS_SUCCESS(status = WffupGetTermLines(
			term, begin, end, &line_w)))
	{
		Wfp2Init(
			&pcre2_state, WffupReCode,
			wcslen(line_w), WffupReMd, line_w);
	}

	/*
	 * Iteratively attempt to match the regular expression and UTF-16 encoded
	 * terminal buffer string described by pcre2_state until either a non-zero
	 * length match comprising a range intersected by the x coordinate parameter
	 * is found or until either WFR_STATUS_CONDITION_PCRE2_ERROR,
	 * WFR_STATUS_CONDITION_PCRE2_NO_MATCH, or WFR_STATUS_CONDITION_PCRE2_DONE
	 * is returned.
	 */

	donefl = false;
	while (WFR_STATUS_SUCCESS(status) && !donefl) {
		switch (WFR_STATUS_CONDITION(status = Wfp2MatchGlobal(
				&pcre2_state, &match_begin, &match_end)))
		{
		case WFR_STATUS_CONDITION_PCRE2_ERROR:
			WFR_DEBUGF("error %d trying to match any URL(s) in line `%S'", pcre2_state.last_error, line_w);
			break;
		case WFR_STATUS_CONDITION_PCRE2_NO_MATCH:
			break;

		/*
		 * Given a non-zero length match the range of which is intersected by {begin,end},
		 * attempt to duplicate the corresponding substring and return it, its size in bytes,
		 * and the beginning and end positions thereof. Given failure, return failure.
		 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
		case WFR_STATUS_CONDITION_PCRE2_DONE:
			donefl = true;
#pragma GCC diagnostic pop

		case WFR_STATUS_CONDITION_PCRE2_CONTINUE:
			if (((size_t)begin.x >= match_begin) && ((size_t)begin.x <= match_end)) {
				match_len = match_end - match_begin;
				if (match_len > 0) {
					WFR_DEBUGF("URL `%*.*S' matches regular expression", match_len, match_len, &line_w[match_begin]);
					if (!(*phover_url_w = WfrWcsNDup(&line_w[match_begin], match_len))) {
						status = WFR_STATUS_FROM_ERRNO();
					} else {
						pbegin->x = match_begin;
						if (pbegin->x > term->cols) {
							pbegin->y = begin.y + (pbegin->x / term->cols);
							pbegin->x %= term->cols;
						} else {
							pbegin->y = begin.y;
						}

						pend->x = match_end;
						if (pend->x > term->cols) {
							pend->y = begin.y + (pend->x / term->cols);
							pend->x %= term->cols;
						} else {
							pend->y = end.y;
						}

						donefl = true;
						*phover_url_w_size = match_len + 1;
						status = WFR_STATUS_CONDITION_SUCCESS;
					}
				} else {
					status = WFR_STATUS_FROM_ERRNO1(EINVAL);
				}
			}
			break;
		}
	}

	/*
	 * Free line_w and implictly return failure.
	 */

	WFR_FREE_IF_NOTNULL(line_w);

	return status;
}

static WfrStatus
WffupGetTermLine(
	Terminal *	term,
	int		y,
	wchar_t **	pline_w,
	size_t *	pline_w_len
	)
{
	size_t		idx_in, idx_out;
	termline *	line = NULL;
	wchar_t *	line_w = NULL;
	size_t		line_w_len;
	size_t		rtl_idx, rtl_len;
	int		rtl_start;
	WfrStatus	status;
	wchar_t		wch;


	/*
	 * Reject invalid y coordinates falling outside of term->{cols,rows}. Fail given
	 * failure to allocate sufficient memory to the line_w buffer of term->cols + 1
	 * units of wchar_t.
	 */

	if (y >= term->rows) {
		return WFR_STATUS_FROM_ERRNO1(EINVAL);
	} else {
		line_w_len = term->cols;
		if (WFR_STATUS_FAILURE(status = WFR_RESIZE(
				line_w, line_w_len, line_w_len + 1, wchar_t)))
		{
			return status;
		} else {
			line = term->disptext[y];
		}
	}

	/*
	 * Iteratively copy UTF-16 wide characters from the terminal's
	 * buffer of text on real screen into line_w. If a Right-To-Left
	 * character or continuous sequence thereof is encountered, it is
	 * iteratively copied in its entirety from right to left. If a
	 * direct-to-font mode character using the D800 hack is encountered,
	 * only its least significant 8 bits are copied.
	 */

	for (idx_in = idx_out = 0, rtl_len = 0, rtl_start = -1; idx_in < (size_t)term->cols; idx_in++) {
		wch = line->chars[idx_in].chr;

		switch (rtl_start) {
		/*
		 * Non-RTL character
		 */
		case -1:
			if (DIRECT_CHAR(wch) || DIRECT_FONT(wch) || !is_rtl(wch)) {
				if (DIRECT_CHAR(wch) || DIRECT_FONT(wch)) {
					wch &= 0xFF;
				}
				line_w[idx_out] = wch; idx_out++;
			} else if (is_rtl(wch)) {
				rtl_start = idx_in; rtl_len = 1;
			}
			break;

		/*
		 * RTL character or continuous sequence thereof
		 */
		default:
			if (DIRECT_CHAR(wch) || DIRECT_FONT(wch) || !is_rtl(wch)) {
				for (rtl_idx = 0; rtl_idx < rtl_len; rtl_idx++) {
					wch = line->chars[rtl_start + (rtl_len - 1 - rtl_idx)].chr;
					line_w[idx_out] = wch;
					idx_out++;
				}
				rtl_start = -1; rtl_len = 0;
				wch = line->chars[idx_in].chr;
				if (DIRECT_CHAR(wch) || DIRECT_FONT(wch)) {
					wch &= 0xFF;
				}
				line_w[idx_out] = wch; idx_out++;
			} else if (is_rtl(wch)) {
				rtl_len++;
			}
			break;
		}
	}

	/*
	 * Given a remnant of a continuous sequence of RTL characters,
	 * iteratively copy it from right to left.
	 */

	if (rtl_start != -1) {
		for (rtl_idx = 0; rtl_idx < rtl_len; rtl_idx++) {
			wch = line->chars[rtl_start + (rtl_len - 1 - rtl_idx)].chr;
			line_w[idx_out] = wch;
			idx_out++;
		}
		rtl_start = -1; rtl_len = 0;
	}

	/*
	 * NUL-terminate the line_w buffer, write out parameters, and return success.
	 */

	line_w[term->cols] = L'\0';
	*pline_w = line_w, *pline_w_len = line_w_len;

	return WFR_STATUS_CONDITION_SUCCESS;
}

static WfrStatus
WffupGetTermLines(
	Terminal *	term,
	pos		begin,
	pos		end,
	wchar_t **	pline_w
	)
{
	wchar_t *	line_new_w;
	size_t		line_new_w_len;
	wchar_t *	line_w = NULL;
	bool		line_w_initfl;
	size_t		line_w_size;
	WfrStatus	status;


	/*
	 * Reject invalid {begin,end}.{x,y} * coordinates, be they negative or
	 * falling outside of term->{cols,rows}
	 */

	if ((begin.x < 0) || (begin.x >= term->cols)
	||  (begin.y < 0) || (begin.y >= term->rows)
	||  (end.x < 0)   || (end.x >= term->cols)
	||  (end.y < 0)   || (end.y >= term->rows))
	{
		status = WFR_STATUS_FROM_ERRNO1(ERANGE);
	} else {
		/*
		 * Iteratively get and concatenate entire lines from the terminal's
		 * buffer of text on real screen specified by the coordinates {begin,end}.{x,y}.
		 */

		line_w_initfl = true;
		line_w_size = 0;
		status = WFR_STATUS_CONDITION_SUCCESS;

		for (int y = begin.y; y <= end.y; y++) {
			if (WFR_STATUS_FAILURE(status = WffupGetTermLine(
					term, y, &line_new_w, &line_new_w_len)))
			{
				break;
			} else if (WFR_STATUS_FAILURE(status = WFR_RESIZE(
					line_w, line_w_size,
					line_w_size ? (line_w_size + line_new_w_len) : (line_new_w_len + 1),
					wchar_t)))
			{
				WFR_FREE(line_new_w);
				break;
			} else {
				if (line_w_initfl) {
					wcscpy(line_w, line_new_w);
					line_w_initfl = false;
				} else {
					wcscat(line_w, line_new_w);
				}
				WFR_FREE(line_new_w);
			}
		}
	}

	if (WFR_STATUS_SUCCESS(status)) {
		*pline_w = line_w;
	} else {
		WFR_FREE_IF_NOTNULL(line_w);
	}

	return status;
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
WffOpClick(
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
				WFR_STATUS_FROM_WINDOWS(),
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
WffOpDraw(
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
WffOpSelect(
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
		if (WFR_STATUS_SUCCESS(status = WffupGet(
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
WffOpSelectExtendShrink(
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

		if (WFR_STATUS_SUCCESS(status = WffupGet(
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
		return WffOpDraw(conf, tattr, term, x, y);

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
			return WffOpClick(conf, message, term, x, y);
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
			return WffOpSelect(term, x, y);

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
			return WffOpSelectExtendShrink(term, wParam);
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
