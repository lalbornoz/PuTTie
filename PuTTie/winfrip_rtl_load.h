/*
 * winfrip_rtl_load.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_LOAD_H
#define PUTTY_WINFRIP_RTL_LOAD_H

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

typedef WfrStatus (*WfrLoadParseItemFn)(void *param1, void *param2, const char *key, int type, const void *value, size_t value_size);
typedef WfrStatus (*WfrLoadRegSubKeyItemFn)(void *param1, void *param2, const char *key, int type, const void *value, size_t value_len);

WfrStatus	WfrLoadListFromFile(const char *fname, char **plist, size_t *plist_size);
WfrStatus	WfrLoadParse(char *data, size_t data_size, void *param1, void *param2, WfrLoadParseItemFn item_fn);
WfrStatus	WfrLoadRawFile(bool escape_fnamefl, const char *dname, const char *ext, const char *fname, char **pdata, size_t *pdata_size, time_t *pmtime);
WfrStatus	WfrLoadRegSubKey(const char *key_name, const char *subkey, void *param1, void *param2, WfrLoadRegSubKeyItemFn item_fn);

#endif // !PUTTY_WINFRIP_RTL_LOAD_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
