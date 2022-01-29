/*
 * winfrip_storage_file.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "storage.h"

#include "FySTY/winfrip.h"
#include "FySTY/winfrip_priv.h"
#include "FySTY/winfrip_storage_priv.h"
#include "FySTY/winfrip_storage_PuTTY.h"

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

/* ----------------------------------------------------------------------
 * Functions to save and restore PuTTY sessions. Note that this is
 * only the low-level code to do the reading and writing. The
 * higher-level code that translates an internal Conf structure into
 * a set of (key,value) pairs in their external storage format is
 * elsewhere, since it doesn't (mostly) change between platforms.
 */

/*
 * Write a saved session. The caller is expected to call
 * open_setting_w() to get a `void *' handle, then pass that to a
 * number of calls to write_setting_s() and write_setting_i(), and
 * then close it using close_settings_w(). At the end of this call
 * sequence the settings should have been written to the PuTTY
 * persistent storage area.
 *
 * A given key will be written at most once while saving a session.
 * Keys may be up to 255 characters long.  String values have no length
 * limit.
 *
 * Any returned error message must be freed after use.
 */

settings_w *open_settings_w(const char *sessionname, char **errmsg)
{
    return open_settings_w_PuTTY(sessionname, errmsg);
}

void write_setting_s(settings_w *handle, const char *key, const char *value)
{
    write_setting_s(handle, key, value);
}

void write_setting_i(settings_w *handle, const char *key, int value)
{
    write_setting_i(handle, key, value);
}

void write_setting_filename(settings_w *handle,
			    const char *key, Filename *value)
{
    write_setting_filename(handle, key, value);
}

void write_setting_fontspec(settings_w *handle,
			    const char *key, FontSpec *font)
{
    write_setting_fontspec(handle, key, font);
}

void close_settings_w(settings_w *handle)
{
    close_settings_w_PuTTY(handle);
}

/*
 * Read a saved session. The caller is expected to call
 * open_setting_r() to get a `void *' handle, then pass that to a
 * number of calls to read_setting_s() and read_setting_i(), and
 * then close it using close_settings_r().
 *
 * read_setting_s() returns a dynamically allocated string which the
 * caller must free. read_setting_filename() and
 * read_setting_fontspec() likewise return dynamically allocated
 * structures.
 *
 * If a particular string setting is not present in the session,
 * read_setting_s() can return NULL, in which case the caller
 * should invent a sensible default. If an integer setting is not
 * present, read_setting_i() returns its provided default.
 */

settings_r *open_settings_r(const char *sessionname)
{
    return open_settings_r_PuTTY(sessionname);
}

char *read_setting_s(settings_r *handle, const char *key)
{
    return read_setting_s_PuTTY(handle, key);
}

int read_setting_i(settings_r *handle, const char *key, int defvalue)
{
    return read_setting_i_PuTTY(handle, key, defvalue);
}

Filename *read_setting_filename(settings_r *handle, const char *key)
{
    return read_setting_filename_PuTTY(handle, key);
}

FontSpec *read_setting_fontspec(settings_r *handle, const char *key)
{
    return read_setting_fontspec_PuTTY(handle, key);
}

void close_settings_r(settings_r *handle)
{
    close_settings_r(handle);
}

/*
 * Delete a whole saved session.
 */

void del_settings(const char *sessionname)
{
    del_settings_PuTTY(sessionname);
}

/*
 * Enumerate all saved sessions.
 */

settings_e *enum_settings_start(void)
{
    return enum_settings_start_PuTTY();
}

bool enum_settings_next(settings_e *handle, strbuf *out)
{
    return enum_settings_next_PuTTY(handle, out);
}

void enum_settings_finish(settings_e *handle)
{
    enum_settings_finish_PuTTY(handle);
}

/* ----------------------------------------------------------------------
 * Functions to access PuTTY's host key database.
 */

/*
 * See if a host key matches the database entry. Return values can
 * be 0 (entry matches database), 1 (entry is absent in database),
 * or 2 (entry exists in database and is different).
 */

int check_stored_host_key(const char *hostname, int port,
			  const char *keytype, const char *key)
{
    return check_stored_host_key_PuTTY(hostname, port, keytype, key);
}

/*
 * Write a host key into the database, overwriting any previous
 * entry that might have been there.
 */

void store_host_key(const char *hostname, int port,
		    const char *keytype, const char *key)
{
    store_host_key_PuTTY(hostname, port, keytype, key);
}

/* ----------------------------------------------------------------------
 * Functions to access PuTTY's random number seed file.
 */

typedef void (*noise_consumer_t) (void *data, int len);

/*
 * Read PuTTY's random seed file and pass its contents to a noise
 * consumer function.
 */

void read_random_seed(noise_consumer_t consumer)
{
    read_random_seed_PuTTY(consumer);
}

/*
 * Write PuTTY's random seed file from a given chunk of noise.
 */

void write_random_seed(void *data, int len)
{
    write_random_seed_PuTTY(data, len);
}

/* ----------------------------------------------------------------------
 * Cleanup function: remove all of PuTTY's persistent state.
 */

void cleanup_all(void)
{
    cleanup_all_PuTTY();
}

#if 0
/* {{{ */
/*
 * Private subroutine prototypes
 */

static LONG wfsp_file_load_parse(char *line, WfspValueMap *value_map);
static bool wfsp_file_match(WfspKey *key);

/*
 * Private subroutines
 */

static LONG wfsp_file_load_parse(char *line, WfspValueMap *value_map)
{
    int name_begin, name_end;
    LONG rc;
    int type_begin, type_end;
    int value_begin, value_end;


    WFCP_DEBUG_ASSERT(line);
    WFCP_DEBUG_ASSERT(value_map);

    /*
     * XXX
     */

    if ((line[0] == '#') || (strlen(line) == 0)) {
	return ERROR_SUCCESS;
    }

    /*
     * XXX
     */

    name_begin = name_end = -1;
    sscanf(line, "%n%*[^=]%n=", &name_begin, &name_end);
    if ((name_begin == -1) || (name_end == -1)) {
	WFCP_DEBUG_FAIL();
	return ERROR_INVALID_PARAMETER;
    } else {
	value_begin = value_end = -1;
	sscanf(&line[name_end + 1], "\"%n%*[^\n]%n", &value_begin, &value_end);
	if ((value_begin == -1) || (value_end == -1)) {
	    type_begin = type_end = -1;
	    sscanf(&line[name_end + 1], "%n%*[^:]%n:%n%*[^\n]%n", &type_begin, &type_end, &value_begin, &value_end);
	    if ((type_begin == -1) || (type_end == -1)
	    ||  (value_begin == -1) || (value_end == -1)) {
		WFCP_DEBUG_FAIL();
		return ERROR_INVALID_PARAMETER;
	    } else {

		/*
		 * XXX
		 */

		if (strncmp(&line[name_end + 1 + type_begin], "dword", type_end - type_begin) == 0) {
		    if ((rc = wfsp_add_value(
			    value_map,
			    value_end - value_begin, REG_DWORD, &line[name_end + 1 + value_begin],
			    name_end - name_begin, &line[name_begin])) != ERROR_SUCCESS) {
			return rc;
		    }
		} else {
		    WFCP_DEBUG_FAIL();
		    return ERROR_INVALID_PARAMETER;
		}
	    }
	} else if ((rc = wfsp_add_value(
		value_map,
		value_end - value_begin - 1, REG_SZ, &line[name_end + 1 + value_begin],
		name_end - name_begin, &line[name_begin])) != ERROR_SUCCESS) {
	    return rc;
	}
    }

    return ERROR_SUCCESS;
}

static bool wfsp_file_match(WfspKey *key)
{
    static WfspMatchTable *match_table = NULL;
    LONG rc;


    WFCP_DEBUG_ASSERT(key);

    /*
     * XXX
     */

    if (!match_table) {
	if ((rc = wfsp_match_add(&match_table, "Software\\SimonTatham\\PuTTY\\CHMPath", false) != ERROR_SUCCESS)
	||  (rc = wfsp_match_add(&match_table, "Software\\SimonTatham\\PuTTY64\\CHMPath", false) != ERROR_SUCCESS)
	||  (rc = wfsp_match_add(&match_table, "SOFTWARE\\SimonTatham\\PuTTY\\Jumplist", false) != ERROR_SUCCESS)
	||  (rc = wfsp_match_add(&match_table, "SOFTWARE\\SimonTatham\\PuTTY\\SshHostKeys", false) != ERROR_SUCCESS)
	||  (rc = wfsp_match_add(&match_table, "SOFTWARE\\SimonTatham\\PuTTY\\Sessions", false) != ERROR_SUCCESS)
	||  (rc = wfsp_match_add(&match_table, "SOFTWARE\\SimonTatham\\PuTTY\\Sessions", true) != ERROR_SUCCESS)) {
	    wfsp_match_clear(&match_table);
	    WFCP_DEBUG_FAIL();
	    return false;
	}
    }

    /*
     * XXX
     */

    if ((key->root_hkey == HKEY_CURRENT_USER)
    &&  wfsp_match(match_table, key)) {
	return true;
    } else {
	return false;
    }
}
/* }}} */

/*
 * Public subroutines private to FySTY/winfrip_confstore*.c
 */

/* {{{ */
LONG wfsp_file_load(WfspHKEYMap *hkey_map, WfspKey **pkey_list, HKEY hKey)
{
    struct dirent *dire;
    DIR *dirp = NULL;
    FILE *file = NULL;
    char *file_buffer = NULL;
    char fname[MAX_PATH + 1];
    bool foundfl = false;
    WfspHKEY *hkey_item;
    WfspKey *key_item;
    char *line, *line_sep, *p;
    const char *name;
    LONG rc = ERROR_SUCCESS;
    struct stat statbuf;
    WfspHash subkey_hash;
    WfspKey *subkey_item_new = NULL, *subkey_item_last = NULL;
    size_t subkey_len;
    char *subkey_name;


    WFCP_DEBUG_ASSERT(hkey_map);
    WFCP_DEBUG_ASSERT(hkey_map->bitmap);
    WFCP_DEBUG_ASSERT(hkey_map->bitmap_bits);
    WFCP_DEBUG_ASSERT(pkey_list);
    WFCP_DEBUG_ASSERT(hKey);

    /*
     * XXX
     */

    if (((size_t)hKey == 0)
    ||  ((size_t)hKey > hkey_map->bitmap_bits)
    ||  !(hkey_item = hkey_map->hkeyv[(size_t)hKey - 1])) {
	WFCP_DEBUG_FAIL();
	return ERROR_INVALID_HANDLE;
    }

    /*
     * XXX
     */

    for (key_item = *pkey_list; key_item != NULL; key_item = key_item->next) {
	if ((hkey_item->root_hkey == key_item->root_hkey)
	&&  (hkey_item->key_hash == key_item->key_hash)) {
	    foundfl = true; break;
	}
    }
    if (!foundfl) {
	WFCP_DEBUG_FAIL();
	return ERROR_INVALID_HANDLE;
    }

    /*
     * XXX
     */

    if (!wfsp_file_match(key_item)) {
	rc = ERROR_FILE_NOT_FOUND; goto err;
    } else {
	wfsp_get_basename(key_item, &name, NULL);
	ZeroMemory(fname, sizeof(fname));
	snprintf(fname, sizeof(fname), "%s.ini", name);
    }

    /*
     * XXX
     */

    if (stat(fname, &statbuf) < 0) {
	if (errno != ENOENT) {
	    rc = ERROR_INVALID_HANDLE; goto err;
	}
    } else if (!(file = fopen(fname, "rb"))) {
	rc = ERROR_FILE_NOT_FOUND; goto err;
    } else if (!(file_buffer = snewn(statbuf.st_size + 1, char))) {
	rc = ERROR_NOT_ENOUGH_MEMORY; goto err;
    } else if (fread(file_buffer, statbuf.st_size, 1, file) != 1) {
	rc = ERROR_HANDLE_EOF; goto err;
    } else {

	/*
	 * XXX
	 */

	wfsp_clear_values(key_item);
	file_buffer[statbuf.st_size] = '\0', p = &file_buffer[0];
	do {
	    line = p; line_sep = strchr(line, '\n');
	    if (line_sep) {
		*line_sep = '\0', p = line_sep + 1;
		if ((line_sep[-1] == '\r')
		&&  (&line_sep[-1] >= line)) {
		    line_sep[-1] = '\0';
		}
	    }
	    (void)wfsp_file_load_parse(line, &(*pkey_list)->value_map);
	} while (line_sep && (p < &file_buffer[statbuf.st_size]));
    }

    /*
     * XXX
     */

    if (stat(name, &statbuf) <00) {
	if (errno != ENOENT) {
	    rc = ERROR_INVALID_HANDLE; goto err;
	}
    } else if (S_ISDIR(statbuf.st_mode)) {
	if (!(dirp = opendir(name))) {
	    rc = ERROR_INVALID_HANDLE; goto err;
	} else {
	    errno = 0;
	    while ((dire = readdir(dirp))) {

		/*
		 * XXX
		 */

		if ((dire->d_name[0] == '.')
		&&  (dire->d_name[1] == '\0')) {
		    continue;
		} else if ((dire->d_name[0] == '.')
		       &&  (dire->d_name[1] == '.')
		       &&  (dire->d_name[2] == '\0')) {
		    continue;
		}

		/*
		 * XXX
		 */

		subkey_len = strlen(dire->d_name);
		if (!(subkey_item_new = snew(WfspKey))) {
		    rc = ERROR_NOT_ENOUGH_MEMORY; goto err;
		} else if (!(subkey_name = snewn(subkey_len + 1, char))) {
		    sfree(subkey_item_new);
		    rc = ERROR_NOT_ENOUGH_MEMORY; goto err;
		} else {
		    strncpy(subkey_name, dire->d_name, subkey_len); subkey_name[subkey_len] = '\0';
		    if ((p = strstr(subkey_name, ".ini"))) {
			*p = '\0';
		    }
		    subkey_hash = wfsp_fnv32a(subkey_name, subkey_len, sizeof(subkey_hash) * 8);
		}

		/*
		 * XXX
		 */

		*subkey_item_new = WFCP_KEY_EMPTY;
		if (subkey_item_last) {
		    subkey_item_last->next = subkey_item_new;
		} else {
		    key_item->subkey_list = subkey_item_new;
		}

		subkey_item_last = subkey_item_new;
		subkey_item_new->root_hkey = key_item->root_hkey;
		subkey_item_new->root_name = key_item->root_name;
		subkey_item_new->key_name = subkey_name;
		subkey_item_new->key_hash = subkey_hash;
	    }
	    if (errno != 0) {
		rc = ERROR_INVALID_HANDLE; goto err;
	    }
	}
    }

    /*
     * XXX
     */

out:
    if (dirp) {
	closedir(dirp);
    }
    if (file) {
	fclose(file);
    }
    if (file_buffer) {
	sfree(file_buffer);
    }
    return rc;

err:
    if (foundfl) {
	wfsp_clear_subkeys(key_item);
	wfsp_clear_values(key_item);
    }
    goto out;
}

LONG wfsp_file_save(WfspHKEYMap *hkey_map, WfspKey *key_list, HKEY hKey)
{
    // FIXME TODO XXX
    return ERROR_INVALID_FUNCTION;
}
/* }}} */
#endif
