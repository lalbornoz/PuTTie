/*
 * winfrip_feature_storage.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_FEATURE_STORAGE_SESSIONS_H
#define PUTTY_WINFRIP_FEATURE_STORAGE_SESSIONS_H

/*
 * Public subroutine prototypes used by/in:
 * PuTTie/winfrip_putty_config.c
 * config.c
 */

void	WffsSessionsConfigPanel(struct controlbox *b);
void	WffsSessionsConfigPanelDroplistBackendHandler(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);

#endif // !PUTTY_WINFRIP_FEATURE_STORAGE_SESSIONS_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
