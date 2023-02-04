/*
 * winfrip_rtl_shortcut.hpp - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_RTL_SHORTCUT_HPP
#define PUTTY_WINFRIP_RTL_SHORTCUT_HPP

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

WfrStatus WfrCreateShortcut(LPCWSTR app_user_model_id, LPCWSTR description, LPCWSTR shortcut_pname, LPCWSTR target_pname, LPCWSTR working_dname);

#endif // !PUTTY_WINFRIP_RTL_SHORTCUT_HPP

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
