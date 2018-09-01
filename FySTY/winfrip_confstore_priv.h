/*
 * winfrip_confstore_priv.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#ifndef PUTTY_WINFRIP_CONFSTORE_PRIV_H
#define PUTTY_WINFRIP_CONFSTORE_PRIV_H

/*
 * Preprocessor macros private to FySTY/winfrip_confstore*.c
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
 * Type definitions private to FySTY/winfrip_confstore*.c prototypes
 */

/*
 * XXX document
 */
typedef struct WinFrippConfStoreHKEYSubKey {
    HKEY				hKey, hKey_parent;
    char *				lpSubKey;
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
 * Public subroutines private to FySTY/winfrip_confstore*.c prototypes
 */

/*
 * XXX document
 */
void winfripp_confstore_debugf(const char *fmt, const char *file, const char *func, int line, ...);

/*
 * XXX document
 */
BOOL winfripp_confstore_map_hkey_add(uint64_t *bitmap, size_t *pbitmap_bits, WinFrippConfStoreHKEYSubKey *map, size_t map_len, HKEY hKey_parent, LPCSTR lpSubKey, HKEY *phKey);
BOOL winfripp_confstore_map_hkey_del(uint64_t *bitmap, size_t *pbitmap_bits, WinFrippConfStoreHKEYSubKey *map, size_t map_len, HKEY hKey);
BOOL winfripp_confstore_map_hkey_get(WinFrippConfStoreHKEYSubKey *map, size_t map_len, HKEY hKey, const char **pfull_key);

/*
 * XXX document
 */
BOOL winfripp_confstore_map_value_del(WinFrippConfStoreKeyMap *map, char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, LPCSTR lpValueName);
BOOL winfripp_confstore_map_value_get(WinFrippConfStoreKeyMap *map, char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, LPCSTR lpValueName, LPBYTE lpData, LPDWORD lpcbData);
BOOL winfripp_confstore_map_value_hash(char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, size_t nbits, LPCSTR lpValueName, uint32_t *phash);
BOOL winfripp_confstore_map_value_set(WinFrippConfStoreKeyMap *map, char *full_keyval_buf, size_t full_keyval_buf_size, const char *key, LPCSTR lpValueName, DWORD dwType, BYTE *lpData, DWORD cbData);

/*
 * XXX document
 */
void winfripp_confstore_free_value_map(WinFrippConfStoreKeyMap *map);
BOOL winfripp_confstore_init_value_map(size_t nmap_bits, WinFrippConfStoreKeyMap **pmap);

#endif
