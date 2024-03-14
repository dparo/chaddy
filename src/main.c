// SPDX-FileCopyrightText: 2023 Davide Paro
//
// SPDX-License-Identifier: MIT

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <unistd.h>

#include "std.h"
#include "version.h"
#include "html5.h"

#include <log.h>
#include <argtable3.h>
#include <hedley.h>

enum {
    MAX_NUMBER_OF_ERRORS_TO_DISPLAY = 16,
};

static void print_brief_description(const char *progname);
static void print_version(void);
static void print_use_help_for_more_information(const char *progname);

static int main2(const char **defines, int32_t num_defines) {
    (void)defines;
    (void)num_defines;

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;

    char sendBuff[1025];
    time_t ticks;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd <= 0) {
        perror("Failed to create listen socket");
        exit(EXIT_FAILURE);
    }

    // Reuse address for quick development
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    uint16_t port = 5000;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    int rc = 0;
    rc = bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (rc == -1) {
        perror("Failed to bind");
        exit(EXIT_FAILURE);
    }

    rc = listen(listenfd, 10);
    if (rc == -1) {
        perror("Failed to listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d ...\n", port);
    while (1) {
        connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
        if (connfd <= 0) {
            perror("Failed to accept");
            exit(EXIT_FAILURE);
        }

        char http_req[64 * 1024];
        ssize_t nread = 0;

        // NOTE(d.paro): Dumb GET HTTP body extractor. Completely unsafe !!!!
        while ((nread = read(connfd, http_req, sizeof(http_req))) > 0) {
            printf("%.*s", (int) nread, http_req);
            fflush(stdout);

            if (http_req[nread - 2] == '\r' && http_req[nread - 1] == '\n') {
                break;
            }
        }

        char *first_space = strchr(http_req, ' ');
        char *second_space = strchr(first_space + 1, ' ');


        printf("first_space offset: %zu\n", first_space - http_req);
        printf("second_space offset: %zu\n", second_space - http_req);

        char http_verb[16];
        char http_path[1024] = {0};

        strncpy(http_path, first_space + 1, MIN((uintptr_t) ARRAY_LEN(http_path), (uintptr_t) second_space - (uintptr_t) first_space - 1));
        strncpy(http_verb, http_req, MIN((uintptr_t) ARRAY_LEN(http_verb), (uintptr_t) first_space - (uintptr_t) http_req));


        printf("\n\n");
        printf("Parsed HTTP Verb: %s\n", http_verb);
        printf("Parsed requested path: %s\n", http_path);
        printf("\n");


        if (0 == strcasecmp("get", http_verb)) {
            if (0 == strcmp("/", http_path) || 0 == strcmp("/index.html", http_path)) {

                FILE *conn = fdopen(connfd, "w");
                char buffer[64 * 1024] = {0};
                FILE *f = fmemopen(buffer, ARRAY_LEN(buffer), "w");


                HtmlRenderer r = {0};
                r.fstream = f;

                html5_render_raw_text(&r, "<!DOCTYPE html>\n");
                HTML(&r, {"lang", "en"}) {
                    HEAD(&r) {
                        const char title[] = "CHADDY <&'>";
                        META(&r, {"charset", "utf-8"});
                        META(&r, {"http-equiv", "content-language"}, {"content", "en"});
                        META(&r, {"name", "title"}, {"content", title});
                        TITLE(&r, title);
                        SCRIPT(&r, NULL, {"src", "https://unpkg.com/htmx.org@1.9.10"});
                    }
                    BODY(&r) {
                        INPUT(&r, {"type", "checkbox"}, {"checked", NULL}, {"name", "cheese"},
                              {rand() % 2 ? "disabled" : NULL, NULL});
                        BR(&r);
                        for (int i = 0; i < 100; i++) {
                            char buf[128] = {0};
                            snprintf(buf, ARRAY_LEN(buf), "Hello world %d", i);
                            bool cond = rand() % 2;
                            B_IF(&r, cond, {"class", "foo"}, {"style", "bold"}) {
                                html5_render_escaped(&r, buf);
                            }
                            BR(&r);
                        }
                    }
                }

                fflush(f);
                fseek(f, 0L, SEEK_END);
                long int sz = ftell(f);

                fprintf(conn, "%s\r\n", "HTTP/1.1 200 OK");
                fprintf(conn, "%s: %zu\r\n", "Content-Length", sz);
                fprintf(conn, "%s: %s\r\n", "Content-Type", "text/html; charset=utf-8");
                fprintf(conn, "%s: %s\r\n", "Connection", "close");
                fprintf(conn, "%s: %s\r\n", "Server-Timing", "miss, db;dur=53, app;dur=47.2");
                fprintf(conn, "\r\n");
                fflush(conn);


                // MSG_NOSIGNAL Prevents SIGPIPE sginal when writing
                // to sockets that were prematurely closed on the cliends end
                send(connfd, buffer, (size_t)sz, MSG_NOSIGNAL);

                fclose(f);
                // close(connfd);
            } else if (0 == strcmp("/favicon.ico", http_path)) {
                // TODO(d.paro): Serve the favicon
                close(connfd);
            } else {
                FILE *conn = fdopen(connfd, "w");

                fprintf(conn, "%s\r\n", "HTTP/1.1 404 Not Found");
                fprintf(conn, "%s: %zu\r\n", "Content-Length", 0L);
                fprintf(conn, "%s: %s\r\n", "Connection", "close");
                fprintf(conn, "\r\n");

                fflush(conn);
                fclose(conn);
            }
        } else {
        }

        close(connfd);
        close(connfd);
    }

    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {

    const char *progname = argv[0];
    FILE *log_file_handle = NULL;

    if (argc == 1) {
        print_brief_description(progname);
        print_version();
        printf("\n");
        print_use_help_for_more_information(progname);
        return EXIT_FAILURE;
    }

    struct arg_str *defines =
        arg_strn("D", "define", "KEY=VALUE", 0, argc + 2, "define parameters");
    struct arg_lit *verbose = arg_lit0("v", "verbose", "verbose messages");
    struct arg_file *logfile =
        arg_file0("l", "log", NULL,
                  "specify an additional file where log information would be "
                  "stored (default none)");
    struct arg_lit *help = arg_lit0(NULL, "help", "print this help and exit");
    struct arg_lit *version = arg_lit0(NULL, "version", "print version information and exit");
    struct arg_end *end = arg_end(MAX_NUMBER_OF_ERRORS_TO_DISPLAY);

    void *argtable[] = {help, version, verbose, logfile, defines, end};

    int nerrors = 0;
    int exitcode = 0;

    /* verify the argtable[] entries were allocated successfully */
    if (arg_nullcheck(argtable) != 0) {
        printf("%s: insufficient memory\n", progname);
        exitcode = 1;
        goto exit;
    }

    // No logging file by default
    logfile->filename[0] = NULL;

    nerrors = arg_parse(argc, argv, argtable);

    /* special case: '--help' takes precedence over error reporting */
    if (help->count > 0) {
        print_brief_description(progname);
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        arg_print_glossary(stdout, argtable, "  %-32s %s\n");
        exitcode = 0;
        goto exit;
    }

    /* special case: '--version' takes precedence error reporting */
    if (version->count > 0) {
        print_version();
        exitcode = 0;
        goto exit;
    }

    /* If the parser returned any errors then display them and exit */
    if (nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        print_use_help_for_more_information(progname);
        exitcode = 1;
        goto exit;
    }

    if (verbose->count > 0) {
        log_set_level(LOG_INFO);
    } else {
        log_set_level(LOG_WARN);
    }

    if (logfile->count > 0) {
        log_file_handle = fopen(logfile->filename[0], "w");
        if (log_file_handle) {
            log_add_fp(log_file_handle, LOG_INFO);
        } else {
            fprintf(stderr, "%s: Failed to open for logging\n", logfile->filename[0]);
        }
    }

    exitcode = main2(defines->sval, defines->count);

exit:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

    if (log_file_handle) {
        fclose(log_file_handle);
        log_file_handle = NULL;
    }

    return exitcode;
}

static void print_brief_description(const char *progname) {
    (void)progname;
    printf("%s: %s\n", PROJECT_NAME, PROJECT_DESCRIPTION);
}

static void print_version(void) {
    printf("%s v%s (%s, revision: %s)\n", PROJECT_NAME, PROJECT_VERSION, GIT_DATE, GIT_SHA1);
    printf("Compiled with %s v%s (%s), %s build\n", C_COMPILER_ID, C_COMPILER_VERSION,
           C_COMPILER_ABI, BUILD_TYPE);
}
static void print_use_help_for_more_information(const char *progname) {
    printf("Try '%s --help' for more information.\n", progname);
}
