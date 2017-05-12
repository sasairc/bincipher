/*
 * bincipher
 *
 * cipher2bin.c
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

#include "./misc.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <n_cipher.h>

#define PROGNAME    "cipher2bin"
#define AUTHOR      "sasairc"
#define MAIL_TO     "sasairc@ssiserver.moe.hm"

typedef struct _LIST_T {
    int     number;
    char*   character;
    struct  _LIST_T*    next;
} list_t;

typedef struct {
    char*   seed;
    char*   delimiter;
    char*   iarg;
    char*   oarg;
} c2b_t;

void print_usage(N_CIPHER* n_cipher)
{
    init_n_cipher(&n_cipher);
    n_cipher->config(&n_cipher, NULL, NULL);
    fprintf(stdout, "The %s, with %s\n\
Usage: bin2cipher --output=FILE [OPTION]...\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
\n\
  -i,  --input=FILE          input file (default stdin)\n\
  -o,  --output=FILE         output file (require)\n\
  -s,  --seed=STR            specify seed string (default = %s)\n\
  -m,  --delimiter=STR       specify delimiter string (default = %s)\n\
\n\
       --help                display this help and exit\n\
\n\
Report %s bugs to %s <%s>\n", 
        PROGNAME, n_cipher->version(), n_cipher->seed, n_cipher->delimiter,
        PROGNAME, AUTHOR, MAIL_TO);
    n_cipher->release(n_cipher);

    exit(0);
}

unsigned char char_to_bin(N_CIPHER* nc, char* str)
{
    int             i       = 0,
                    digit   = 0;

    unsigned char   bin     = '\0',
                    code    = '\0';

    size_t          byte    = 0;

    list_t*         table   = NULL;

    digit = mbstrlen(str) - 1;
    while (*str != '\0') {
        code = (unsigned char)*str;

        /*
         * get character size
         */
        if ((code & 0x80) == 0x00)
            byte = 1;
        else if ((code & 0xE0) == 0xC0)
            byte = 2;
        else if ((code & 0xF0) == 0xE0)
            byte = 3;
        else if ((code & 0xF8) == 0xF0)
            byte = 4;
        else if ((code & 0xFC) == 0xF8)
            byte = 5;
        else if ((code & 0xFE) == 0xFC)
            byte = 6;
        else
            return 0;

        /*
         * search character
         */
        table = nc->table->start;
        while (table != NULL) {
            i = 0;
            while (*(str + i) == *(table->character + i))
                i++;
            if (i >= byte)
                break;
            else
                table = table->next;
        }
        if (table == 0 && i < byte)
            return 0;

        bin += table->number * pow(nc->table->decimal, digit);
        str += byte;
        digit--;
    }

    return bin;
}

int cipher_to_bin(N_CIPHER* nc, FILE* fp1, FILE* fp2)
{
    int             c       = 0;

    size_t          done    = 0,
                    size    = 512;

    unsigned char   bin     = '\0';

    char*           token   = NULL,
        *           buf     = NULL;

    if ((buf = (char*)
                malloc(sizeof(char) * (size + 1))) == NULL) {
        fprintf(stderr, "%s: %s\n",
                PROGNAME, strerror(errno));
        goto ERR;
    }
    while ((c = fgetc(fp1)) != EOF) {
        if (done == (size - 1)) {
            size += size;
            if ((buf = (char*)
                        realloc(buf, sizeof(char) * (size + 1))) == NULL) {
                fprintf(stderr, "%s: %s\n",
                        PROGNAME, strerror(errno));
                goto ERR;
            }
        }
        *(buf + done) = c;
        done++;
    }
    token = mbstrtok(buf, nc->delimiter);
    while (token != NULL) {
        bin = char_to_bin(nc, token);
        token = mbstrtok(NULL, nc->delimiter);
        if (token != NULL)
            fwrite(&bin, sizeof(unsigned char), 1, fp2);
    }
    free(buf);

    return 0;

ERR:
    if (buf != NULL)
        free(buf);

    return -1;
}

int main(int argc, char* argv[])
{
    int         res         = 0,
                index       = 0,
                status      = 0;

    FILE*       fp1         = NULL,
        *       fp2         = NULL;

    N_CIPHER*   nc          = NULL;

    c2b_t       c2b         = {
        NULL, NULL, NULL, NULL,
    };

    struct  option opts[] = {
        {"seed",        required_argument,  NULL, 's'},
        {"delimiter",   required_argument,  NULL, 'm'},
        {"input",       required_argument,  NULL, 'i'},
        {"output",      required_argument,  NULL, 'o'},
        {"help",        no_argument,        NULL,  0 },
        {0, 0, 0, 0},
    };

    while ((res = getopt_long(argc, argv, "s:m:i:o:", opts, &index)) != -1) {
        switch (res) {
            case    's':
                c2b.seed = optarg;
                break;
            case    'm':
                c2b.delimiter = optarg;
                break;
            case    'i':
                c2b.iarg = optarg;
                break;
            case    'o':
                c2b.oarg = optarg;
                break;
            case    0:
                print_usage(nc);
            case    '?':
                return -1;
        }
    }
    if (c2b.oarg == NULL)
        print_usage(nc);

    if (c2b.iarg != NULL) {
        if ((fp1 = fopen(c2b.iarg, "r")) == NULL) {
            fprintf(stderr, "%s: %s: %s\n",
                    PROGNAME, c2b.iarg, strerror(errno));
            status = 3; goto RELEASE;
        }
    } else {
        fp1 = stdin;
    }
    if ((fp2 = fopen(c2b.oarg, "wb")) == NULL) {
        fprintf(stderr, "%s: %s: %s\n",
                PROGNAME, c2b.oarg, strerror(errno));
        status = 3; goto RELEASE;
    }

    init_n_cipher(&nc);
    if (c2b.seed != NULL || c2b.delimiter != NULL) {
        switch (nc->check_argument(c2b.seed, c2b.delimiter)) {
            case    0:
                break;
            case    S_TOO_SHORT:
                fprintf(stderr, "%s: seed too short: %s\n",
                        PROGNAME, c2b.seed);
                status = 2; goto RELEASE;
            case    D_TOO_SHORT:
                fprintf(stderr, "%s: delimiter too short: %s\n",
                        PROGNAME, c2b.delimiter);
                status = 2; goto RELEASE;
            case    S_TOO_LONG:
                fprintf(stderr, "%s: seed too long: %s\n",
                        PROGNAME, c2b.seed);
                status = 2; goto RELEASE;
            default:
                fprintf(stderr, "%s: invalid seed or delimiter\n",
                        PROGNAME);
                status = 2; goto RELEASE;
        }
    }
    nc->config(&nc, c2b.seed, c2b.delimiter);
    status = cipher_to_bin(nc, fp1, fp2);

RELEASE:
    if (fp1 != stdin && fp1 != NULL)
        fclose(fp1);
    if (fp2 != NULL)
        fclose(fp2);
    if (nc != NULL)
        nc->release(nc);

    return status;
}
