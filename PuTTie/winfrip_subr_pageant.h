/*
 * winfrip_subr_pageant.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_SUBR_PAGEANT_H
#define PUTTY_WINFRIP_SUBR_PAGEANT_H


/*
 * Public subroutine prototypes private to windows/pageant.c
 */

typedef struct CommandLineKey {
	Filename *	fn;
	bool		add_encrypted;
} CommandLineKey;

void WfPageantAddKey(bool encrypted, Filename *fn, int (*win_add_keyfile)(Filename *, bool));
void WfPageantAddKeysFromCmdLine(bool add_keys_encrypted, CommandLineKey *clkeys, size_t nclkeys, int (*win_add_keyfile)(Filename *, bool));
void WfPageantAppendCmdLineBackendArgString(TCHAR *cmdline, size_t cmdline_size);
void WfPageantAppendParamBackendArgString(TCHAR *param, size_t param_size);
void WfPageantDeleteAllKeys(void (*pageant_delete_all)(void));
void WfPageantDeleteKey(int nkey, bool (*pageant_delete_nth_key)(int), const char *(*pageant_get_nth_key_path)(int));
void WfPageantInit(LPSTR *pcmdline);
void WfPageantUpdateSessions(UINT idm_sessions_base, int initial_menuitems_count, char *putty_default, char *putty_path, HMENU session_menu);

#endif // !PUTTY_WINFRIP_SUBR_PAGEANT_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */
