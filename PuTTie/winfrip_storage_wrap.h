/*
 * winfrip_storage_wrap.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
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

#define check_stored_host_key	check_stored_host_key_PuTTY
#define store_host_key		store_host_key_PuTTY

#define enum_host_ca_start	enum_host_ca_start_PuTTY
#define enum_host_ca_next	enum_host_ca_next_PuTTY
#define enum_host_ca_finish	enum_host_ca_finish_PuTTY

#define host_ca_load		host_ca_load_PuTTY
#define host_ca_save		host_ca_save_PuTTY
#define host_ca_delete		host_ca_delete_PuTTY

#define cleanup_all		cleanup_all_PuTTY

#endif // !PUTTY_WINFRIP_STORAGE_WRAP_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
