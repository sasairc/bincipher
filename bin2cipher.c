/*
 * bincipher
 *
 * bin2cipher.c
 * 
 * Copyright (c) 2017 sasairc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <n_cipher.h>

#define PROGNAME    "bin2cipher"
#define AUTHOR      "sasairc"
#define MAIL_TO     "sasairc@ssiserver.moe.hm"

typedef struct _LIST_T {
    int     number;
    char*   character;
    struct  _LIST_T*    next;
    struct  _LIST_T*    prev;
} list_t;

typedef struct {
    char*   seed;
    char*   delimiter;
    char*   iarg;
    char*   oarg;
    size_t  warg;
} b2c_t;

void print_usage(N_CIPHER* n_cipher)
{
    init_n_cipher(&n_cipher);
    n_cipher->config(&n_cipher, NULL, NULL);
    fprintf(stdout, "The %s, with %s\n\
Usage: bin2cipher --input=FILE [OPTION]...\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
\n\
  -i,  --input=FILE          input file (required)\n\
  -o,  --output=FILE         output file (default stdout)\n\
  -s,  --seed=STR            specify seed string (default = %s)\n\
  -m,  --delimiter=STR       specify delimiter string (default = %s)\n\
  -w,  --wrap=SIZE           wrap encoded lines after SIZE byte (default = 0)\n\
\n\
       --help                display this help and exit\n\
       --version             output version infomation and exit\n\
\n\
Report %s bugs to %s <%s>\n", 
        PROGNAME, n_cipher->version(), n_cipher->seed, n_cipher->delimiter,
        PROGNAME, AUTHOR, MAIL_TO);
    n_cipher->release(n_cipher);

    exit(0);
}

void print_version(N_CIPHER* n_cipher)
{
    init_n_cipher(&n_cipher);
    fprintf(stdout, "%s with %s\n",
            PROGNAME, n_cipher->version());
    n_cipher->release(n_cipher);

    exit(0);
}

list_t* seek_table_end(list_t* start)
{
    list_t* t1  = start;

    while (t1->next != NULL)
        t1 = t1->next;

    return t1;
}

int bin_to_cipher(FILE* fp1, FILE* fp2, N_CIPHER* nc, int wrap)
{
    int             y       = 0,
                    fragmnt = 0;

    size_t          byte    = 0;

    unsigned char   b       = '\0';

    char*           p       = NULL,
        *           buf[8]  = {NULL};

    list_t*         end     = NULL,
          *         t1      = NULL,
          *         t2      = NULL;

    end = seek_table_end(nc->table->start);
    while (fread(&b, sizeof(unsigned char), 1, fp1) == 1) {
        y = 0;
        while (b > 0) {
            fragmnt = b % nc->table->decimal;
            t1 = nc->table->start;
            t2 = end;
            while (t1 != NULL && t2 != NULL) {
                if (fragmnt == t1->number) {
                    p = t1->character;
                    break;
                }
                if (fragmnt == t2->number) {
                    p = t2->character;
                    break;
                }
                t1 = t1->next;
                t2 = t2->prev;
            }
            *(buf + y) = p;
            b /= nc->table->decimal;
            y++;
        }
        y--;
        while (y >= 0) {
            fputs(*(buf + y), fp2);
            y--;
        }
        fputs(nc->delimiter, fp2);
        if (wrap > 0 && wrap == byte) {
            fputs("\n", fp2);
            byte = 0;
        }
        byte++;
    }

    return 0;
}

int strisdigit(char* str)
{
    int i   = 0;

    while (i < strlen(str)) {
        if (!isdigit(*(str + i))) {
            fprintf(stderr, "%s: %s: invalid wrap size\n",
                    PROGNAME, str);

            return -1;
        }
        i++;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    int         res         = 0,
                index       = 0,
                status      = 0;

    FILE*       fp1         = NULL,
        *       fp2         = NULL;

    N_CIPHER*   nc          = NULL;

    b2c_t       b2c         = {
        NULL, NULL, NULL, NULL, 0,
    };

    struct  option opts[] = {
        {"seed",        required_argument,  NULL, 's'},
        {"delimiter",   required_argument,  NULL, 'm'},
        {"input",       required_argument,  NULL, 'i'},
        {"output",      required_argument,  NULL, 'o'},
        {"wrap",        required_argument,  NULL, 'w'},
        {"help",        no_argument,        NULL,  0 },
        {"version",     no_argument,        NULL,  1 },
        {0, 0, 0, 0},
    };

    while ((res = getopt_long(argc, argv, "s:m:i:o:w:", opts, &index)) != -1) {
        switch (res) {
            case    's':
                b2c.seed = optarg;
                break;
            case    'm':
                b2c.delimiter = optarg;
                break;
            case    'i':
                b2c.iarg = optarg;
                break;
            case    'o':
                b2c.oarg = optarg;
                break;
            case    'w':
                if (strisdigit(optarg) < 0)
                    return -1;
                b2c.warg = atoi(optarg);
                break;
            case    0:
                print_usage(nc);
            case    1:
                print_version(nc);
            case    '?':
                return -1;
        }
    }
    if (b2c.iarg == NULL)
        print_usage(nc);

    if ((fp1 = fopen(b2c.iarg, "rb")) == NULL) {
        fprintf(stderr, "%s: %s: %s\n",
                PROGNAME, b2c.iarg, strerror(errno));
        status = 3; goto RELEASE;
    }
    if (b2c.oarg != NULL) {
        if ((fp2 = fopen(b2c.oarg, "w")) == NULL) {
            fprintf(stderr, "%s: %s: %s\n",
                    PROGNAME, b2c.oarg, strerror(errno));
            status = 3; goto RELEASE;
        }
    } else {
        fp2 = stdout;
    }

    init_n_cipher(&nc);
    if (b2c.seed != NULL || b2c.delimiter != NULL) {
        switch (nc->check_argument(b2c.seed, b2c.delimiter)) {
            case    0:
                break;
            case    S_TOO_SHORT:
                fprintf(stderr, "%s: seed too short: %s\n",
                        PROGNAME, b2c.seed);
                status = 2; goto RELEASE;
            case    D_TOO_SHORT:
                fprintf(stderr, "%s: delimiter too short: %s\n",
                        PROGNAME, b2c.delimiter);
                status = 2; goto RELEASE;
            case    S_TOO_LONG:
                fprintf(stderr, "%s: seed too long: %s\n",
                        PROGNAME, b2c.seed);
                status = 2; goto RELEASE;
            default:
                fprintf(stderr, "%s: invalid seed or delimiter\n",
                        PROGNAME);
                status = 2; goto RELEASE;
        }
    }
    nc->config(&nc, b2c.seed, b2c.delimiter);
    status = bin_to_cipher(fp1, fp2, nc, b2c.warg);

RELEASE:
    if (fp1 != NULL)
        fclose(fp1);
    if (fp2 != stdout && fp2 != NULL)
        fclose(fp2);
    if (nc != NULL)
        nc->release(nc);

    return status;
}
