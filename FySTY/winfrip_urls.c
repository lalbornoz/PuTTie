/*
 * winfrip_urls.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "terminal.h"
#include "FySTY/winfrip.h"
#include "FySTY/winfrip_priv.h"

#include <shlwapi.h>

/*
 * Preprocessor macros
 */

/*
 * XXX document
 */
#define winfripp_urls_posle(p1,px,py)	\
	((p1).y < (py) || ((p1).y == (py) && (p1).x <= (px)))
#define winfripp_urls_poslt(px,py,p2)	\
	((py) < (p2).y || ((py) == (p2).y && (px) < (p2).x))

/*
 * {External,Static} variables
 */

/*
 * XXX document
 */
static pos winfripp_urls_end = {0, 0};
static pos winfripp_urls_start = {0, 0};
static WinFrippUrlsState winfripp_urls_state = WINFRIPP_URLS_STATE_NONE;

/*
 * XXX document
 */
static wchar_t *winfripp_urls_buf_w = NULL;
static size_t winfripp_urls_buf_w_size = 0;
static wchar_t *winfripp_urls_url_w = NULL;

/*
 * XXX document
 */
static char *winfripp_urls_match_spec_conf = NULL;
static wchar_t *winfripp_urls_match_spec_w = NULL;
static wchar_t **winfripp_urls_matchv_w = NULL;
static size_t winfripp_urls_matchc_w = 0;

/*
 * Private subroutine prototypes
 */

static BOOL winfripp_init_urls_get_endstart(Terminal *term, pos *pend, pos *pstart, int x, int y);
static BOOL winfripp_init_urls_get_matchv(Conf *conf, char **pmatch_spec_conf, wchar_t **pmatch_spec_w, size_t *pmatchc_w, wchar_t ***pmatchv_w);
static BOOL winfripp_init_urls_get_url(pos hover_end, pos hover_start, Terminal *term, wchar_t **phover_url_w, size_t *phover_url_w_size);
static wchar_t *winfripp_init_urls_unnest(Conf *conf, wchar_t *url_w);

/*
 * Private subroutines
 */

static BOOL winfripp_init_urls_get_endstart(Terminal *term, pos *pend, pos *pstart, int x, int y)
{
    int x_end, x_start;
    wchar_t wch;


    WINFRIPP_DEBUG_ASSERT(term);
    WINFRIPP_DEBUG_ASSERT(pend);
    WINFRIPP_DEBUG_ASSERT(pstart);

    /*
     * XXX document
    */
    if ((x >= term->cols) || (y >= term->rows)) {
	return FALSE;
    }

    /*
     * XXX document
     */
    for (x_start = x; x_start >= 0; x_start--) {
	wch = term->disptext[y]->chars[x_start].chr;
	if (DIRECT_FONT(wch)) {
	    wch &= 0xFF;
	}
	if (wch == 0x20) {
	    x_start++; break;
	} else if (x_start == 0) {
	    break;
	}
    }
    if ((x_start < 0) || (x_start > x)) {
	return FALSE;
    }

    /*
     * XXX document
     */
    for (x_end = x; x_end < term->cols; x_end++) {
	wch = term->disptext[y]->chars[x_end].chr;
	if (DIRECT_FONT(wch)) {
	    wch &= 0xFF;
	}
	if (wch == 0x20) {
	    break;
	}
    }
    if (x_end == x) {
	return FALSE;
    }

    /*
     * XXX document
     */
    pend->x = x_end;
    pend->y = pstart->y = y;
    pstart->x = x_start;
    return TRUE;
}

static BOOL winfripp_init_urls_get_matchv(Conf *conf, char **pmatch_spec_conf, wchar_t **pmatch_spec_w, size_t *pmatchc_w, wchar_t ***pmatchv_w)
{
    char *match_spec_conf;
    size_t match_spec_conf_len;
    wchar_t *match_spec_w, *p, *q;
    size_t new_matchc_w;
    wchar_t **new_matchv_w;
    size_t nitem, nitems;


    WINFRIPP_DEBUG_ASSERT(pmatch_spec_conf);
    WINFRIPP_DEBUG_ASSERT(pmatchc_w);
    WINFRIPP_DEBUG_ASSERT(pmatchv_w);

    /*
     * XXX document
     */
    match_spec_conf = conf_get_str(conf, CONF_frip_urls_match_spec);
    match_spec_conf_len = strlen(match_spec_conf);
    if (!match_spec_conf_len) {
	WINFRIPP_DEBUG_FAIL();
	return FALSE;
    } else if (*pmatch_spec_conf != match_spec_conf) {
	if (!winfripp_towcsdup(match_spec_conf, match_spec_conf_len + 1, &match_spec_w)) {
	    WINFRIPP_DEBUG_FAIL();
	    return FALSE;
	}
    } else {
	return TRUE;
    }

    /*
     * XXX document
     */
    for (nitems = 1, p = match_spec_w; *p; p++) {
	if (*p == L';') {
	    nitems++;
	}
    }
    if (!(new_matchv_w = snewn(nitems, wchar_t *))) {
	sfree(match_spec_w);
	WINFRIPP_DEBUG_FAIL();
	return FALSE;
    } else {
	ZeroMemory(new_matchv_w, nitems * sizeof(*new_matchv_w));
	new_matchc_w = nitems;
    }

    /*
     * XXX document
     */
    for (nitem = 0, p = q = match_spec_w; nitem < nitems; q++) {
	if (*q == L';') {
	    new_matchv_w[nitem] = p; *q = L'\0'; p = q + 1; nitem++;
	} else if (*q == L'\0') {
	    new_matchv_w[nitem] = p; break;
	}
    }

    /*
     * XXX document
     */
    if (*pmatch_spec_w) {
	 sfree(*pmatch_spec_w);
    }
    if (*pmatchv_w) {
	 sfree(*pmatchv_w);
    }
    *pmatch_spec_conf = match_spec_conf;
    *pmatch_spec_w = match_spec_w;
    *pmatchc_w = new_matchc_w;
    *pmatchv_w = new_matchv_w;
    return TRUE;
}

static BOOL winfripp_init_urls_get_url(pos hover_end, pos hover_start, Terminal *term, wchar_t **phover_url_w, size_t *phover_url_w_size)
{
    size_t hover_len, idx_in, idx_out, new_buf_w_size;
    wchar_t *new_buf_w;
    size_t rtl_idx, rtl_len;
    int rtl_start;
    wchar_t wch;


    WINFRIPP_DEBUG_ASSERT(phover_url_w);
    WINFRIPP_DEBUG_ASSERT(phover_url_w_size);

    /*
     * XXX document
    */
    hover_len = hover_end.x - hover_start.x;
    new_buf_w_size = (hover_len + 1) * sizeof(**phover_url_w);
    if (new_buf_w_size > *phover_url_w_size) {
	if ((new_buf_w = sresize(*phover_url_w, new_buf_w_size, wchar_t))) {
	    *phover_url_w = new_buf_w;
	    *phover_url_w_size = new_buf_w_size;
	} else {
	    WINFRIPP_DEBUG_FAIL();
	    return FALSE;
	}
    }

    /*
     * XXX document
    */
    for (idx_in = idx_out = 0, rtl_len = 0, rtl_start = -1; idx_in < hover_len; idx_in++) {
	wch = term->disptext[hover_start.y]->chars[hover_start.x + idx_in].chr;
	if (rtl_start == -1) {
	    if (DIRECT_FONT(wch) || !is_rtl(wch)) {
		if (DIRECT_FONT(wch)) {
		    wch &= 0xFF;
		}
		(*phover_url_w)[idx_out] = wch; idx_out++;
	    } else if (is_rtl(wch)) {
		rtl_start = idx_in; rtl_len = 1;
	    }
	} else if (rtl_start != -1) {
	    if (DIRECT_FONT(wch) || !is_rtl(wch)) {
		for (rtl_idx = 0; rtl_idx < rtl_len; rtl_idx++) {
		    wch = term->disptext[hover_start.y]->chars[hover_start.x + rtl_start + (rtl_len - 1 - rtl_idx)].chr;
		    (*phover_url_w)[idx_out] = wch;
		    idx_out++;
		}
		rtl_start = -1; rtl_len = 0;
		wch = term->disptext[hover_start.y]->chars[hover_start.x + idx_in].chr;
		if (DIRECT_FONT(wch)) {
		    wch &= 0xFF;
		}
		(*phover_url_w)[idx_out] = wch; idx_out++;
	    } else if (is_rtl(wch)) {
		rtl_len++;
	    }
	}
    }
    if (rtl_start != -1) {
	for (rtl_idx = 0; rtl_idx < rtl_len; rtl_idx++) {
	    wch = term->disptext[hover_start.y]->chars[hover_start.x + rtl_start + (rtl_len - 1 - rtl_idx)].chr;
	    (*phover_url_w)[idx_out] = wch;
	    idx_out++;
	}
	rtl_start = -1; rtl_len = 0;
    }

    /*
     * XXX document
     */
    (*phover_url_w)[hover_len] = L'\0';
    return TRUE;
}

static wchar_t *winfripp_init_urls_unnest(Conf *conf, wchar_t *url_w)
{
    int foundfl;
    char *nest_char, *nest_chars;
    wchar_t *url_char_w, *url_new_w;
    size_t url_w_len;


    WINFRIPP_DEBUG_ASSERT(url_w);

    nest_chars = conf_get_str(conf, CONF_frip_urls_nest_chars);
    url_w_len = wcslen(url_w);

    for (url_char_w = url_w + (url_w_len ? url_w_len - 1 : 0);
	 url_char_w >= url_w; url_char_w--)
    {
	for (foundfl = 0, nest_char = nest_chars; *nest_char; nest_char++) {
	    if (!iswprint(*nest_char) || (*url_char_w == (wchar_t)(*nest_char))) {
		*url_char_w = '\0'; foundfl = 1;
	    }
	}
	if (!foundfl) {
	    break;
	}
    }
    for (url_char_w = url_new_w = url_w; *url_char_w; url_char_w++) {
	for (foundfl = 0, nest_char = nest_chars; *nest_char; nest_char++) {
	    if (!iswprint(*nest_char) || (*url_char_w == (wchar_t)(*nest_char))) {
		url_new_w = url_char_w + 1; foundfl = 1;
	    }
	}
	if (!foundfl) {
	    break;
	}
    }

    return url_new_w;
}

/*
 * Public subroutines private to FySTY/winfrip*.c
 */

void winfripp_urls_config_panel(struct controlbox *b)
{
    struct controlset *s;


    WINFRIPP_DEBUG_ASSERT(b);

    /*
     * The Frippery: URLs panel.
     */

    ctrl_settitle(b, "Frippery/URLs", "Configure pointless frippery: clickable URLs");
    s = ctrl_getset(b, "Frippery/URLs", "frip_urls", "Clickable URLs settings");
    ctrl_editbox(s, "Match string", NO_SHORTCUT, 75, P(WINFRIPP_HELP_CTX),
		 conf_editbox_handler, I(CONF_frip_urls_match_spec), I(1));
    ctrl_text(s, "Specify multiple match strings by separating them with "
	      "a single semicolon.", P(WINFRIPP_HELP_CTX));
    ctrl_editbox(s, "Nesting characters", NO_SHORTCUT, 75, P(WINFRIPP_HELP_CTX),
		 conf_editbox_handler, I(CONF_frip_urls_nest_chars), I(1));
    ctrl_text(s, "URLs are purged of leading and/or trailing nesting characters.", P(WINFRIPP_HELP_CTX));
}

/*
 * Public subroutines
 */

WinFripReturn winfrip_urls_op(WinFripUrlsOp op, Conf *conf, HWND hwnd, UINT message, unsigned long *tattr, Terminal *term, WPARAM wParam, int x, int y)
{
    size_t nmatch;


    /*
     * XXX document
     */
    if (op == WINFRIP_URLS_OP_CTRL_EVENT) {
	if (wParam & MK_CONTROL) {
	    op = WINFRIP_URLS_OP_CTRL_DOWN;
	} else if (!(wParam & MK_CONTROL)) {
	    op = WINFRIP_URLS_OP_CTRL_UP;
	}
    } else if (op == WINFRIP_URLS_OP_MOUSE_EVENT) {
	if ((message == WM_LBUTTONDOWN) && (wParam & MK_CONTROL)) {
	    op = WINFRIP_URLS_OP_MOUSE_DOWN;
	} else if ((message == WM_LBUTTONUP) && (wParam & MK_CONTROL)) {
	    op = WINFRIP_URLS_OP_MOUSE_UP;
	} else {
	    return WINFRIP_RETURN_CONTINUE;
	}
    }

    switch (op) {
    default:
	WINFRIPP_DEBUG_FAIL();
	return WINFRIP_RETURN_FAILURE;

    /*
     * XXX document
     */
    case WINFRIP_URLS_OP_CLEAR:
	if (winfripp_urls_state == WINFRIPP_URLS_STATE_CLEAR) {
	    winfripp_urls_state = WINFRIPP_URLS_STATE_NONE;
	    winfripp_urls_start.x = winfripp_urls_start.y = 0;
	    winfripp_urls_end.x = winfripp_urls_end.y = 0;
	}
	return WINFRIP_RETURN_CONTINUE;

    /*
     * XXX document
     */
    case WINFRIP_URLS_OP_CTRL_UP:
	if ((winfripp_urls_state == WINFRIPP_URLS_STATE_SELECT) ||
	    (winfripp_urls_state == WINFRIPP_URLS_STATE_CLICK))
	{
	    winfripp_urls_state = WINFRIPP_URLS_STATE_CLEAR;
	    term_update(term);
	}
	return WINFRIP_RETURN_CONTINUE;

    /*
     * XXX document
     */
    case WINFRIP_URLS_OP_DRAW:
	if (winfripp_urls_posle(winfripp_urls_start, x, y) &&
	    winfripp_urls_poslt(x, y, winfripp_urls_end))
	{
	    switch (winfripp_urls_state) {
	    default:
		WINFRIPP_DEBUG_FAIL();
		return WINFRIP_RETURN_FAILURE;
	    case WINFRIPP_URLS_STATE_NONE:
		break;
	    case WINFRIPP_URLS_STATE_CLEAR:
		*tattr &= ~(ATTR_REVERSE | ATTR_UNDER); break;
	    case WINFRIPP_URLS_STATE_SELECT:
		*tattr |= ATTR_UNDER; break;
	    case WINFRIPP_URLS_STATE_CLICK:
		*tattr |= ATTR_REVERSE; break;
	    }
	}
	return WINFRIP_RETURN_CONTINUE;

    /*
     * XXX document
     */
    case WINFRIP_URLS_OP_CTRL_DOWN:
	if (winfripp_init_urls_get_endstart(term, &winfripp_urls_end,
					    &winfripp_urls_start, x, y)) {
	    winfripp_urls_state = WINFRIPP_URLS_STATE_SELECT;
	    term_update(term);
	}
	return WINFRIP_RETURN_CONTINUE;

    /*
     * XXX document
     */
    case WINFRIP_URLS_OP_MOUSE_DOWN:
    case WINFRIP_URLS_OP_MOUSE_UP:
	if (winfripp_urls_state == WINFRIPP_URLS_STATE_SELECT) {
	    if (winfripp_urls_end.x > winfripp_urls_start.x) {
		/*
		 * XXX document
		*/
		if (!winfripp_init_urls_get_url(winfripp_urls_end, winfripp_urls_start, term,
						&winfripp_urls_buf_w, &winfripp_urls_buf_w_size)) {
		    WINFRIPP_DEBUG_FAIL();
		    if (winfripp_urls_buf_w) {
			ZeroMemory(winfripp_urls_buf_w, winfripp_urls_buf_w_size);
		    }
		    winfripp_urls_state = WINFRIPP_URLS_STATE_NONE;
		    term_update(term);
		    return WINFRIP_RETURN_BREAK;
		} else if (!winfripp_init_urls_get_matchv(conf,
							  &winfripp_urls_match_spec_conf,
							  &winfripp_urls_match_spec_w,
							  &winfripp_urls_matchc_w,
							  &winfripp_urls_matchv_w)) {
		    WINFRIPP_DEBUG_FAIL();
		    ZeroMemory(winfripp_urls_buf_w, winfripp_urls_buf_w_size);
		    winfripp_urls_state = WINFRIPP_URLS_STATE_NONE;
		    term_update(term);
		    return WINFRIP_RETURN_BREAK;
		}

		/*
		 * XXX document
		 */
		winfripp_urls_url_w = winfripp_init_urls_unnest(conf, winfripp_urls_buf_w);
		for (nmatch = 0; nmatch < winfripp_urls_matchc_w; nmatch++) {
		    if (PathMatchSpecW(winfripp_urls_url_w, winfripp_urls_matchv_w[nmatch])) {
			WINFRIPP_DEBUGF("URL `%S' matches `%S'", winfripp_urls_url_w, winfripp_urls_matchv_w[nmatch]);
			winfripp_urls_state = WINFRIPP_URLS_STATE_CLICK;
			term_update(term);
			return WINFRIP_RETURN_BREAK;
		    }
		}
		WINFRIPP_DEBUGF("failed to match URL `%S'", winfripp_urls_url_w);
		ZeroMemory(winfripp_urls_buf_w, winfripp_urls_buf_w_size);
		winfripp_urls_state = WINFRIPP_URLS_STATE_NONE;
		winfripp_urls_url_w = NULL;
		term_update(term);
		return WINFRIP_RETURN_BREAK;
	    } else {
		WINFRIPP_DEBUG_FAIL();
		if (winfripp_urls_buf_w) {
		    ZeroMemory(winfripp_urls_buf_w, winfripp_urls_buf_w_size);
		}
		winfripp_urls_state = WINFRIPP_URLS_STATE_NONE;
		term_update(term);
		return WINFRIP_RETURN_BREAK;
	    }
	} else if (winfripp_urls_state == WINFRIPP_URLS_STATE_CLICK) {
	    /*
	     * XXX document
	     */
	    ShellExecuteW(NULL, L"open", winfripp_urls_url_w, NULL, NULL, SW_SHOWNORMAL);
	    ZeroMemory(winfripp_urls_buf_w, winfripp_urls_buf_w_size);
	    winfripp_urls_state = WINFRIPP_URLS_STATE_NONE;
	    winfripp_urls_url_w = NULL;
	    term_update(term);
	    return WINFRIP_RETURN_BREAK;
	} else {
	    return WINFRIP_RETURN_CONTINUE;
	}
    }
}
