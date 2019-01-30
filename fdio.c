/* vi: set sw=4 ts=4:
 *
 * fdio.c
 *
 * Functions for doing I/O on file descriptors.
 *
 * by Nick Patavalis (npat@efault.net)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

#include "logger.h"
#include <time.h>
#include <sys/time.h>

/**********************************************************************/

ssize_t
writen_ni(int fd, const void *buff, size_t n)
{
    int char_pos = 0, ret = 0;
    time_t raw;
    struct tm *t;
    struct timeval tv;
    time(&raw);
    t = localtime(&raw);
    gettimeofday(&tv, NULL);

    for (char_pos = 0; char_pos < n; char_pos++) {
        switch(((char *)buff)[char_pos]) {
        case '\n':
            if (logger_timestamp_e == LOGGER_TIMESTAMP_COMPLEX)
                ret += dprintf(fd, "\n[%04d-%02d-%02d %02d:%02d:%02d.%03ld] ", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, tv.tv_usec / 1000);
            else if (logger_timestamp_e == LOGGER_TIMESTAMP_SIMPLE)
                ret += dprintf(fd, "\n[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
            else if (logger_timestamp_e == LOGGER_TIMESTAMP_NONE)
                ret += dprintf(fd, "\n");
            break;
        default:
            ret += dprintf(fd, "%c", ((char *)buff)[char_pos]);
        }
    }

    return ret;
}

int
fd_vprintf (int fd, const char *format, va_list ap)
{
    char buf[256];
    int len;

    len = vsnprintf(buf, sizeof(buf), format, ap);
    buf[sizeof(buf) - 1] = '\0';

    return writen_ni(fd, buf, len);
}

int
fd_printf (int fd, const char *format, ...)
{
    va_list args;
    int len;

    va_start(args, format);
    len = fd_vprintf(fd, format, args);
    va_end(args);

    return len;
}

/**********************************************************************/

#ifndef LINENOISE

static int
cput(int fd, char c)
{
    return write(fd, &c, 1);
}

static int
cdel (int fd)
{
    const char del[] = "\b \b";
    return write(fd, del, sizeof(del) - 1);
}

static int
xput (int fd, unsigned char c)
{
    const char hex[] = "0123456789abcdef";
    char b[4];

    b[0] = '\\'; b[1] = 'x'; b[2] = hex[c >> 4]; b[3] = hex[c & 0x0f];
    return write(fd, b, sizeof(b));
}

static int
xdel (int fd)
{
    const char del[] = "\b\b\b\b    \b\b\b\b";
    return write(fd, del, sizeof(del) - 1);
}

int
fd_readline (int fdi, int fdo, char *b, int bsz)
{
    int r;
    unsigned char c;
    unsigned char *bp, *bpe;

    bp = (unsigned char *)b;
    bpe = (unsigned char *)b + bsz - 1;

    while (1) {
        r = read(fdi, &c, 1);
        if ( r <= 0 ) { r = -1; goto out; }

        switch (c) {
        case '\b':
        case '\x7f':
            if ( bp > (unsigned char *)b ) {
                bp--;
                if ( isprint(*bp) )
                    cdel(fdo);
                else
                    xdel(fdo);
            } else {
                cput(fdo, '\x07');
            }
            break;
        case '\x03': /* CTRL-c */
            r = -1;
            errno = EINTR;
            goto out;
        case '\r':
            *bp = '\0';
            r = bp - (unsigned char *)b;
            goto out;
        default:
            if ( bp < bpe ) {
                *bp++ = c;
                if ( isprint(c) )
                    cput(fdo, c);
                else
                    xput(fdo, c);
            } else {
                cput(fdo, '\x07');
            }
            break;
        }
    }

out:
    return r;
}

#endif /* of LINENOISE */

/**********************************************************************/

/*
 * Local Variables:
 * mode:c
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
