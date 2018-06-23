/*
 * winfrip_urls.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "terminal.h"
#include "winfrip.h"
#include "winfrip_priv.h"

#include <assert.h>
#include <shlwapi.h>

/*
 * Preprocessor macros
 */

/*
 * XXX document
 */
#define winfrip_posle(p1,px,py) ( (p1).y < (py) || ( (p1).y == (py) && (p1).x <= (px) ) )
#define winfrip_poslt(px,py,p2) ( (py) < (p2).y || ( (py) == (p2).y && (px) < (p2).x ) )

/*
 * {External,Static} variables
 */

/*
 * XXX document
 */
extern Conf *conf;

/*
 * XXX document
 */
static pos winfrip_urls_end = {0, 0};
static pos winfrip_urls_start = {0, 0};
static WinFripUrlsState winfrip_urls_state = WINFRIP_URLS_STATE_NONE;

/*
 * XXX document
 */
static wchar_t *winfrip_urls_url_w = NULL;
static size_t winfrip_urls_url_w_size = 0;

/*
 * XXX document
 */
static char *winfrip_urls_match_spec_conf = NULL;
static wchar_t **winfrip_urls_matchv_w = NULL;
static size_t winfrip_urls_matchc_w = 0;

/*
 * Private subroutine prototypes
 */

static BOOL winfrip_init_urls_get_endstart(Terminal *term, pos *pend, pos *pstart, int x, int y);
static BOOL winfrip_init_urls_get_matchv(char **pmatch_spec_conf, size_t *pmatchc_w, wchar_t ***pmatchv_w);
static BOOL winfrip_init_urls_get_url(pos hover_end, pos hover_start, wchar_t **phover_url_w, size_t *phover_url_w_size);

/*
 * Private subroutines
 */

static BOOL winfrip_init_urls_get_endstart(Terminal *term, pos *pend, pos *pstart, int x, int y)
{
    int x_end, x_start;
    wchar_t wch;


    assert(term);
    assert(pend);
    assert(pstart);

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

static BOOL winfrip_init_urls_get_matchv(char **pmatch_spec_conf, size_t *pmatchc_w, wchar_t ***pmatchv_w)
{
    char *match_spec_conf;
    size_t match_spec_conf_len;
    wchar_t *match_spec_w, *p, *q;
    size_t new_matchc_w;
    wchar_t **new_matchv_w;
    size_t nitem, nitems;


    assert(pmatch_spec_conf);
    assert(pmatchc_w);
    assert(pmatchv_w);

    /*
     * XXX document
     */
    match_spec_conf = conf_get_str(conf, CONF_frip_urls_match_spec);
    match_spec_conf_len = strlen(match_spec_conf);
    if (!match_spec_conf_len) {
	return FALSE;
    } else if (*pmatch_spec_conf != match_spec_conf) {
	if (!winfrip_towcsdup(match_spec_conf, match_spec_conf_len + 1, &match_spec_w)) {
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
    new_matchc_w = nitems + 1;
    if (!(new_matchv_w = snewn(new_matchc_w, wchar_t *))) {
	sfree(match_spec_w);
	return FALSE;
    } else {
	ZeroMemory(new_matchv_w, new_matchc_w * sizeof(*new_matchv_w));
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
    sfree(match_spec_w);
    if (*pmatchv_w) {
	sfree(*pmatchv_w);
    }
    *pmatch_spec_conf = match_spec_conf;
    *pmatchc_w = new_matchc_w;
    *pmatchv_w = new_matchv_w;
    return TRUE;
}

static BOOL winfrip_init_urls_get_url(pos hover_end, pos hover_start, wchar_t **phover_url_w, size_t *phover_url_w_size)
{
    size_t hover_len, idx_in, idx_out, new_buf_w_size;
    wchar_t *new_buf_w;
    size_t rtl_idx, rtl_len;
    int rtl_start;
    wchar_t wch;


    assert(phover_url_w);
    assert(phover_url_w_size);

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

/*
 * Public subroutines
 */

BOOL winfrip_urls_op(WinFripUrlsOp op, HWND hwnd, UINT message, unsigned long *tattr, Terminal *term, WPARAM wParam, int x, int y)
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
	    return FALSE;
	}
    }

    switch (op) {
    default:
	break;

    /*
     * XXX document
     */
    case WINFRIP_URLS_OP_CLEAR:
	if (winfrip_urls_state == WINFRIP_URLS_STATE_CLEAR) {
	    winfrip_urls_state = WINFRIP_URLS_STATE_NONE;
	    winfrip_urls_start.x = winfrip_urls_start.y = 0;
	    winfrip_urls_end.x = winfrip_urls_end.y = 0;
	}
	break;

    /*
     * XXX document
     */
    case WINFRIP_URLS_OP_CTRL_UP:
	if ((winfrip_urls_state == WINFRIP_URLS_STATE_SELECT) ||
	    (winfrip_urls_state == WINFRIP_URLS_STATE_CLICK))
	{
	    winfrip_urls_state = WINFRIP_URLS_STATE_CLEAR;
	    term_update(term);
	}
	break;

    /*
     * XXX document
     */
    case WINFRIP_URLS_OP_DRAW:
	if (winfrip_posle(winfrip_urls_start, x, y) &&
	    winfrip_poslt(x, y, winfrip_urls_end))
	{
	    switch (winfrip_urls_state) {
	    case WINFRIP_URLS_STATE_NONE:
		break;
	    case WINFRIP_URLS_STATE_CLEAR:
		*tattr &= ~(ATTR_REVERSE | ATTR_UNDER); break;
	    case WINFRIP_URLS_STATE_SELECT:
		*tattr |= ATTR_UNDER; break;
	    case WINFRIP_URLS_STATE_CLICK:
		*tattr |= ATTR_REVERSE; break;
	    }
	}
	break;

    /*
     * XXX document
     */
    case WINFRIP_URLS_OP_CTRL_DOWN:
	if (winfrip_init_urls_get_endstart(term, &winfrip_urls_end,
					    &winfrip_urls_start, x, y)) {
	    winfrip_urls_state = WINFRIP_URLS_STATE_SELECT;
	    term_update(term);
	}
	break;

    /*
     * XXX document
     */
    case WINFRIP_URLS_OP_MOUSE_DOWN:
    case WINFRIP_URLS_OP_MOUSE_UP:
	if (winfrip_urls_state == WINFRIP_URLS_STATE_SELECT) {
	    if (winfrip_urls_end.x > winfrip_urls_start.x) {
		/*
		 * XXX document
		*/
		if (!winfrip_init_urls_get_url(winfrip_urls_end, winfrip_urls_start,
						&winfrip_urls_url_w, &winfrip_urls_url_w_size)) {
		    if (winfrip_urls_url_w) {
			ZeroMemory(winfrip_urls_url_w, winfrip_urls_url_w_size);
		    }
		    winfrip_urls_state = WINFRIP_URLS_STATE_NONE;
		    term_update(term);
		    return TRUE;
		} else if (!winfrip_init_urls_get_matchv(&winfrip_urls_match_spec_conf,
							  &winfrip_urls_matchc_w,
							  &winfrip_urls_matchv_w)) {
		    ZeroMemory(winfrip_urls_url_w, winfrip_urls_url_w_size);
		    winfrip_urls_state = WINFRIP_URLS_STATE_NONE;
		    term_update(term);
		    return TRUE;
		}

		/*
		 * XXX document
		 */
		for (nmatch = 0; nmatch < winfrip_urls_matchc_w; nmatch++) {
		    if (PathMatchSpecW(winfrip_urls_url_w, winfrip_urls_matchv_w[nmatch])) {
			winfrip_urls_state = WINFRIP_URLS_STATE_CLICK;
			term_update(term);
			return TRUE;
		    }
		}
		ZeroMemory(winfrip_urls_url_w, winfrip_urls_url_w_size);
		winfrip_urls_state = WINFRIP_URLS_STATE_NONE;
		term_update(term);
		return TRUE;
	    } else {
		if (winfrip_urls_url_w) {
		    ZeroMemory(winfrip_urls_url_w, winfrip_urls_url_w_size);
		}
		winfrip_urls_state = WINFRIP_URLS_STATE_NONE;
		term_update(term);
		return TRUE;
	    }
	} else if (winfrip_urls_state == WINFRIP_URLS_STATE_CLICK) {
	    /*
	     * XXX document
	     */
	    ShellExecuteW(NULL, L"open", winfrip_urls_url_w, NULL, NULL, SW_SHOWNORMAL);
	    ZeroMemory(winfrip_urls_url_w, winfrip_urls_url_w_size);
	    winfrip_urls_state = WINFRIP_URLS_STATE_NONE;
	    term_update(term);
	    return TRUE;
	}
	break;
    }
    return FALSE;
}
