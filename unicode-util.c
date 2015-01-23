/*
 * Copyright (C) 2015 Simon Aittamaa
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>

#include "unicode.h"

enum action_t {
        ACTION_UNKNOWN,
        ACTION_CONVERT,
        ACTION_TEST,
};

enum encoding_t {
        ENCODING_UNKNOWN,
        ENCODING_UTF8,
        ENCODING_UTF16,
        ENCODING_UCS2,
        ENCODING_UCS4,
};

static enum encoding_t to_encoding(const char *name) {
        if (!strcmp(optarg, "utf8")) {
                return ENCODING_UTF8;
        } else if (!strcmp(optarg, "utf16")) {
                return ENCODING_UTF16;
        } else if (!strcmp(optarg, "ucs2")) {
                return ENCODING_UCS2;
        } else if (!strcmp(optarg, "ucs4")) {
                return ENCODING_UCS4;
        } else {
                return ENCODING_UNKNOWN;
        }
}

#define BUF_SIZE 1*1024*1024

static int convert(enum encoding_t input_encoding, enum encoding_t output_encoding) {
        uint8_t *buf;
        size_t rb, cb, off;
        FILE *input, *output;

        input = freopen(NULL, "rb", stdin);
        if (!input) {
                fprintf(stderr, "freopen(stdin) failed: %s\n", strerror(errno));
                return -1;
        }

        output = freopen(NULL, "wb", stdout);
        if (!output) {
                fprintf(stderr, "freopen(stdout) failed: %s\n", strerror(errno));
                return -1;
        }

        buf = calloc(BUF_SIZE, 1);
        if (!buf) {
                fprintf(stderr, "calloc() failed: %s\n", strerror(errno));
                return -1;
        }

        off = 0;
        rb = fread(buf, 1, BUF_SIZE, input);
        if (!feof(input)) {
                fprintf(stderr, "fread(): Input data too large (> %d)\n", BUF_SIZE);
                return -1;
        }

        if (ferror(input)) {
                fprintf(stderr, "ferror(): %s\n", strerror(errno));
                return -1;
        }

        for (;;) {
                if (off >= rb) {
                        break;
                }

                if (input_encoding == ENCODING_UTF8) {
                        uint32_t to;
                        const uint8_t *from = buf+off;
                        cb = utf8_to_ucs4(from, rb-off, &to);
                        off += cb;
                        if (!cb) {
                                fprintf(stderr, "Convertion to UTF-8 failed at offset %zu\n", off);
                                return -1;
                        }
                        fwrite(&to, sizeof(to), 1, output);
                } else if (input_encoding == ENCODING_UCS4) {
                        uint8_t to[4];
                        const uint32_t from = *((uint32_t *)(buf+off));
                        cb = ucs4_to_utf8(from, &to[0], rb-off);
                        off += sizeof(uint32_t);
                        if (!cb) {
                                fprintf(stderr, "Convertion to UCS-4 failed at offset %zu\n", off);
                                return -1;
                        }
                        fwrite(to, cb, 1, output);
                } else {
                        fprintf(stderr, "Invalid input-type: %d\n", input_encoding);
                        return -1;
                }
        }

        fflush(stderr);

        return 0;
}

static int test_all(void) {
        size_t len, i;
        uint8_t to[4];
        uint32_t from, conv;

        for (from=0;from<0x1fffff;from++) {
                len = ucs4_to_utf8(from, to, sizeof(to));
                if (!len) {
                        fprintf(stderr, "Conversion of UCS-4 value 0x%08x failed.\n", from);
                        return -1;
                }
                len = utf8_to_ucs4(to, len, &conv);
                if (!len) {
                        fprintf(stderr, "Conversion from UTF-8 encoding of UCS-4 value 0x%08x failed.\n", from);
                        return -1;
                }

                if (conv != from) {
                        fprintf(stderr, "Converted value missmatch:\n");
                        fprintf(stderr, "Original: 0x%08x\n", from);
                        fprintf(stderr, "USC-4: 0x%08x.\n", conv);
                        fprintf(stderr, "UTF-8:");
                        for (i=0;i<len;i++) {
                                fprintf(stderr, " 0x%02x", to[i]);
                        }
                        fprintf(stderr, "\n");
                        return -1;
                }
        }

        return 0;
}

static const struct option options[] = {
        {
                .name = "from",
                .has_arg = required_argument,
                .flag = NULL,
                .val = 0,
        },
        {
                .name = "to",
                .has_arg = required_argument,
                .flag = NULL,
                .val = 0,
        },
        {
                .name = "convert",
                .has_arg = no_argument,
                .flag = NULL,
                .val = 0,
        },
        {
                .name = "test",
                .has_arg = no_argument,
                .flag = NULL,
                .val = 0,
        },
        {
                .name = "help",
                .has_arg = no_argument,
                .flag = NULL,
                .val = 0,
        },
        {NULL}
};

static void print_help(const char *name) {
        fprintf(stderr, "Usage: %s <options>\n", name);
        fprintf(stderr, "\t--to=<encoding>   Set input encoding.\n");
        fprintf(stderr, "\t--from=<encoding> Set input encoding.\n");
        fprintf(stderr, "\t--convert         Perform convertion using to and from encoding.\n");
        fprintf(stderr, "\t--test            Perform encoding and decoding test across valid range.\n");
        fprintf(stderr, "\n");
}

int main(int argc, char **argv) {
        int show_help = 0;
        enum encoding_t input_encoding, output_encoding;
        enum action_t action = ACTION_UNKNOWN;

        for (;;) {
                int option;
                int result = getopt_long(argc, argv, "", options, &option);
                if (result == -1) {
                        break;
                } else if (result != 0) {
                        fprintf(stderr, "getopt_long() failed: %d\n", result);
                }

                if (!strcmp(options[option].name, "to")) {
                        output_encoding = to_encoding(optarg);
                        if (output_encoding == ENCODING_UNKNOWN) {
                                fprintf(stderr, "Unknown output encoding: %s\n", optarg);
                                return EXIT_FAILURE;
                        }
                } else if (!strcmp(options[option].name, "from")) {
                        input_encoding = to_encoding(optarg);
                        if (input_encoding == ENCODING_UNKNOWN) {
                                fprintf(stderr, "Unknown input encoding: %s\n", optarg);
                                return EXIT_FAILURE;
                        }
                } else if (!strcmp(options[option].name, "test")) {
                        action = ACTION_TEST;
                } else if (!strcmp(options[option].name, "convert")) {
                        action = ACTION_CONVERT;
                } else if (!strcmp(options[option].name, "help")) {
                        show_help = 1;
                } else {
                        fprintf(stderr, "Internal error, unhandled option: %s\n", options[option].name);
                        return EXIT_FAILURE;
                }
        }

        if (optind < argc) {
                fprintf(stderr, "Unknown arguments:\n");
                while (optind < argc) {
                        fprintf(stderr, "\t%s\n", argv[optind++]);
                }
                return EXIT_FAILURE;
        }

        if (action == ACTION_UNKNOWN || show_help) {
                if (action == ACTION_UNKNOWN && !show_help) {
                        fprintf(stderr, "No action specified.\n");
                }
                char *tmp = strdup(argv[0]);
                print_help(basename(tmp));
                free(tmp);
                return show_help?EXIT_SUCCESS:EXIT_FAILURE;
        }

        switch (action) {
                case ACTION_TEST:
                        if (test_all()) {
                                return EXIT_FAILURE;
                        }
                        break;
                case ACTION_CONVERT:
                        if (convert(input_encoding, output_encoding)) {
                                return EXIT_FAILURE;
                        }
                        break;
                default:
                        fprintf(stderr, "Internal error, unhandled action %d.\n", action);
                        return EXIT_FAILURE;
                        break;
        }

        return EXIT_SUCCESS;
}
