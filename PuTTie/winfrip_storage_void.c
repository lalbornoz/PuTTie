/*
 * winfrip_storage_void.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include "putty.h"
#include "storage.h"

#include "PuTTie/winfrip.h"
#include "PuTTie/winfrip_priv.h"
#include "PuTTie/winfrip_storage_priv.h"
#include "PuTTie/winfrip_storage_PuTTY.h"

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

typedef struct settings_w {
} settings_w;
typedef struct settings_r {
} settings_r;

/*
 * Public subroutines
 */

settings_w *open_settings_w(const char *sessionname, char **errmsg)
{
	(void)sessionname; (void)errmsg;
	return NULL;
}

void write_setting_s(settings_w *handle, const char *key, const char *value)
{
	(void)handle; (void)key; (void)value;
}

void write_setting_i(settings_w *handle, const char *key, int value)
{
	(void)handle; (void)key; (void)value;
}

void write_setting_filename(settings_w *handle,
							const char *key, Filename *value)
{
	(void)handle; (void)key; (void)value;
}

void write_setting_fontspec(settings_w *handle,
							const char *key, FontSpec *font)
{
	(void)handle; (void)key; (void)font;
}

void close_settings_w(settings_w *handle)
{
	(void)handle;
}


settings_r *open_settings_r(const char *sessionname)
{
	(void)sessionname;
	return NULL;
}

char *read_setting_s(settings_r *handle, const char *key)
{
	(void)handle; (void)key;
	return NULL;
}

int read_setting_i(settings_r *handle, const char *key, int defvalue)
{
	(void)handle; (void)key; (void)defvalue;
	return -1;
}

Filename *read_setting_filename(settings_r *handle, const char *key)
{
	(void)handle; (void)key;
	return NULL;
}

FontSpec *read_setting_fontspec(settings_r *handle, const char *key)
{
	(void)handle; (void)key;
	return NULL;
}

void close_settings_r(settings_r *handle)
{
	(void)handle;
}


void del_settings(const char *sessionname)
{
	(void)sessionname;
}


settings_e *enum_settings_start(void)
{
	return NULL;
}

bool enum_settings_next(settings_e *handle, strbuf *out)
{
	(void)handle; (void)out;
	return false;
}

void enum_settings_finish(settings_e *handle)
{
	(void)handle;
}


int check_stored_host_key(const char *hostname, int port,
						  const char *keytype, const char *key)
{
	(void)hostname; (void)port; (void)keytype; (void)key;
	return 1;
}

void store_host_key(Seat *seat, const char *hostname, int port,
					const char *keytype, const char *key)
{
	(void)seat; (void)hostname; (void)port; (void)keytype; (void)key;
}


typedef void (*noise_consumer_t) (void *data, int len);

void read_random_seed(noise_consumer_t consumer)
{
	(void)consumer;
}

void write_random_seed(void *data, int len)
{
	(void)data; (void)len;
}


void cleanup_all(void)
{
}

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */
