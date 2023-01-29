/*
 * no-jump-list-pageant.c: stub jump list functions for pageant
 * that don't update the jump list.
 */

#include <stddef.h>

void add_session_to_jumplist_PuTTY(const char *const sessionname) {}
void remove_session_from_jumplist_PuTTY(const char *const sessionname) {}
void clear_jumplist_PuTTY(void) {}

int add_to_jumplist_registry(const char *item) { return -1; }
char *get_jumplist_registry_entries_PuTTY(void) { return NULL; }
int remove_from_jumplist_registry(const char *item) { return -1; }
