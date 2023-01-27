/*
 * winfrip_storage_host_keys.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_HOST_KEYS_H
#define PUTTY_WINFRIP_STORAGE_HOST_KEYS_H

/*
 * Public host key storage subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfrStatus	WfsCleanupHostKeys(WfsBackend backend);
WfrStatus	WfsClearHostKeys(WfsBackend backend, bool delete_in_backend);
WfrStatus	WfsDeleteHostKey(WfsBackend backend, bool delete_in_backend, const char *key_name);
WfrStatus	WfsEnumerateHostKeys(WfsBackend backend, bool cached, bool initfl, bool *pdonefl, const char **pkey_name, void *state);
WfrStatus	WfsExportHostKey(WfsBackend backend_from, WfsBackend backend_to, bool movefl, const char *key_name);
WfrStatus	WfsExportHostKeys(WfsBackend backend_from, WfsBackend backend_to, bool clear_to, bool continue_on_error, void (*error_fn)(const char *, WfrStatus));
WfrStatus	WfsGetHostKey(WfsBackend backend, bool cached, const char *key_name, const char **pkey);
WfrStatus	WfsPrintHostKeyName(const char *hostname, int port, const char *keytype, char **pkey_name);
WfrStatus	WfsRenameHostKey(WfsBackend backend, bool rename_in_backend, const char *key_name, const char *key_name_new);
WfrStatus	WfsSaveHostKey(WfsBackend backend, const char *key_name, const char *key);
WfrStatus	WfsSetHostKey(WfsBackend backend, const char *key_name, const char *key);

#endif // !PUTTY_WINFRIP_STORAGE_HOST_KEYS_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
