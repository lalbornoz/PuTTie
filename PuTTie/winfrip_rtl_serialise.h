/*
 * winfrip_rtl_serialise.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_SERIALISE_H
#define PUTTY_WINFRIP_RTL_SERIALISE_H

#include "winfrip_rtl_pcre2.h"
#include "winfrip_rtl_registry.h"
#include "winfrip_rtl_tree.h"

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

typedef WfrStatus (*WfrLoadParseItemFn)(void *param1, void *param2, const char *key, int type, const void *value, size_t value_size);
typedef WfrStatus (*WfrLoadRegSubKeyItemFn)(void *param1, void *param2, const char *key, int type, const void *value, size_t value_len);

WfrStatus	WfrLoadParse(char *data, size_t data_size, WfrLoadParseItemFn item_fn, void *param1, void *param2);
WfrStatus	WfrLoadRegSubKey(const char *key_name, const char *subkey, WfrLoadRegSubKeyItemFn item_fn, void *param1, void *param2);
WfrStatus	WfrSaveToFileV(bool escape_fnamefl, char *dname, const char *ext, const char *fname, ...);
WfrStatus	WfrSaveToRegSubKeyV(const char *key_name, const char *subkey, ...);
WfrStatus	WfrSaveTreeToFile(bool escape_fnamefl, char *dname, const char *ext, const char *fname, WfrTree *tree);
WfrStatus	WfrSaveTreeToRegSubKey(const char *key_name, const char *subkey, WfrTree *tree);

#endif // !PUTTY_WINFRIP_RTL_SERIALISE_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
