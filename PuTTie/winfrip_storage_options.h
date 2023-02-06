/*
 * winfrip_storage_options.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_OPTIONS_H
#define PUTTY_WINFRIP_STORAGE_OPTIONS_H

/*
 * Public global options storage subroutine prototypes private to PuTTie/winfrip_storage*.c
 */

WfrStatus	WfsCleanupOptions(WfsBackend backend);
WfrStatus	WfsClearOptions(WfsBackend backend, bool delete_in_backend);
WfrStatus	WfsCopyOption(WfsBackend backend_from, WfsBackend backend_to, bool save_in_backend, const char *key);
WfrStatus	WfsDeleteOption(WfsBackend backend, bool delete_in_backend, const char *key);
WfrStatus	WfsEnumerateOptions(WfsBackend backend, bool initfl, bool *pdonefl, char **pkey, void **pstate);
WfrStatus	WfsExportOptions(WfsBackend backend_from, WfsBackend backend_to, bool clear_to, bool continue_on_error, void (*error_fn)(const char *, WfrStatus));
WfrStatus	WfsGetOption(WfsBackend backend, const char *key, WfrTreeItem **pitem, void **pvalue, size_t *pvalue_size, WfrTreeItemType *pvalue_type);
WfrStatus	WfsGetOptionIntWithDefault(WfsBackend backend, const char *key, int value_default, int **pvalue);
WfrStatus	WfsGetOptionWithDefault(WfsBackend backend, const char *key, void *value_default, size_t value_default_size, WfrTreeItemType value_default_type, void **pvalue, size_t *pvalue_size, WfrTreeItemType *pvalue_type);
WfrStatus	WfsLoadOptions(WfsBackend backend);
WfrStatus	WfsRenameOption(WfsBackend backend, bool rename_in_backend, const char *key, const char *key_new);
WfrStatus	WfsSaveOptions(WfsBackend backend);
WfrStatus	WfsSetOption(WfsBackend backend, bool clone_value, bool set_in_backend, const char *key, const void *value, size_t value_size, WfrTreeItemType value_type);

#endif // !PUTTY_WINFRIP_STORAGE_OPTIONS_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
