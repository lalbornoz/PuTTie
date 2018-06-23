/*
 * winfrip_confstore.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#include "putty.h"
#include "dialog.h"
#include "winfrip.h"
#include "winfrip_priv.h"

#include <assert.h>

/*
 * {External,Static} variables
 */

/*
 * XXX document
 */
extern Conf *conf;

/*
 * Public subroutines
 */

LONG winfrip_confstore_RegCloseKey(HKEY hKey)
{
    return RegCloseKey(hKey);
}

LONG winfrip_confstore_RegCreateKey(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult)
{
    return RegCreateKeyA(hKey, lpSubKey, phkResult);
}

LONG winfrip_confstore_RegCreateKeyEx(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition)
{
    return RegCreateKeyExA(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
}

LONG winfrip_confstore_RegDeleteKey(HKEY hKey, LPCSTR lpSubKey)
{
    return RegDeleteKeyA(hKey, lpSubKey);
}

LONG winfrip_confstore_RegDeleteValue(HKEY hKey, LPCSTR lpValueName)
{
    return RegDeleteValueA(hKey, lpValueName);
}

LONG winfrip_confstore_RegEnumKey(HKEY hKey, DWORD dwIndex, LPSTR lpName, DWORD cchName)
{
    return RegEnumKeyA(hKey, dwIndex, lpName, cchName);
}

LONG winfrip_confstore_RegOpenKey(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult)
{
    return RegOpenKeyA(hKey, lpSubKey, phkResult);
}

LONG winfrip_confstore_RegQueryValueEx(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
    return RegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

LONG winfrip_confstore_RegSetValueEx(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE *lpData, DWORD cbData)
{
    return RegSetValueExA(hKey, lpValueName, Reserved, dwType, lpData, cbData);
}
