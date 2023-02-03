/*
 * winfrip_rtl_registry.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_REGISTRY_H
#define PUTTY_WINFRIP_REGISTRY_H

#include <stdarg.h>

/*
 * Public type definitions private to PuTTie/winfrip_storage*.c
 */

typedef struct WfrEnumerateRegState {
	bool	donefl;
	DWORD	dwIndex;
	HKEY	hKey;
} WfrEnumerateRegState;
#define WFR_ENUMERATE_REG_STATE_EMPTY {					\
	.donefl = false,						\
	.dwIndex = 0,							\
	.hKey = 0,							\
}
#define WFR_ENUMERATE_REG_STATE_INIT(state) ({				\
	(state) = (WfrEnumerateRegState)WFR_ENUMERATE_REG_STATE_EMPTY;	\
	WFR_STATUS_CONDITION_SUCCESS;					\
})

/*
 * Public subroutine prototypes private to PuTTie/winfrip*.c
 */

#define		WfrCreateRegKey(hKey, phKey, ...)	WfrOpenRegKey((hKey), true, true, (phKey), ## __VA_ARGS__, (const char *)NULL)
#define		WfrCreateRegKeyV(hKey, phKey, ap)	WfrOpenRegKey((hKey), true, true, (phKey), ap)
void		WfrEnumerateRegCancel(WfrEnumerateRegState **pstate);
WfrStatus	WfrEnumerateRegInit(WfrEnumerateRegState **pstate, ...);
WfrStatus	WfrEnumerateRegKeys(bool *pdonefl, char **pitem_name, WfrEnumerateRegState **pstate);
WfrStatus	WfrEnumerateRegValues(bool *pdonefl, void **pitem_data, size_t *pitem_data_len, char **pitem_name, DWORD *pitem_type, WfrEnumerateRegState **pstate);
WfrStatus	WfrEscapeRegKey(const char *key, char **pkey_escaped);
WfrStatus	WfrOpenRegKey(HKEY hKey, bool createfl, bool writefl, HKEY *phKey, ...);
WfrStatus	WfrOpenRegKeyV(HKEY hKey, bool createfl, bool writefl, HKEY *phKey, va_list ap);
#define		WfrOpenRegKeyRo(hKey, phKey, ...)	WfrOpenRegKey((hKey), false, false, (phKey), ## __VA_ARGS__, (const char *)NULL)
#define		WfrOpenRegKeyRoV(hKey, phKey, ap)	WfrOpenRegKeyV((hKey), false, false, (phKey), ap)
#define		WfrOpenRegKeyRw(hKey, phKey, ...)	WfrOpenRegKey((hKey), false, true, (phKey), ## __VA_ARGS__, (const char *)NULL)
#define		WfrOpenRegKeyRwV(hKey, phKey, ap)	WfrOpenRegKeyV((hKey), false, true, (phKey), ap)
WfrStatus	WfrUnescapeRegKey(const char *key_escaped, char **pkey);

#endif // !PUTTY_WINFRIP_REGISTRY_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
