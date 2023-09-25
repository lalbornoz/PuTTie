/*
 * Implementation of Filename for Unix, including f_open().
 */

#include <fcntl.h>
#include <unistd.h>

#include "putty.h"

Filename *filename_from_str(const char *str)
{
    Filename *fn = snew(Filename);
    fn->path = dupstr(str);
    return fn;
}

Filename *filename_copy(const Filename *fn)
{
    return filename_from_str(fn->path);
}

const char *filename_to_str(const Filename *fn)
{
    return fn->path;
}

bool filename_equal(const Filename *f1, const Filename *f2)
{
    return !strcmp(f1->path, f2->path);
}

bool filename_is_null(const Filename *fn)
{
    return !fn->path[0];
}

void filename_free(Filename *fn)
{
    sfree(fn->path);
    sfree(fn);
}

void filename_serialise(BinarySink *bs, const Filename *f)
{
    put_asciz(bs, f->path);
}
Filename *filename_deserialise(BinarySource *src)
{
    return filename_from_str(get_asciz(src));
}

char filename_char_sanitise(char c)
{
    if (c == '/')
        return '.';
    return c;
}

FILE *f_open(const Filename *filename, char const *mode, bool is_private)
{
    /* {{{ winfrip */
#if 1
    int         fd;
    wchar_t *   filenameW = NULL;
    FILE *      fp = NULL;
    wchar_t *   modeW = NULL;
    WfrStatus   status;


    if (WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(
            filename->cpath, strlen(filename->cpath), &filenameW))
    &&  WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(
            mode, strlen(mode), &modeW)))
    {
        if (!is_private) {
            fp = _wfopen(filenameW, modeW);
        } else {
            assert(modeW[0] == L'w');        /* is_private is meaningless for read,
                                                and tricky for append */
            fd = _wopen(filenameW, O_WRONLY | O_CREAT | O_TRUNC, 0600);
            if (fd >= 0) {
                fp = _wfdopen(fd, modeW);
            }
        }
    }

    WFR_FREE_IF_NOTNULL(filenameW);
    WFR_FREE_IF_NOTNULL(modeW);

    return fp;
#else
    /* winfrip }}} */
    if (!is_private) {
        return fopen(filename->path, mode);
    } else {
        int fd;
        assert(mode[0] == 'w');        /* is_private is meaningless for read,
                                          and tricky for append */
        fd = open(filename->path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd < 0)
            return NULL;
        return fdopen(fd, mode);
    }
    /* {{{ winfrip */
#endif
    /* winfrip }}} */
}

/* {{{ winfrip */
/*
 * vim:expandtab sw=4 ts=4
 */
/* winfrip }}} */
