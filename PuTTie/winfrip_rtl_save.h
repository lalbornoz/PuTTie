/*
 * winfrip_rtl_save.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_SAVE_H
#define PUTTY_WINFRIP_RTL_SAVE_H

#include "winfrip_rtl_tree.h"

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

WfrStatus	WfrSaveListToFile(const char *fname, const char *fname_tmp, const char *list, size_t list_size);
WfrStatus	WfrSaveRawFile(bool escape_fnamefl, bool recreate_dnamefl, char *dname, const char *ext, const char *fname, const char *data, size_t data_size);
WfrStatus	WfrSaveToFileV(bool escape_fnamefl, char *dname, const char *ext, const char *fname, ...);
WfrStatus	WfrSaveToRegSubKeyV(const char *key_name, const char *subkey, ...);
WfrStatus	WfrSaveTreeToFile(bool escape_fnamefl, char *dname, const char *ext, const char *fname, WfrTree *tree);
WfrStatus	WfrSaveTreeToRegSubKey(const char *key_name, const char *subkey, WfrTree *tree);

#endif // !PUTTY_WINFRIP_RTL_SAVE_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
