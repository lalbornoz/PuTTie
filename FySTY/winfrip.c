/*
 * winfrip.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "winfrip.h"
#include "winfrip_priv.h"

#include <assert.h>

/*
 * Public subroutines private to FySTY/winfrip*.c
 */

BOOL winfrip_towcsdup(char *in, size_t in_size, wchar_t **pout_w)
{
    size_t out_w_len, out_w_size;
    wchar_t *out_w;


    /*
     * XXX document
     */
    out_w_len = MultiByteToWideChar(CP_ACP, 0, in, in_size, NULL, 0);
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
    winfrip_bgimg_config_panel(b);
    winfrip_mouse_config_panel(b);
    winfrip_transp_config_panel(b);
    winfrip_urls_config_panel(b);
}

void winfrip_debug_init(void)
{
    /*
     * XXX document
     */
#ifdef WINFRIP_DEBUG
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
#endif
}
