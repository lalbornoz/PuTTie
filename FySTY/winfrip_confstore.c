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
 * Static variables
 */

/*
 * XXX document
 */
static WinFripGeneralStoreBackEnd winfripp_confstore_backend = WINFRIP_GENERAL_STORE_BACKEND_REGISTRY;

/*
 * XXX document
 */
static WinFrippConfStoreHKEYSubKey winfripp_confstore_hkey_map[64] = {{NULL, NULL, NULL}};
static size_t winfripp_confstore_hkey_map_len = 64;

/*
 * XXX document
 */
static uint64_t winfripp_confstore_hkey_bitmap = 0ULL;
static size_t winfripp_confstore_hkey_bitmap_bits = 64;

/*
 * Public subroutines
 */

WinFripGeneralStoreBackEnd winfrip_confstore_backend_get(void)
{
    /*
     * XXX document
     */
    return winfripp_confstore_backend;
}

void winfrip_confstore_backend_set(WinFripGeneralStoreBackEnd new_backend)
{
    /*
     * XXX document
     */
    winfripp_confstore_backend = new_backend;
}

/*
 * Key {creation,open} methods
 */

LONG winfrip_confstore_RegCreateKey(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult)
{
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpSubKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(phkResult);

    /*
     * XXX document
     */
    switch (winfrip_confstore_backend_get()) {
    default:
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return ERROR_INVALID_FUNCTION;

    case WINFRIP_GENERAL_STORE_BACKEND_REGISTRY:
	return RegCreateKeyA(hKey, lpSubKey, phkResult);

    case WINFRIP_GENERAL_STORE_BACKEND_EPHEMERAL:
    case WINFRIP_GENERAL_STORE_BACKEND_FILE:
	if (winfripp_confstore_map_hkey_add(&winfripp_confstore_hkey_bitmap,
					    &winfripp_confstore_hkey_bitmap_bits,
					    winfripp_confstore_hkey_map,
					    winfripp_confstore_hkey_map_len,
					    hKey, lpSubKey, phkResult)) {
	    return ERROR_SUCCESS;
	} else {
	    return ERROR_NOT_ENOUGH_MEMORY;
	}
    }
}

LONG winfrip_confstore_RegCreateKeyEx(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition)
{
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(phkResult);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpSubKey);

    /*
     * XXX document
     */
    switch (winfrip_confstore_backend_get()) {
    default:
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return ERROR_INVALID_FUNCTION;

    case WINFRIP_GENERAL_STORE_BACKEND_REGISTRY:
	return RegCreateKeyExA(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);

    case WINFRIP_GENERAL_STORE_BACKEND_EPHEMERAL:
    case WINFRIP_GENERAL_STORE_BACKEND_FILE:
	if (winfripp_confstore_map_hkey_add(&winfripp_confstore_hkey_bitmap,
					    &winfripp_confstore_hkey_bitmap_bits,
					    winfripp_confstore_hkey_map,
					    winfripp_confstore_hkey_map_len,
					    hKey, lpSubKey, phkResult)) {
	    return ERROR_SUCCESS;
	} else {
	    return ERROR_NOT_ENOUGH_MEMORY;
	}
    }
}

LONG winfrip_confstore_RegOpenKey(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult)
{
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpSubKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(phkResult);

    /*
     * XXX document
     */
    switch (winfrip_confstore_backend_get()) {
    default:
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return ERROR_INVALID_FUNCTION;

    case WINFRIP_GENERAL_STORE_BACKEND_REGISTRY:
	return RegOpenKeyA(hKey, lpSubKey, phkResult);

    case WINFRIP_GENERAL_STORE_BACKEND_EPHEMERAL:
    case WINFRIP_GENERAL_STORE_BACKEND_FILE:
	if (winfripp_confstore_map_hkey_add(&winfripp_confstore_hkey_bitmap,
					    &winfripp_confstore_hkey_bitmap_bits,
					    winfripp_confstore_hkey_map,
					    winfripp_confstore_hkey_map_len,
					    hKey, lpSubKey, phkResult)) {
	    return ERROR_SUCCESS;
	} else {
	    return ERROR_NOT_ENOUGH_MEMORY;
	}
    }
}

/*
 * Key {closing,deletion,enumeration} methods
 */

LONG winfrip_confstore_RegCloseKey(HKEY hKey)
{
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);

    /*
     * XXX document
     */
    switch (winfrip_confstore_backend_get()) {
    default:
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return ERROR_INVALID_FUNCTION;

    case WINFRIP_GENERAL_STORE_BACKEND_REGISTRY:
	return RegCloseKey(hKey);

    case WINFRIP_GENERAL_STORE_BACKEND_EPHEMERAL:
    case WINFRIP_GENERAL_STORE_BACKEND_FILE:
	if (winfripp_confstore_map_hkey_del(&winfripp_confstore_hkey_bitmap,
					    &winfripp_confstore_hkey_bitmap_bits,
					    winfripp_confstore_hkey_map,
					    winfripp_confstore_hkey_map_len,
					    hKey)) {
	    return ERROR_SUCCESS;
	} else {
	    return ERROR_NOT_ENOUGH_MEMORY;
	}
    }
}

LONG winfrip_confstore_RegDeleteKey(HKEY hKey, LPCSTR lpSubKey)
{
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpSubKey);

    /*
     * XXX document
     */
    switch (winfrip_confstore_backend_get()) {
    default:
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return ERROR_INVALID_FUNCTION;

    case WINFRIP_GENERAL_STORE_BACKEND_REGISTRY:
	return RegDeleteKeyA(hKey, lpSubKey);

    case WINFRIP_GENERAL_STORE_BACKEND_EPHEMERAL:
    case WINFRIP_GENERAL_STORE_BACKEND_FILE:
	/* TODO implement */
	return ERROR_INVALID_FUNCTION;
    }
}

LONG winfrip_confstore_RegEnumKey(HKEY hKey, DWORD dwIndex, LPSTR lpName, DWORD cchName)
{
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpName);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(cchName > 0);

    /*
     * XXX document
     */
    switch (winfrip_confstore_backend_get()) {
    default:
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return ERROR_INVALID_FUNCTION;

    case WINFRIP_GENERAL_STORE_BACKEND_REGISTRY:
	return RegEnumKeyA(hKey, dwIndex, lpName, cchName);

    case WINFRIP_GENERAL_STORE_BACKEND_EPHEMERAL:
    case WINFRIP_GENERAL_STORE_BACKEND_FILE:
	/* TODO implement */
	return ERROR_INVALID_FUNCTION;
    }
}

/*
 * Value {deletion,querying,setting} methods
 */

LONG winfrip_confstore_RegDeleteValue(HKEY hKey, LPCSTR lpValueName)
{
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpValueName);

    /*
     * XXX document
     */
    switch (winfrip_confstore_backend_get()) {
    default:
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return ERROR_INVALID_FUNCTION;

    case WINFRIP_GENERAL_STORE_BACKEND_REGISTRY:
	return RegDeleteValueA(hKey, lpValueName);

    case WINFRIP_GENERAL_STORE_BACKEND_EPHEMERAL:
    case WINFRIP_GENERAL_STORE_BACKEND_FILE:
	/* TODO implement */
	return ERROR_INVALID_FUNCTION;
    }
}

LONG winfrip_confstore_RegQueryValueEx(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpValueName);

    /*
     * XXX document
     */
    switch (winfrip_confstore_backend_get()) {
    default:
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return ERROR_INVALID_FUNCTION;

    case WINFRIP_GENERAL_STORE_BACKEND_REGISTRY:
	return RegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);

    case WINFRIP_GENERAL_STORE_BACKEND_EPHEMERAL:
    case WINFRIP_GENERAL_STORE_BACKEND_FILE:
	/* TODO implement */
	return ERROR_INVALID_FUNCTION;
    }
}

LONG winfrip_confstore_RegSetValueEx(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE *lpData, DWORD cbData)
{
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(hKey);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpValueName);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(lpData);
    WINFRIPP_CONFSTORE_DEBUG_ASSERT(cbData > 0);

    /*
     * XXX document
     */
    switch (winfrip_confstore_backend_get()) {
    default:
	WINFRIPP_CONFSTORE_DEBUG_FAIL();
	return ERROR_INVALID_FUNCTION;

    case WINFRIP_GENERAL_STORE_BACKEND_REGISTRY:
	return RegSetValueExA(hKey, lpValueName, Reserved, dwType, lpData, cbData);

    case WINFRIP_GENERAL_STORE_BACKEND_EPHEMERAL:
    case WINFRIP_GENERAL_STORE_BACKEND_FILE:
	/* TODO implement */
	return ERROR_INVALID_FUNCTION;
    }
}
