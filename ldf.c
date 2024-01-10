// SPDX-License-Identifier: MIT
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mem_file.h"

const char * __progname;

static int usage(int argc, const char * argv[]);

int opt_pa = 0;
int opt_file = 0;

intptr_t mem_pa = ~0;
size_t mem_sz = 0;
const char * rfname = NULL;

static int parse_args(int argc, const char * argv[]) {
    int ret = EXIT_SUCCESS;
    int i;

    printf("argv[%d]:%s"NL, 0, argv[0]);
    for (i=1; i<argc; i++) {
        const char * arg = argv[i];
        if (*arg == '-') {
            if ((strcmp(arg, "-h") == 0) || (strcmp(arg, "--help") == 0)) {
                return usage(argc, argv);
            } else {
                printf("Unknown argument argv[%d]:%s"NL, 0, argv[i]);
            }
        } else if (!opt_pa) {
            mem_pa = strtoull(argv[i], NULL, 0);
            opt_pa = 1;
        } else if (!opt_file) {
            rfname = argv[i];

            if (rfname && rfname[0]) {
                struct stat sb;
                ret = stat(rfname, &sb);
                if (ret != 0) {
                    printf("%s: Cannot stat file '%s'!"NL, argv[0], rfname);
                    return EXIT_FAILURE;
                }
                if (S_ISDIR(sb.st_mode)) {
                    printf("%s: file '%s' is not a file!"NL, argv[0], rfname);
                    return EXIT_FAILURE;
                }
                mem_sz = sb.st_size;
            }
            opt_file = 1;
        } else {
            printf("Unknown argument argv[%d]:%s"NL, 0, argv[i]);
        }
    }

    if ((opt_pa == 0) || (opt_file == 0)) {
        return usage(argc, argv);
    }

    printf("ADDRESS=0x%lX FILE=%s(0x%lX)"NL, 
        mem_pa, rfname, mem_sz);

    return ret;
}
int main(int argc, const char * argv[]) {
    int ret = 0;

    __progname = argv[0];
    if ((ret = parse_args(argc, argv)) != EXIT_SUCCESS) {
        return ret;
    }

    if (!rfname || !rfname[0]) {
        return usage(argc, argv);
    }

    if ((ret = mem_ldfile(mem_pa, mem_sz, rfname)) != EXIT_SUCCESS) {
        return ret;
    }

    printf("done"NL);

    return EXIT_SUCCESS;
}

static int usage(int argc, const char * argv[]) {
    printf(
            "usage: %s [OPTIONS] ADDRESS FILE"NL
            ""NL
            "Load file to memory."NL
            ""NL
            "ADDRESS            Physical Address"NL
            "FILE               Image file"NL
            ""NL
            "OPTIONS"NL
            "-h, --help         Show this message"NL
            ""NL
            , argv[0]);
    fflush(stdout);
    return -1;
}

