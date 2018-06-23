/*
 * winfrip_confstore_post.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#ifndef PUTTY_WINFRIP_CONFSTORE_POST_H
#define PUTTY_WINFRIP_CONFSTORE_POST_H

#undef RegCloseKey
#undef RegCreateKey
#undef RegCreateKeyEx
#undef RegDeleteKey
#undef RegDeleteValue
#undef RegEnumKey
#undef RegOpenKey
#undef RegQueryValueEx
#undef RegSetValueEx
#define RegCloseKey winfrip_confstore_RegCloseKey
#define RegCreateKey winfrip_confstore_RegCreateKey
#define RegCreateKeyEx winfrip_confstore_RegCreateKeyEx
#define RegDeleteKey winfrip_confstore_RegDeleteKey
#define RegDeleteValue winfrip_confstore_RegDeleteValue
#define RegEnumKey winfrip_confstore_RegEnumKey
#define RegOpenKey winfrip_confstore_RegOpenKey
#define RegQueryValueEx winfrip_confstore_RegQueryValueEx
#define RegSetValueEx winfrip_confstore_RegSetValueEx

#endif
