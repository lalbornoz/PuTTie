/*
 * winfrip_storage_wrap.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_WRAP_H
#define PUTTY_WINFRIP_STORAGE_WRAP_H

#define open_settings_w		open_settings_w_PuTTY
#define write_setting_s		write_setting_s_PuTTY
#define write_setting_i		write_setting_i_PuTTY
#define write_setting_filename	write_setting_filename_PuTTY
#define write_setting_fontspec	write_setting_fontspec_PuTTY
#define close_settings_w	close_settings_w_PuTTY

#define open_settings_r		open_settings_r_PuTTY
#define read_setting_s		read_setting_s_PuTTY
#define read_setting_i		read_setting_i_PuTTY
#define read_setting_filename	read_setting_filename_PuTTY
#define read_setting_fontspec	read_setting_fontspec_PuTTY
#define close_settings_r	close_settings_r_PuTTY

#define del_settings		del_settings_PuTTY

#define enum_settings_start	enum_settings_start_PuTTY
#define enum_settings_next	enum_settings_next_PuTTY
#define enum_settings_finish	enum_settings_finish_PuTTY

#define verify_host_key		verify_host_key_PuTTY
#define store_host_key		store_host_key_PuTTY

#define read_random_seed	read_random_seed_PuTTY
#define write_random_seed	write_random_seed_PuTTY

#define cleanup_all		cleanup_all_PuTTY

#endif /* !PUTTY_WINFRIP_STORAGE_WRAP_H */
