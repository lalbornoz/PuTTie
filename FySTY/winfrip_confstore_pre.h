/*
 * winfrip_confstore_pre.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#ifndef PUTTY_WINFRIP_CONFSTORE_PRE_H
#define PUTTY_WINFRIP_CONFSTORE_PRE_H

#define RegCloseKey winfrip_confstore_XxxRegCloseKey
#define RegCreateKey winfrip_confstore_XxxRegCreateKey
#define RegCreateKeyEx winfrip_confstore_XxxRegCreateKeyEx
#define RegDeleteKey winfrip_confstore_XxxRegDeleteKey
#define RegDeleteValue winfrip_confstore_XxxRegDeleteValue
#define RegEnumKey winfrip_confstore_XxxRegEnumKey
#define RegOpenKey winfrip_confstore_XxxRegOpenKey
#define RegQueryValueEx winfrip_confstore_XxxRegQueryValueEx
#define RegSetValueEx winfrip_confstore_XxxRegSetValueEx

#endif
