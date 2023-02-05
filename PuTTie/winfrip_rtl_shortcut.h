/*
 * winfrip_rtl_shortcut.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_SHORTCUT_H
#define PUTTY_WINFRIP_RTL_SHORTCUT_H

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

WfrStatus WfrCreateShortcut(const wchar_t *app_user_model_id, const wchar_t *description, const wchar_t *shortcut_pname, const wchar_t *target_pname, const wchar_t *working_dname);
WfrStatus WfrCreateShortcutStartup(const wchar_t *app_user_model_id, const wchar_t *description, const wchar_t *shortcut_fname, const wchar_t *target_pname, const wchar_t *working_dname);
WfrStatus WfrDeleteShortcutStartup(const wchar_t *shortcut_fname);

#endif // !PUTTY_WINFRIP_RTL_SHORTCUT_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
