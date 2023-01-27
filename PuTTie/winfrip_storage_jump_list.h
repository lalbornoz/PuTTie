/*
 * winfrip_storage_jump_list.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_JUMP_LIST_H
#define PUTTY_WINFRIP_STORAGE_JUMP_LIST_H

/*
 * Public jump list storage subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

void		WfsAddJumpList(WfsBackend backend, const char *const sessionname);
WfrStatus	WfsCleanupJumpList(WfsBackend backend);
void		WfsClearJumpList(WfsBackend backend);
WfrStatus	WfsExportJumpList(WfsBackend backend_from, WfsBackend backend_to, bool movefl);
char *		WfsGetEntriesJumpList(WfsBackend backend);
void		WfsRemoveJumpList(WfsBackend backend, const char *const sessionname);

#endif // !PUTTY_WINFRIP_STORAGE_JUMP_LIST_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
