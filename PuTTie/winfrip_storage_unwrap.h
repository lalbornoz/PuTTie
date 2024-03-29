/*
 * winfrip_storage_unwrap.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_UNWRAP_H
#define PUTTY_WINFRIP_STORAGE_UNWRAP_H

#undef open_settings_w
#undef write_setting_s
#undef write_setting_i
#undef write_setting_filename
#undef write_setting_fontspec
#undef close_settings_w

#undef open_settings_r
#undef read_setting_s
#undef read_setting_i
#undef read_setting_filename
#undef read_setting_fontspec
#undef close_settings_r

#undef del_settings

#undef enum_settings_start
#undef enum_settings_next
#undef enum_settings_finish

#undef check_stored_host_key
#undef store_host_key

#undef enum_host_ca_start
#undef enum_host_ca_next
#undef enum_host_ca_finish

#undef host_ca_load
#undef host_ca_save
#undef host_ca_delete

#undef cleanup_all

#endif // !PUTTY_WINFRIP_STORAGE_UNWRAP_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
