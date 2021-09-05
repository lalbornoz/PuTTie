/*
 * winfrip_storage_PuTTY.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_PUTTY_H
#define PUTTY_WINFRIP_STORAGE_PUTTY_H

settings_w *open_settings_w_PuTTY(const char *sessionname, char **errmsg);
void write_setting_s_PuTTY(settings_w *handle, const char *key, const char *value);
void write_setting_i_PuTTY(settings_w *handle, const char *key, int value);
void write_setting_filename_PuTTY(settings_w *handle, const char *key, Filename *value);
void write_setting_fontspec_PuTTY(settings_w *handle, const char *key, FontSpec *font);
void close_settings_w_PuTTY(settings_w *handle);

settings_r *open_settings_r_PuTTY(const char *sessionname);
char *read_setting_s_PuTTY(settings_r *handle, const char *key);
int read_setting_i_PuTTY(settings_r *handle, const char *key, int defvalue);
Filename *read_setting_filename_PuTTY(settings_r *handle, const char *key);
FontSpec *read_setting_fontspec_PuTTY(settings_r *handle, const char *key);
void close_settings_r_PuTTY(settings_r *handle);

void del_settings_PuTTY(const char *sessionname);

settings_e *enum_settings_start_PuTTY(void);
bool enum_settings_next_PuTTY(settings_e *handle, strbuf *out);
void enum_settings_finish_PuTTY(settings_e *handle);

int verify_host_key_PuTTY(const char *hostname, int port, const char *keytype, const char *key);
void store_host_key_PuTTY(const char *hostname, int port, const char *keytype, const char *key);

typedef void (*noise_consumer_t_PuTTY) (void *data, int len);
void read_random_seed_PuTTY(noise_consumer_t_PuTTY consumer);
void write_random_seed_PuTTY(void *data, int len);

void cleanup_all_PuTTY(void);

#endif /* !PUTTY_WINFRIP_STORAGE_PUTTY_H */
