/*
 * winfrip_confstore.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "FySTY/winfrip.h"
#include "FySTY/winfrip_priv.h"

/*
 * Preprocessor macros
 */

/*
 * XXX document
 */
#ifdef WINFRIP_DEBUG
#define WINFRIP_CONFSTORE_DEBUG
#endif

/*
 * XXX document
 */
#ifdef WINFRIP_CONFSTORE_DEBUG
#define WINFRIPP_CONFSTORE_DEBUG_ASSERT(expr) do {				\
	if (!(expr)) {								\
	    WINFRIPP_CONFSTORE_DEBUGF("assertion failure: %s\n"			\
				      "GetLastError(): 0x%08x",			\
				      #expr, GetLastError());			\
	}									\
} while (0)
#define WINFRIPP_CONFSTORE_DEBUG_ASSERTF(expr, fmt, ...) do {			\
	if (!(expr)) {								\
	    WINFRIPP_CONFSTORE_DEBUGF(fmt "\n" "assertion failure: %s\n"	\
				      "GetLastError(): 0x%08x",			\
				      ##__VA_ARGS__, #expr, GetLastError());	\
	}									\
} while (0)
#define WINFRIPP_CONFSTORE_DEBUG_FAIL()						\
	WINFRIPP_CONFSTORE_DEBUGF("failure condition", __FILE__, __func__, __LINE__)
#define WINFRIPP_CONFSTORE_DEBUGF(fmt, ...)					\
	winfripp_confstore_debugf(fmt, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#else
#define WINFRIPP_CONFSTORE_DEBUG_ASSERT(expr)					\
	(void)(expr)
#define WINFRIPP_CONFSTORE_DEBUG_ASSERTF(expr, fmt, ...)			\
	(void)(expr)
#define WINFRIPP_CONFSTORE_DEBUG_FAIL()
#define WINFRIPP_CONFSTORE_DEBUGF(fmt, ...)
#endif

/*
 * XXX document
 */
#define WINFRIPP_CONFSTORE_IF_HKEY_PREDEF(hkey)					\
    if (((hkey) == HKEY_CLASSES_ROOT) ||					\
	((hkey) == HKEY_CURRENT_CONFIG) ||					\
	((hkey) == HKEY_CURRENT_USER) ||					\
	((hkey) == HKEY_LOCAL_MACHINE) ||					\
	((hkey) == HKEY_USERS))

/*
 * Type definitions
 */

/*
 * XXX document
 */
typedef struct WinFrippConfStoreHKEYSubKey {
    HKEY				hKey, hKey_parent;
    LPCSTR				lpSubKey;
} WinFrippConfStoreHKEYSubKey;

/*
 * XXX document
 */
typedef struct WinFrippConfStoreKeyValue {
    DWORD				dwType;
    char *				key;
    uint32_t				key_hash;
    BYTE *				lpData;
    DWORD				cbData;
    struct WinFrippConfStoreKeyValue *	next;
} WinFrippConfStoreKeyValue;

/*
 * XXX document
 */
typedef struct WinFrippConfStoreKeyMap {
    size_t				nbits;
    WinFrippConfStoreKeyValue **	map;
} WinFrippConfStoreKeyMap;

/*
 * {External,Static} variables
 */

/*
 * XXX document
 */
extern Conf *conf;

/*
 * XXX document
 */
static WinFrippConfStoreHKEYSubKey winfripp_confstore_hkey_map[16] = {{NULL, NULL, NULL}};
static size_t winfripp_confstore_hkey_map_len = 16;

/*
 * XXX document
 */
static char winfripp_confstore_full_key_buf[256] = {'\0',};
static size_t winfripp_confstore_full_key_buf_size = 256;
static char winfripp_confstore_full_keyval_buf[256] = {'\0',};
static size_t winfripp_confstore_full_keyval_buf_size = 256;

/*
 * XXX document
 */
static WinFrippConfStoreKeyMap winfripp_confstore_key_map = {0, NULL};

/*
 * Private subroutine prototypes
 */

void winfripp_confstore_debugf(const char *fmt, const char *file, const char *func, int line, ...);
static BOOL winfripp_confstore_map_hkey_add(WinFrippConfStoreHKEYSubKey *map, size_t map_len, HKEY hKey, HKEY hKey_parent, LPCSTR lpSubKey);
static BOOL winfripp_confstore_map_hkey_del(WinFrippConfStoreHKEYSubKey *map, size_t map_len, HKEY hKey, LPCSTR lpSubKey);
static BOOL winfripp_confstore_map_hkey_get(WinFrippConfStoreHKEYSubKey *map, size_t map_len, char *full_key_buf, size_t full_key_buf_size, HKEY hKey, const char **pfull_key);
static BOOL winfripp_confstore_map_value_del(WinFrippConfStoreKeyMap *map, char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, LPCSTR lpValueName);
static BOOL winfripp_confstore_map_value_get(WinFrippConfStoreKeyMap *map, char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, LPCSTR lpValueName, LPBYTE lpData, LPDWORD lpcbData);
static BOOL winfripp_confstore_map_value_hash(char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, size_t nbits, LPCSTR lpValueName, uint32_t *phash);
static BOOL winfripp_confstore_map_value_set(WinFrippConfStoreKeyMap *map, char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, LPCSTR lpValueName, DWORD dwType, BYTE *lpData, DWORD cbData);
static void winfripp_confstore_free_value_map(WinFrippConfStoreKeyMap *map);
static BOOL winfripp_confstore_init_value_map(size_t nmap_bits, WinFrippConfStoreKeyMap **pmap);

/*
 * Private subroutines
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

static BOOL winfripp_confstore_map_hkey_add(WinFrippConfStoreHKEYSubKey *map, size_t map_len, HKEY hKey, HKEY hKey_parent, LPCSTR lpSubKey)
{
    size_t nmap_item;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map_len > 0);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpSubKey);

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
	if (!map[nmap_item].hKey) {
	    map[nmap_item].hKey = hKey;
	    map[nmap_item].hKey_parent = hKey_parent;
	    map[nmap_item].lpSubKey = lpSubKey;
	    WINFRIPP_CONFSTORE_DEBUGF("hKey=%p, hKey_parent=%p, lpSubKey=`%s'", hKey, hKey_parent, lpSubKey);
	    return TRUE;
	}
    }

    /*
     * XXX document
     */
    WINFRIPP_CONFSTORE_DEBUG_FAIL();
    return FALSE;
}

static BOOL winfripp_confstore_map_hkey_del(WinFrippConfStoreHKEYSubKey *map, size_t map_len, HKEY hKey, LPCSTR lpSubKey)
{
    size_t nmap_item;


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
	if (lpSubKey) {
	    if (map[nmap_item].hKey_parent == hKey) {
		WINFRIPP_CONFSTORE_DEBUGF("hKey=%p, hKey_parent=%p, lpSubKey=`%s'",
					  hKey, map[nmap_item].hKey_parent, lpSubKey);
		map[nmap_item].hKey = NULL;
		map[nmap_item].hKey_parent = NULL;
		map[nmap_item].lpSubKey = NULL;
		return TRUE;
	    }
	} else {
	    if (map[nmap_item].hKey == hKey) {
		WINFRIPP_CONFSTORE_DEBUGF("hKey=%p, hKey_parent=%p, lpSubKey=NULL",
					  hKey, map[nmap_item].hKey_parent);
		map[nmap_item].hKey = NULL;
		map[nmap_item].hKey_parent = NULL;
		map[nmap_item].lpSubKey = NULL;
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

static BOOL winfripp_confstore_map_hkey_get(WinFrippConfStoreHKEYSubKey *map, size_t map_len, char *full_key_buf, size_t full_key_buf_size, HKEY hKey, const char **pfull_key)
{
    HKEY hKey_chase;
    size_t full_key_len_cur;
    size_t niter, nmap_item;
    size_t subkey_len;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(map_len > 0);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(full_key_buf);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(full_key_buf_size);
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
    hKey_chase = hKey; full_key_len_cur = 0;
    ZeroMemory(full_key_buf, full_key_buf_size);
    for (niter = 0; niter < 16; niter++) {
	for (nmap_item = 0; nmap_item < map_len; nmap_item++) {
	    if (map[nmap_item].hKey == hKey_chase) {
		/*
		 * XXX document
		 */
		subkey_len = strlen(map[nmap_item].lpSubKey);
		if ((full_key_len_cur + subkey_len + 1) > full_key_buf_size) {
		    WINFRIPP_CONFSTORE_DEBUG_FAIL();
		    return FALSE;
		} else {
		    memcpy(&full_key_buf[full_key_len_cur],
			   map[nmap_item].lpSubKey, subkey_len);
		    full_key_len_cur += subkey_len;
		}

		/*
		 * XXX document
		 */
		if (!map[nmap_item].hKey_parent) {
		    full_key_buf[full_key_len_cur] = '\0';
		    *pfull_key = full_key_buf;
		    return TRUE;
		} else {
		    hKey_chase = map[nmap_item].hKey_parent;
		    break;
		}
	    }
	}
    }

    /*
     * XXX document
     */
    WINFRIPP_CONFSTORE_DEBUG_FAIL();
    return FALSE;
}

static BOOL winfripp_confstore_map_value_del(WinFrippConfStoreKeyMap *map, char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, LPCSTR lpValueName)
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

static BOOL winfripp_confstore_map_value_get(WinFrippConfStoreKeyMap *map, char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, LPCSTR lpValueName, LPBYTE lpData, LPDWORD lpcbData)
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

static BOOL winfripp_confstore_map_value_hash(char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, size_t nbits, LPCSTR lpValueName, uint32_t *phash)
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

static BOOL winfripp_confstore_map_value_set(WinFrippConfStoreKeyMap *map, char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, LPCSTR lpValueName, DWORD dwType, BYTE *lpData, DWORD cbData)
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

static void winfripp_confstore_free_value_map(WinFrippConfStoreKeyMap *map)
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

static BOOL winfripp_confstore_init_value_map(size_t nmap_bits, WinFrippConfStoreKeyMap **pmap)
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

/*
 * Key {creation,open} methods
 */

LONG winfrip_confstore_RegCreateKey(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult)
{
    LONG rc;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpSubKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(phkResult);

    /*
     * XXX document
     */
    rc = RegCreateKeyA(hKey, lpSubKey, phkResult);
    if (rc == ERROR_SUCCESS) {
	winfripp_confstore_map_hkey_add(winfripp_confstore_hkey_map,
					winfripp_confstore_hkey_map_len,
					*phkResult, hKey, lpSubKey);
    }
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(rc == ERROR_SUCCESS);
    return rc;
}

LONG winfrip_confstore_RegCreateKeyEx(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition)
{
    LONG rc;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpSubKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(phkResult);

    /*
     * XXX document
     */
    rc = RegCreateKeyExA(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
    if (rc == ERROR_SUCCESS) {
	winfripp_confstore_map_hkey_add(winfripp_confstore_hkey_map,
					winfripp_confstore_hkey_map_len,
					*phkResult, hKey, lpSubKey);
    }
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(rc == ERROR_SUCCESS);
    return rc;
}

LONG winfrip_confstore_RegOpenKey(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult)
{
    LONG rc;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpSubKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(phkResult);

    /*
     * XXX document
     */
    rc = RegOpenKeyA(hKey, lpSubKey, phkResult);
    if (rc == ERROR_SUCCESS) {
	winfripp_confstore_map_hkey_add(winfripp_confstore_hkey_map,
					winfripp_confstore_hkey_map_len,
					*phkResult, hKey, lpSubKey);
    }
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(rc == ERROR_SUCCESS);
    return rc;
}

/*
 * Key {closing,deletion,enumeration} methods
 */

LONG winfrip_confstore_RegCloseKey(HKEY hKey)
{
    LONG rc;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);

    /*
     * XXX document
     */
    rc = RegCloseKey(hKey);
    if (rc == ERROR_SUCCESS) {
	winfripp_confstore_map_hkey_del(winfripp_confstore_hkey_map,
					winfripp_confstore_hkey_map_len,
					hKey, NULL);
    }
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(rc == ERROR_SUCCESS);
    return rc;
}

LONG winfrip_confstore_RegDeleteKey(HKEY hKey, LPCSTR lpSubKey)
{
    LONG rc;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpSubKey);

    /*
     * XXX document
     */
    rc = RegDeleteKeyA(hKey, lpSubKey);
    if (rc == ERROR_SUCCESS) {
	winfripp_confstore_map_hkey_del(winfripp_confstore_hkey_map,
					winfripp_confstore_hkey_map_len,
					hKey, lpSubKey);
    }
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(rc == ERROR_SUCCESS);
    return rc;
}

LONG winfrip_confstore_RegEnumKey(HKEY hKey, DWORD dwIndex, LPSTR lpName, DWORD cchName)
{
    LONG rc;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpName);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(cchName > 0);

    /*
     * XXX document
     */
    rc = RegEnumKeyA(hKey, dwIndex, lpName, cchName);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(rc == ERROR_SUCCESS);
    return rc;
}

/*
 * Value {deletion,querying,setting} methods
 */

LONG winfrip_confstore_RegDeleteValue(HKEY hKey, LPCSTR lpValueName)
{
    LONG rc;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpValueName);

    /*
     * XXX document
     */
    rc = RegDeleteValueA(hKey, lpValueName);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(rc == ERROR_SUCCESS);
    return rc;
}

LONG winfrip_confstore_RegQueryValueEx(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
    LONG rc;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpValueName);

    /*
     * XXX document
     */
    rc = RegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(rc == ERROR_SUCCESS);
    return rc;
}

LONG winfrip_confstore_RegSetValueEx(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE *lpData, DWORD cbData)
{
    LONG rc;


    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpValueName);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpData);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(cbData > 0);

    /*
     * XXX document
     */
    rc = RegSetValueExA(hKey, lpValueName, Reserved, dwType, lpData, cbData);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(rc == ERROR_SUCCESS);
    return rc;
}
