/*
 * winfrip_storage_adapter.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_ADAPTER_H
#define PUTTY_WINFRIP_ADAPTER_H

/*
 * Public subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

bool WfsDisableAdapterDisplayErrors(void);
bool WfsEnableAdapterDisplayErrors(void);
bool WfsGetAdapterDisplayErrors(void);
void WfsSetAdapterDisplayErrors(bool display_errorsfl);

#endif // !PUTTY_WINFRIP_ADAPTER_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
