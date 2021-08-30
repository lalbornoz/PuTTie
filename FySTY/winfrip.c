/*
 * winfrip.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "terminal.h"
#include "FySTY/winfrip.h"
#include "FySTY/winfrip_priv.h"

#include <stdarg.h>

/*
 * Public subroutines private to FySTY/winfrip*.c
 */

#ifdef WINFRIP_DEBUG
void winfripp_debugf(const char *fmt, const char *file, const char *func, int line, ...)
{
    va_list ap;


    fprintf(stderr, "In %s:%d:%s():\n", file, line, func);
    va_start(ap, line);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n\n");
}
#endif

typedef struct compressed_scrollback_line {
    size_t len;
} compressed_scrollback_line;

BOOL winfripp_get_term_line(Terminal *term, wchar_t **pline_w, size_t *pline_w_len, int y)
{
    size_t idx_in, idx_out;
    termline *line = NULL;
    wchar_t *line_w;
    size_t line_w_len;
    size_t rtl_idx, rtl_len;
    int rtl_start;
    wchar_t wch;


    WINFRIPP_DEBUG_ASSERT(term);
    WINFRIPP_DEBUG_ASSERT(pline_w);
    WINFRIPP_DEBUG_ASSERT(pline_w_len);

    /*
     * Reject invalid y coordinates falling outside of term->{cols,rows}. Fail given
     * failure to allocate sufficient memory to the line_w buffer of term->cols + 1
     * units of wchar_t.
     */

    if (y >= term->rows) {
	WINFRIPP_DEBUG_FAIL();
	return FALSE;
    } else {
	line_w_len = term->cols;
	if (!(line_w = sresize(NULL, line_w_len + 1, wchar_t))) {
	    WINFRIPP_DEBUG_FAIL();
	    return FALSE;
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

    for (idx_in = idx_out = 0, rtl_len = 0, rtl_start = -1; idx_in < term->cols; idx_in++) {
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
    return TRUE;
}

BOOL winfripp_is_vkey_down(int nVirtKey)
{
    return (GetKeyState(nVirtKey) < 0);
}

BOOL winfripp_towcsdup(char *in, size_t in_size, wchar_t **pout_w)
{
    size_t out_w_len, out_w_size;
    wchar_t *out_w;


    WINFRIPP_DEBUG_ASSERT(in);
    WINFRIPP_DEBUG_ASSERT(in_size > 0);
    WINFRIPP_DEBUG_ASSERT(pout_w);

    out_w_len = MultiByteToWideChar(CP_ACP, 0, in, in_size, NULL, 0);
    WINFRIPP_DEBUG_ASSERT(out_w_len > 0);
    if (out_w_len > 0) {
	out_w_size = out_w_len * sizeof(*out_w);
	out_w = snewn(out_w_size, wchar_t);
	ZeroMemory(out_w, out_w_size);
	if (MultiByteToWideChar(CP_ACP, 0, in, in_size, out_w, out_w_size) == out_w_len) {
	    *pout_w = out_w;
	    return TRUE;
	}
    }
    return FALSE;
}

wchar_t *winfripp_wcsndup(const wchar_t *in_w, size_t in_w_len)
{
    size_t out_w_size;
    wchar_t *out_w;


    WINFRIPP_DEBUG_ASSERT(in_w);
    WINFRIPP_DEBUG_ASSERT(in_w_len > 0);

    out_w_size = (in_w_len + 1) * sizeof(*out_w);
    out_w = snewn(out_w_size, wchar_t);
    ZeroMemory(out_w, out_w_size);
    wcsncpy(out_w, in_w, in_w_len);
    return out_w;
}

/*
 * Public subroutines
 */

void winfrip_config_panel(struct controlbox *b)
{
    /*
     * The Frippery panel.
     */
    ctrl_settitle(b, "Frippery", "Pointless frippery");

    winfripp_bgimg_config_panel(b);
    winfripp_general_config_panel(b);
    winfripp_mouse_config_panel(b);
    winfripp_transp_config_panel(b);
    winfripp_urls_config_panel(b);
}

void winfrip_debug_init(void)
{
#ifdef WINFRIP_DEBUG
    COORD dwSize = {.X = 125, .Y = 50 * 25};
    SMALL_RECT consoleWindow = {.Left = 0, .Top = 0, .Right = dwSize.X - 1, .Bottom = (dwSize.Y / 25) - 1};
    HANDLE hConsoleOutput;


    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
    hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    (void)SetConsoleScreenBufferSize(hConsoleOutput, dwSize);
    (void)SetConsoleWindowInfo(hConsoleOutput, TRUE, &consoleWindow);
#endif
}
