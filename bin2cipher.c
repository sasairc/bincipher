/*
 * bin2cipher.c
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <n_cipher.h>

#define PROGNAME    "bin2cipher"

typedef struct _LIST_T {
    int     number;
    char*   character;
    struct  _LIST_T*    next;
} list_t;

int bin_to_cipher(FILE* fp, N_CIPHER* nc)
{
    int             y       = 0,
                    fragmnt = 0;

    unsigned char   b       = '\0';

    char*           buf[8]  = {NULL};

    list_t*         table   = NULL;

    while (fread(&b, sizeof(unsigned char), 1, fp) == 1) {
        y = 0;
        while (b > 0) {
            fragmnt = b % nc->table->decimal;
            table = nc->table->start;
            while (fragmnt != table->number)
                table = table->next;
            *(buf + y) = table->character;
            b /= nc->table->decimal;
            y++;
        }
        y--;
        while (y >= 0) {
            fprintf(stdout, "%s",
                    *(buf + y));
            y--;
        }
        fprintf(stdout, "%s",
                nc->delimiter);
    }
    putchar('\n');

    return 0;
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
        fprintf(stdout, "Usage: bin2cipher INPUT\n");
        goto RELEASE;
    }
    if ((fp = fopen(argv[optind], "rb")) == NULL) {
        fprintf(stderr, "%s: %s\n",
                PROGNAME, strerror(errno));
        status = 3; goto RELEASE;
    }
    status = bin_to_cipher(fp, nc);

RELEASE:
    if (fp != NULL)
        fclose(fp);

    nc->release(nc);

    return status;
}
