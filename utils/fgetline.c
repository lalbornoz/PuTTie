/*
 * Read an entire line of text from a file. Return a buffer
 * malloced to be as big as necessary (caller must free).
 */

#include "defs.h"
#include "misc.h"

/* {{{ winfrip */
#include <fcntl.h>
#include <wchar.h>

#include "../PuTTie/winfrip_rtl.h"
/* winfrip }}} */

char *fgetline(FILE *fp)
{
    /* {{{ winfrip */
#if 1
    int         mode_old;
    char *      ret = NULL;
    wchar_t *   retW;
    size_t      retW_len;
    size_t      retW_size;


    retW_len = 0;
    retW_size = 512;
    retW = snewn(retW_size, wchar_t);
    if (!retW) {
        return NULL;
    }

    mode_old = _setmode(_fileno(stdin), _O_U16TEXT);

    while (fgetws(
            retW + retW_len,
            retW_size - retW_len, fp))
    {
        retW_len += wcslen(retW + retW_len);
        if ((retW_len >= 2)
        &&  (retW[retW_len - 2] == L'\n')
        &&  (retW[retW_len - 1] == L'\n'))
        {
            break;          /* got a pair of newlines (FUCK YOU WINDOWS,) we're done */
        } else if (!sgrowarrayn_nm(retW, retW_size, retW_len, 512)) {
            sfree(retW);
            (void)_setmode(_fileno(stdin), mode_old);
            return NULL;
        }
    }

    (void)_setmode(_fileno(stdin), mode_old);

    if (retW_len == 0) {    /* first fgetws returned NULL */
        sfree(retW);
        return NULL;
    } else {
        retW[retW_len] = L'\0';
    }

    (void)WfrConvertUtf16ToUtf8String(retW, retW_size, &ret);
    return ret;
#else
    /* winfrip }}} */
    char *ret = snewn(512, char);
    size_t size = 512, len = 0;
    while (fgets(ret + len, size - len, fp)) {
        len += strlen(ret + len);
        if (len > 0 && ret[len-1] == '\n')
            break;                     /* got a newline, we're done */
        sgrowarrayn_nm(ret, size, len, 512);
    }
    if (len == 0) {                    /* first fgets returned NULL */
        sfree(ret);
        return NULL;
    }
    ret[len] = '\0';
    return ret;
    /* {{{ winfrip */
#endif
    /* winfrip }}} */
}

/* {{{ winfrip */
/*
 * vim:expandtab sw=4 ts=4
 */
/* winfrip }}} */
