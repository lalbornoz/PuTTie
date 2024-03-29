/*
 * PuTTY version numbering
 */

/*
 * The difficult part of deciding what goes in these version strings
 * is done in Buildscr, and then written into version.h. All we have
 * to do here is to drop it into variables of the right names.
 */

#include "putty.h"
#include "ssh.h"

/* {{{ winfrip */
#if 0
/* winfrip }}} */
#include "version.h"
/* {{{ winfrip */
#endif
/* winfrip }}} */

const char ver[] = TEXTVER;
const char sshver[] = SSHVER;

/*
 * SSH local version string MUST be under 40 characters. Here's a
 * compile time assertion to verify this.
 */
enum { vorpal_sword = 1 / (sizeof(sshver) <= 40) };

/* {{{ winfrip */
/*
 * vim:expandtab sw=4 ts=4
 */
/* winfrip }}} */
