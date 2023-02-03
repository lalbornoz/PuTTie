/*
 * winfrip_storage_privkey_list.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_PRIVKEY_LIST_H
#define PUTTY_WINFRIP_STORAGE_PRIVKEY_LIST_H

/*
 * Public Pageant private key list storage subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfrStatus	WfsAddPrivKeyList(WfsBackend backend, const char *const privkey_name);
WfrStatus	WfsCleanupPrivKeyList(WfsBackend backend);
WfrStatus	WfsClearPrivKeyList(WfsBackend backend);
WfrStatus	WfsExportPrivKeyList(WfsBackend backend_from, WfsBackend backend_to, bool movefl);
WfrStatus	WfsGetEntriesPrivKeyList(WfsBackend backend, char **pprivkey_list, size_t *pprivkey_list_size);
WfrStatus	WfsRemovePrivKeyList(WfsBackend backend, const char *const privkey_name);

#endif // !PUTTY_WINFRIP_STORAGE_PRIVKEY_LIST_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
