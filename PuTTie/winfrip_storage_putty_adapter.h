/*
 * winfrip_storage_putty_adapter.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_PUTTY_ADAPTER_H
#define PUTTY_WINFRIP_STORAGE_PUTTY_ADAPTER_H

/*
 * Public wrapped PuTTY storage.h type definitions and subroutine prototypes
 * (only reproduced for consistency's sake)
 */

// settings_w *open_settings_w(const char *sessionname, char **errmsg);
// void write_setting_s(settings_w *handle, const char *key, const char *value);
// void write_setting_i(settings_w *handle, const char *key, int value);
// void write_setting_filename(settings_w *handle, const char *key, Filename *value);
// void write_setting_fontspec(settings_w *handle, const char *key, FontSpec *font);
// void close_settings_w(settings_w *handle);

// settings_r *open_settings_r(const char *sessionname);
// char *read_setting_s(settings_r *handle, const char *key);
// int read_setting_i(settings_r *handle, const char *key, int defvalue);
// Filename *read_setting_filename(settings_r *handle, const char *key);
// FontSpec *read_setting_fontspec(settings_r *handle, const char *key);
// void close_settings_r(settings_r *handle);

// void del_settings(const char *sessionname);

// settings_e *enum_settings_start(void);
// bool enum_settings_next(settings_e *handle, strbuf *out);
// void enum_settings_finish(settings_e *handle);

// int check_stored_host_key(const char *hostname, int port, const char *keytype, const char *key);
// void store_host_key(Seat *seat, const char *hostname, int port, const char *keytype, const char *key);

// typedef void (*noise_consumer_t) (void *data, int len);
// void read_random_seed(noise_consumer_t consumer);
// void write_random_seed(void *data, int len);

// void cleanup_all(void);

/*
 * Public wrapped PuTTY windows/jump-list.c subroutine prototypes
 */

void add_session_to_jumplist(const char *const sessionname);
void clear_jumplist(void);
void remove_session_from_jumplist(const char *const sessionname);

#endif // !PUTTY_WINFRIP_STORAGE_PUTTY_ADAPTER_H

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
