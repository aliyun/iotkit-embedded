
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "aliot_platform.h"

#include "utils_hexdump.h"

#define DUMP_PREFIX_ADDRESS ALIOT_HEXDUMP_PREFIX_ADDRESS
#define DUMP_PREFIX_OFFSET  ALIOT_HEXDUMP_PREFIX_OFFSET

#define DUMP_PRINT aliot_platform_printf


const char hex_asc[] = "0123456789abcdef";

#define hex_asc_lo(x)   hex_asc[((x) & 0x0f)]
#define hex_asc_hi(x)   hex_asc[((x) & 0xf0) >> 4]


static inline int hexdump_min(int a, int b)
{
    return (a < b) ? a : b;
}

/**
 * hex_to_bin - convert a hex digit to its real value
 * @ch: ascii character represents hex digit
 *
 * hex_to_bin() converts one hex digit to its actual value or -1 in case of bad
 * input.
 */
int hex_to_bin(char ch)
{
    if ((ch >= '0') && (ch <= '9')) {
        return ch - '0';
    }
    ch = tolower(ch);
    if ((ch >= 'a') && (ch <= 'f')) {
        return ch - 'a' + 10;
    }
    return -1;
}


/**
 * hex2bin - convert an ascii hexadecimal string to its binary representation
 * @dst: binary result
 * @src: ascii hexadecimal string
 * @count: result length
 */
void hex2bin(uint8_t *dst, const char *src, size_t count)
{
    while (count--) {
        *dst = hex_to_bin(*src++) << 4;
        *dst += hex_to_bin(*src++);
        dst++;
    }
}


/**
 * hex_dump_to_buffer - convert a blob of data to "hex ASCII" in memory
 * @buf: data blob to dump
 * @len: number of bytes in the @buf
 * @rowsize: number of bytes to print per line; must be 16 or 32
 * @groupsize: number of bytes to print at a time (1, 2, 4, 8; default = 1)
 * @linebuf: where to put the converted data
 * @linebuflen: total size of @linebuf, including space for terminating NUL
 * @ascii: include ASCII after the hex output
 *
 * hex_dump_to_buffer() works on one "line" of output at a time, i.e.,
 * 16 or 32 bytes of input data converted to hex + ASCII output.
 *
 * Given a buffer of u8 data, hex_dump_to_buffer() converts the input data
 * to a hex + ASCII dump at the supplied memory location.
 * The converted output is always NUL-terminated.
 *
 * E.g.:
 *   hex_dump_to_buffer(frame->data, frame->len, 16, 1,
 *          linebuf, sizeof(linebuf), true);
 *
 * example output buffer:
 * 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f  @ABCDEFGHIJKLMNO
 */
void hex_dump_to_buffer(const void *buf, size_t len, int rowsize,
                        int groupsize, char *linebuf, size_t linebuflen,
                        int ascii)
{
    const uint8_t *ptr = buf;
    uint8_t ch;
    int j, lx = 0;
    int ascii_column;

    if (rowsize != 16 && rowsize != 32) {
        rowsize = 16;
    }

    if (!len) {
        goto nil;
    }
    if (len > rowsize) {    /* limit to one line at a time */
        len = rowsize;
    }
    if ((len % groupsize) != 0) { /* no mixed size output */
        groupsize = 1;
    }

    switch (groupsize) {
        case 8: {
            const uint64_t *ptr8 = buf;
            int ngroups = len / groupsize;

            for (j = 0; j < ngroups; j++)
                lx += snprintf(linebuf + lx, linebuflen - lx,
                               "%s%16.16llx", j ? " " : "",
                               (unsigned long long) * (ptr8 + j));
            ascii_column = 17 * ngroups + 2;
            break;
        }

        case 4: {
            const uint32_t *ptr4 = buf;
            int ngroups = len / groupsize;

            for (j = 0; j < ngroups; j++)
                lx += snprintf(linebuf + lx, linebuflen - lx,
                               "%s%8.8x", j ? " " : "", *(ptr4 + j));
            ascii_column = 9 * ngroups + 2;
            break;
        }

        case 2: {
            const uint16_t *ptr2 = buf;
            int ngroups = len / groupsize;

            for (j = 0; j < ngroups; j++)
                lx += snprintf(linebuf + lx, linebuflen - lx,
                               "%s%4.4x", j ? " " : "", *(ptr2 + j));
            ascii_column = 5 * ngroups + 2;
            break;
        }

        default:
            for (j = 0; (j < len) && (lx + 3) <= linebuflen; j++) {
                ch = ptr[j];
                linebuf[lx++] = hex_asc_hi(ch);
                linebuf[lx++] = hex_asc_lo(ch);
                linebuf[lx++] = ' ';
            }
            if (j) {
                lx--;
            }

            ascii_column = 3 * rowsize + 2;
            break;
    }
    if (!ascii) {
        goto nil;
    }

    while (lx < (linebuflen - 1) && lx < (ascii_column - 1)) {
        linebuf[lx++] = ' ';
    }
    for (j = 0; (j < len) && (lx + 2) < linebuflen; j++) {
        ch = ptr[j];
        linebuf[lx++] = (isascii(ch) && isprint(ch)) ? ch : '.';
    }
nil:
    linebuf[lx++] = '\0';
}


/**
 * print_hex_dump - print a text hex dump to syslog for a binary blob of data
 * @level: kernel log level (e.g. KERN_DEBUG)
 * @prefix_str: string to prefix each line with;
 *  caller supplies trailing spaces for alignment if desired
 * @prefix_type: controls whether prefix of an offset, address, or none
 *  is printed (%DUMP_PREFIX_OFFSET, %DUMP_PREFIX_ADDRESS, %DUMP_PREFIX_NONE)
 * @rowsize: number of bytes to print per line; must be 16 or 32
 * @groupsize: number of bytes to print at a time (1, 2, 4, 8; default = 1)
 * @buf: data blob to dump
 * @len: number of bytes in the @buf
 * @ascii: include ASCII after the hex output
 *
 * Given a buffer of u8 data, print_hex_dump() prints a hex + ASCII dump
 * to the kernel log at the specified kernel log level, with an optional
 * leading prefix.
 *
 * print_hex_dump() works on one "line" of output at a time, i.e.,
 * 16 or 32 bytes of input data converted to hex + ASCII output.
 * print_hex_dump() iterates over the entire input @buf, breaking it into
 * "line size" chunks to format and print.
 *
 * E.g.:
 *   print_hex_dump(KERN_DEBUG, "raw data: ", DUMP_PREFIX_ADDRESS,
 *          16, 1, frame->data, frame->len, true);
 *
 * Example output using %DUMP_PREFIX_OFFSET and 1-byte mode:
 * 0009ab42: 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f  @ABCDEFGHIJKLMNO
 * Example output using %DUMP_PREFIX_ADDRESS and 4-byte mode:
 * ffffffff88089af0: 73727170 77767574 7b7a7978 7f7e7d7c  pqrstuvwxyz{|}~.
 */
void print_hex_dump(const char *level, const char *prefix_str, int prefix_type,
                    int rowsize, int groupsize,
                    const void *buf, size_t len, int ascii)
{
    const uint8_t *ptr = buf;
    int i, linelen, remaining = len;
    char linebuf[32 * 3 + 2 + 32 + 1];

    if (rowsize != 16 && rowsize != 32) {
        rowsize = 16;
    }

    for (i = 0; i < len; i += rowsize) {
        linelen = hexdump_min(remaining, rowsize);
        remaining -= rowsize;

        hex_dump_to_buffer(ptr + i, linelen, rowsize, groupsize,
                           linebuf, sizeof(linebuf), ascii);

        switch (prefix_type) {
            case DUMP_PREFIX_ADDRESS:
                DUMP_PRINT("%s%s%p: %s\n",
                           level, prefix_str, ptr + i, linebuf);
                break;
            case DUMP_PREFIX_OFFSET:
                DUMP_PRINT("%s%s%.8x: %s\n", level, prefix_str, i, linebuf);
                break;
            default:
                DUMP_PRINT("%s%s%s\n", level, prefix_str, linebuf);
                break;
        }
    }
}


void aliot_hexdump(aliot_hexdump_prefix_type_t prefix_type,
                               int rowsize,
                               int groupsize,
                               const void *buf,
                               size_t len,
                               int ascii)
{
    print_hex_dump("", "", prefix_type, rowsize, groupsize, buf, len, ascii);
}
