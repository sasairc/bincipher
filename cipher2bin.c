/*
 * cipher2bin.c
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

typedef struct _LIST_T {
    int     number;
    char*   character;
    struct  _LIST_T*    next;
} list_t;

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

int cipher_to_bin(N_CIPHER* nc, FILE* fp)
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
    while ((c = fgetc(stdin)) != EOF) {
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
        fwrite(&bin, sizeof(unsigned char), 1, fp);
        token = mbstrtok(NULL, nc->delimiter);
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

    char*       seed        = NULL,
        *       delimiter   = NULL;

    FILE*       fp          = NULL;

    N_CIPHER*   nc          = NULL;

    struct  option opts[] = {
        {"seed",        required_argument,  NULL, 's'},
        {"delimiter",   required_argument,  NULL, 'm'},
        {0, 0, 0, 0},
    };
    while ((res = getopt_long(argc, argv, "s:m:", opts, &index)) != -1) {
        switch (res) {
            case    's':
                seed = optarg;
                break;
            case    'm':
                delimiter = optarg;
                break;
            case    '?':
                return -1;
        }
    }
    init_n_cipher(&nc);
    if (seed != NULL || delimiter != NULL) {
        switch (nc->check_argument(seed, delimiter)) {
            case    0:
                break;
            case    S_TOO_SHORT:
                fprintf(stderr, "%s: seed too short: %s\n",
                        PROGNAME, seed);
                status = 2; goto RELEASE;
            case    D_TOO_SHORT:
                fprintf(stderr, "%s: delimiter too short: %s\n",
                        PROGNAME, delimiter);
                status = 2; goto RELEASE;
            case    S_TOO_LONG:
                fprintf(stderr, "%s: seed too long: %s\n",
                        PROGNAME, seed);
                status = 2; goto RELEASE;
            default:
                fprintf(stderr, "%s: invalid seed or delimiter\n",
                        PROGNAME);
                status = 2; goto RELEASE;
        }
    }
    nc->config(&nc, seed, delimiter);
    if (optind == argc) {
        fprintf(stdout, "Usage: cipher2bin OUTPUT\n");
        goto RELEASE;
    }
    if ((fp = fopen(argv[optind], "wb")) == NULL) {
        fprintf(stderr, "%s: %s\n",
                PROGNAME, strerror(errno));
        status = 3; goto RELEASE;
    }
    status = cipher_to_bin(nc, fp);

RELEASE:
    if (fp != NULL)
        fclose(fp);
    nc->release(nc);

    return status;
}
