/*
 * no-jump-list.c: stub jump list functions for Windows executables
 * that don't update the jump list.
 */

/* {{{ winfrip */
#include "PuTTie/winfrip_storage_jumplist_wrap.h"
/* winfrip }}} */
#include "putty.h"

void add_session_to_jumplist(const char * const sessionname) {}
void remove_session_from_jumplist(const char * const sessionname) {}
void clear_jumplist(void) {}

/* {{{ winfrip */
/*
 * vim:expandtab sw=4 ts=4
 */
/* winfrip }}} */
