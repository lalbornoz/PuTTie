/*
 * winfrip_storage_host_ca.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_HOST_CA_H
#define PUTTY_WINFRIP_STORAGE_HOST_CA_H

/*
 * Public host CAs storage subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfrStatus	WfsAddHostCA(WfsBackend backend, const char *public_key, const char *name, bool permit_rsa_sha1, bool permit_rsa_sha256, bool permit_rsa_sha512, const char *validity, WfspHostCA **phca);
WfrStatus	WfsCleanupHostCAs(WfsBackend backend);
WfrStatus	WfsClearHostCAs(WfsBackend backend, bool delete_in_backend);
WfrStatus	WfsCloseHostCA(WfsBackend backend, WfspHostCA *hca);
WfrStatus	WfsCopyHostCA(WfsBackend backend_from, WfsBackend backend_to, const char *name, WfspHostCA *hca, WfspHostCA **phca);
WfrStatus	WfsDeleteHostCA(WfsBackend backend, bool delete_in_backend, const char *name);
WfrStatus	WfsEnumerateHostCAs(WfsBackend backend, bool cached, bool initfl, bool *pdonefl, char **pname, void *state);
WfrStatus	WfsExportHostCA(WfsBackend backend_from, WfsBackend backend_to, bool movefl, char *name);
WfrStatus	WfsExportHostCAs(WfsBackend backend_from, WfsBackend backend_to, bool clear_to, bool continue_on_error, void (*error_fn)(const char *, WfrStatus));
WfrStatus	WfsGetHostCA(WfsBackend backend, bool cached, const char *name, WfspHostCA **phca);
WfrStatus	WfsRenameHostCA(WfsBackend backend, bool rename_in_backend, const char *name, const char *name_new);
WfrStatus	WfsSaveHostCA(WfsBackend backend, WfspHostCA *hca);

#endif // !PUTTY_WINFRIP_STORAGE_HOST_CA_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
