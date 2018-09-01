/*
 * winfrip_confstore.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "FySTY/winfrip.h"
#include "FySTY/winfrip_priv.h"
#include "FySTY/winfrip_confstore_priv.h"

/*
 * Public subroutines private to FySTY/winfrip_confstore*.c
 */

#ifdef WINFRIP_DEBUG
void winfripp_confstore_debugf(const char *fmt, const char *file, const char *func, int line, ...)
{
    va_list ap;


    /*
     * XXX document
     */
    fprintf(stderr, "In %s:%d:%s():\n", file, line, func);
    va_start(ap, line);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n\n");
}
#endif

/*
 * XXX document
 */

BOOL winfripp_confstore_map_hkey_add(uint64_t *bitmap, size_t *pbitmap_bits, WinFrippConfStoreHKEYSubKey *map, size_t map_len, HKEY hKey_parent, LPCSTR lpSubKey, HKEY *phKey)
{
    char *lpSubKey_new;
    HKEY hKey_new;
    size_t nbit_cur, nmap_item;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(bitmap);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(pbitmap_bits);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map_len > 0);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpSubKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(phKey);

    /*
     * XXX document
     */
    if (hKey_parent && hKey_parent == HKEY_PERFORMANCE_DATA) {
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return FALSE;
    }

    /*
     * XXX document
     */
    for (hKey_new = (HKEY)0, nbit_cur = 1; nbit_cur <= *pbitmap_bits; nbit_cur++) {
	if (!(*bitmap & (1 << (nbit_cur - 1)))) {
	    hKey_new = (HKEY)nbit_cur; break;
	}
    }
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey_new != (HKEY)0);
    if (hKey_new == (HKEY)0) {
	return FALSE;
    }

    /*
     * XXX document
     */
    for (nmap_item = 0; nmap_item < map_len; nmap_item++) {
	if (!map[nmap_item].hKey) {
	    if (!(lpSubKey_new = dupstr(lpSubKey))) {
		return FALSE;
	    } else {
		map[nmap_item].hKey = hKey_new;
		map[nmap_item].hKey_parent = hKey_parent;
		map[nmap_item].lpSubKey = lpSubKey_new;
		*bitmap |= (1 << (nbit_cur - 1));
		return TRUE;
	    }
	}
    }

    /*
     * XXX document
     */
    WINFRIPP_CONFSTORE_DEBUG_FAIL();
    return FALSE;
}

BOOL winfripp_confstore_map_hkey_del(uint64_t *bitmap, size_t *pbitmap_bits, WinFrippConfStoreHKEYSubKey *map, size_t map_len, HKEY hKey)
{
    size_t nbit_old, nmap_item;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(bitmap);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(pbitmap_bits);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map_len > 0);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);

    /*
     * XXX document
     */
    if (hKey == HKEY_PERFORMANCE_DATA) {
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return FALSE;
    }

    /*
     * XXX document
     */
    for (nmap_item = 0; nmap_item < map_len; nmap_item++) {
	if (map[nmap_item].hKey == hKey) {
	    nbit_old = (size_t)map[nmap_item].hKey;
	    WINFRIPP_CONFSTORE_DEBUG_ASSERT(nbit_old <= *pbitmap_bits);
	    if (nbit_old <= *pbitmap_bits) {
		*bitmap &= ~(1 << (nbit_old - 1));
	    }
	    map[nmap_item].hKey = NULL;
	    map[nmap_item].hKey_parent = NULL;
	    sfree(map[nmap_item].lpSubKey);
	    map[nmap_item].lpSubKey = NULL;
	    return TRUE;
	}
    }

    /*
     * XXX document
     */
    WINFRIPP_CONFSTORE_DEBUG_FAIL();
    return FALSE;
}

BOOL winfripp_confstore_map_hkey_get(WinFrippConfStoreHKEYSubKey *map, size_t map_len, HKEY hKey, const char **pfull_key)
{
    char *full_key, *full_key_new;
    size_t full_key_len;
    HKEY hKey_cur;
    size_t key_len;
    size_t niter, nmap_item;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map_len > 0);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(pfull_key);

    /*
     * XXX document
     */
    if (hKey == HKEY_PERFORMANCE_DATA) {
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return FALSE;
    }

    /*
     * XXX document
     */
    full_key = NULL; full_key_len = 0; hKey_cur = hKey;
    for (niter = 0; niter < 16; niter++) {
	for (nmap_item = 0; nmap_item < map_len; nmap_item++) {
	    if (map[nmap_item].hKey == hKey_cur) {
		key_len = strlen(map[nmap_item].lpSubKey);
		WINFRIPP_CONFSTORE_DEBUG_ASSERT(key_len);
		if (key_len) {
		    /*
		     * XXX document
		     */
		    if (!(full_key_new = sresize(full_key, full_key_len + 1 + key_len + 1, char))) {
			if (full_key) {
			    sfree(full_key);
			}
			WINFRIPP_CONFSTORE_DEBUG_FAIL();
			return FALSE;
		    } else {
			full_key = full_key_new;
			memmove(&full_key[full_key_len], full_key, full_key_len);
			memcpy(full_key, map[nmap_item].lpSubKey, key_len);
			full_key[key_len] = '\\';
			full_key_len += full_key_len + 1 + key_len;
		    }
		}

		/*
		 * XXX document
		 */
		if (map[nmap_item].hKey_parent == (HKEY)0) {
		    full_key[full_key_len] = '\0';
		    *pfull_key = full_key;
		    return TRUE;
		} else {
		    hKey_cur = map[nmap_item].hKey_parent;
		}
	    }
	}
    }

    /*
     * XXX document
     */
    if (full_key) {
	sfree(full_key);
    }
    WINFRIPP_CONFSTORE_DEBUG_FAIL();
    return FALSE;
}

/*
 * XXX document
 */

BOOL winfripp_confstore_map_value_del(WinFrippConfStoreKeyMap *map, char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, LPCSTR lpValueName)
{
    uint32_t full_key_hash;
    WinFrippConfStoreKeyValue *item, *item_prev, *map_item;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map->nbits > 0);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map->map);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(full_keyval_buf);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(full_keyval_buf_size);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(key);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpValueName);

    /*
     * XXX document
     */
    if (!winfripp_confstore_map_value_hash(full_keyval_buf, full_keyval_buf_size,
					   key, map->nbits, lpValueName, &full_key_hash)) {
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return FALSE;
    } else {
	map_item = map->map[full_key_hash];
    }

    /*
     * XXX document
     */
    for (item = map_item, item_prev = item; item;
	 item_prev = item, item = item->next)
    {
	if (item->key_hash == full_key_hash) {
	    if (item_prev != item) {
		item_prev->next = item->next;
	    } else if (item_prev == item) {
		map->map[full_key_hash] = item->next;
	    }
	    sfree(item->key); sfree(item->lpData);
	    sfree(item);
	    return TRUE;
	}
    }

    /*
     * XXX document
     */
    WINFRIPP_CONFSTORE_DEBUG_FAIL();
    return FALSE;
}

BOOL winfripp_confstore_map_value_get(WinFrippConfStoreKeyMap *map, char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, LPCSTR lpValueName, LPBYTE lpData, LPDWORD lpcbData)
{
    uint32_t full_key_hash;
    WinFrippConfStoreKeyValue *item, *map_item;
    size_t value_size;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map->nbits > 0);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map->map);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(full_keyval_buf);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(full_keyval_buf_size);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(key);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpValueName);

    /*
     * XXX document
     */
    if (!winfripp_confstore_map_value_hash(full_keyval_buf, full_keyval_buf_size,
					   key, map->nbits, lpValueName, &full_key_hash)) {
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return FALSE;
    } else {
	map_item = map->map[full_key_hash];
    }

    /*
     * XXX document
     */
    for (item = map_item; item; item = item->next) {
	if (item->key_hash == full_key_hash) {
	    value_size = item->cbData;
	    if (lpData && lpcbData) {
		if (value_size > *lpcbData) {
		    *lpcbData = value_size;
		} else {
		    memcpy(lpData, item->lpData, value_size);
		}
	    } else if (!lpData && lpcbData) {
		*lpcbData = value_size;
	    }
	    return TRUE;
	}
    }

    /*
     * XXX document
     */
    WINFRIPP_CONFSTORE_DEBUG_FAIL();
    return FALSE;
}

BOOL winfripp_confstore_map_value_hash(char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, size_t nbits, LPCSTR lpValueName, uint32_t *phash)
{
    uint32_t fnv32_hash = 0x811c9dc5, fnv32_prime = 0x1000193;
    size_t full_keyval_len, key_len, lpValueName_len;
    uint8_t *p, *q;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(full_keyval_buf);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(full_keyval_buf_size > 0);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(key);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(nbits > 0);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpValueName);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(phash);

    /*
     * XXX document
     */
    key_len = strlen(key);
    lpValueName_len = strlen(lpValueName);
    full_keyval_len = key_len + lpValueName_len;
    if (full_keyval_len && ((full_keyval_len + 1) <= full_keyval_buf_size)) {
	memcpy(full_keyval_buf, key, key_len);
	memcpy(full_keyval_buf + key_len, lpValueName, lpValueName_len);
	full_keyval_buf[full_keyval_len] = '\0';
    } else {
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return FALSE;
    }

    /*
     * XXX document
     */
    p = (uint8_t *)full_keyval_buf; q = p + full_keyval_len;
    while (p < q) {
	fnv32_hash ^= (uint32_t)*p++;
	fnv32_hash *= fnv32_prime;
    }
    *phash = fnv32_hash & ((2 << (nbits - 1)) - 1);
    return TRUE;
}

BOOL winfripp_confstore_map_value_set(WinFrippConfStoreKeyMap *map, char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, LPCSTR lpValueName, DWORD dwType, BYTE *lpData, DWORD cbData)
{
    uint32_t full_key_hash;
    WinFrippConfStoreKeyValue *item, *map_item, *new_item;
    BYTE *new_lpData;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map->nbits > 0);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map->map);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(full_keyval_buf);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(full_keyval_buf_size);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(key);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpValueName);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpData);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(cbData > 0);

    /*
     * XXX document
     */
    if (!winfripp_confstore_map_value_hash(full_keyval_buf, full_keyval_buf_size,
					   key, map->nbits, lpValueName, &full_key_hash)) {
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return FALSE;
    } else {
	map_item = map->map[full_key_hash];
    }

    /*
     * XXX document
     */
    if (!(new_lpData = snewn(cbData, BYTE))) {
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return FALSE;
    } else {
	memcpy(new_lpData, lpData, cbData);
    }

    /*
     * XXX document
     */
    if (!(new_item = snew(WinFrippConfStoreKeyValue))) {
	sfree(new_lpData);
	return FALSE;
    } else {
	new_item->dwType = dwType;
	new_item->key = dupstr(full_keyval_buf);
	new_item->key_hash = full_key_hash;
	new_item->lpData = new_lpData;
	new_item->cbData = cbData;
	new_item->next = NULL;
    }

    /*
     * XXX document
     */
    if (!map_item) {
	map->map[full_key_hash] = new_item;
    } else for (item = map_item; item; item = item->next) {
	if (!item->next) {
	    item->next = new_item;
	}
    }
    return TRUE;
}

/*
 * XXX document
 */

void winfripp_confstore_free_value_map(WinFrippConfStoreKeyMap *map)
{
    WinFrippConfStoreKeyValue *item, *item_next, *map_item;
    size_t nmap_item, nmap_items;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map->nbits > 0);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map->map);

    /*
     * XXX document
     */
    if (map->nbits) {
	nmap_items = 2 << (map->nbits - 1);
	for (nmap_item = 0; nmap_item < nmap_items; nmap_item++) {
	    map_item = map->map[nmap_item];
	    if (map_item) {
		for (item = map_item, item_next = item->next; item; item = item_next) {
		    item_next = item->next;
		    sfree(item->key);
		    sfree(item->lpData);
		    sfree(item);
		}
	    }
	    sfree(map_item);
	}
    }
    map->nbits = 0;
    sfree(map->map);
}

BOOL winfripp_confstore_init_value_map(size_t nmap_bits, WinFrippConfStoreKeyMap **pmap)
{
    WinFrippConfStoreKeyMap *new_map;
    WinFrippConfStoreKeyValue **new_map_items;
    size_t nmap_items;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(nmap_bits > 0);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(pmap);

    /*
     * XXX document
     */
    nmap_items = 2 << (nmap_bits - 1);
    if (!(new_map = snew(WinFrippConfStoreKeyMap))) {
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return FALSE;
    } else if (!(new_map_items = snewn(nmap_items, WinFrippConfStoreKeyValue *))) {
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	sfree(new_map);
	return FALSE;
    } else {
	ZeroMemory(new_map_items, nmap_items * sizeof(*new_map_items));
	new_map->nbits = nmap_bits;
	new_map->map = new_map_items;
	*pmap = new_map;
	return TRUE;
    }
}
