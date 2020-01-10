/*
 * wincapi.c: implementation of wincapi.h.
 */

#include "putty.h"

#if !defined NO_SECURITY

#include "putty.h"
#include "ssh.h"

#define WINCAPI_GLOBAL
#include "wincapi.h"

bool got_crypt(void)
{
    static bool attempted = false;
    static bool successful;
    static HMODULE crypt;

    if (!attempted) {
        attempted = true;
        crypt = load_system32_dll("crypt32.dll");
        successful = crypt &&
#ifdef COVERITY
            /* The build toolchain I use with Coverity doesn't know
             * about this function, so can't type-check it */
            GET_WINDOWS_FUNCTION_NO_TYPECHECK(crypt, CryptProtectMemory)
#else
            GET_WINDOWS_FUNCTION(crypt, CryptProtectMemory)
#endif
            ;
    }
    return successful;
}

#ifdef COVERITY
/*
 * The hack I use to build for Coverity scanning, using winegcc and
 * Makefile.mgw, didn't provide some defines in wincrypt.h last time I
 * looked. Therefore, define them myself here, but enclosed in #ifdef
 * COVERITY to ensure I don't make up random nonsense values for any
 * real build.
 */
#ifndef CRYPTPROTECTMEMORY_BLOCK_SIZE
#define CRYPTPROTECTMEMORY_BLOCK_SIZE 16
#endif
#ifndef CRYPTPROTECTMEMORY_CROSS_PROCESS
#define CRYPTPROTECTMEMORY_CROSS_PROCESS 1
#endif
#endif

char *capi_obfuscate_string(const char *realname)
{
    char *cryptdata;
    int cryptlen;
    unsigned char digest[32];
    char retbuf[65];
    int i;

    cryptlen = strlen(realname) + 1;
    cryptlen += CRYPTPROTECTMEMORY_BLOCK_SIZE - 1;
    cryptlen /= CRYPTPROTECTMEMORY_BLOCK_SIZE;
    cryptlen *= CRYPTPROTECTMEMORY_BLOCK_SIZE;

    cryptdata = snewn(cryptlen, char);
    memset(cryptdata, 0, cryptlen);
    strcpy(cryptdata, realname);

    /*
     * CRYPTPROTECTMEMORY_CROSS_PROCESS causes CryptProtectMemory to
     * use the same key in all processes with this user id, meaning
     * that the next PuTTY process calling this function with the same
     * input will get the same data.
     *
     * (Contrast with CryptProtectData, which invents a new session
     * key every time since its API permits returning more data than
     * was input, so calling _that_ and hashing the output would not
     * be stable.)
     *
     * We don't worry too much if this doesn't work for some reason.
     * Omitting this step still has _some_ privacy value (in that
     * another user can test-hash things to confirm guesses as to
     * where you might be connecting to, but cannot invert SHA-256 in
     * the absence of any plausible guess). So we don't abort if we
     * can't call CryptProtectMemory at all, or if it fails.
     */
    if (got_crypt())
        p_CryptProtectMemory(cryptdata, cryptlen,
                             CRYPTPROTECTMEMORY_CROSS_PROCESS);

    /*
     * We don't want to give away the length of the hostname either,
     * so having got it back out of CryptProtectMemory we now hash it.
     */
    hash_simple(&ssh_sha256, make_ptrlen(cryptdata, cryptlen), digest);

    sfree(cryptdata);

    /*
     * Finally, make printable.
     */
    for (i = 0; i < 32; i++) {
        sprintf(retbuf + 2*i, "%02x", digest[i]);
        /* the last of those will also write the trailing NUL */
    }

    return dupstr(retbuf);
}

#endif /* !defined NO_SECURITY */
