/*
 * winfrip.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
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


    /*
     * XXX document
     */
    fprintf(stderr, "In %s:%d:%s():\n", file, line, func);
    va_start(ap, line);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n\n");
}
#endif

BOOL winfripp_towcsdup(char *in, size_t in_size, wchar_t **pout_w)
{
    size_t out_w_len, out_w_size;
    wchar_t *out_w;


    WINFRIPP_DEBUG_ASSERT(in);
    WINFRIPP_DEBUG_ASSERT(in_size > 0);
    WINFRIPP_DEBUG_ASSERT(pout_w);

    /*
     * XXX document
     */
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
    /*
     * XXX document
     */
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
#endif
}
