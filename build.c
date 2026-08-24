#if 0
#/*
mkdir -p build && cc build.c -o ./build/build.c -lcurl -lz -lssl -lcrypto && ./build/build.c "$@"
exit "$?"
# */
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#include "tinyargs.h"

char cwd[PATH_MAX];
char build_dir[PATH_MAX];
char build_ninja_path[PATH_MAX];
char ninja_path[PATH_MAX];

typedef enum BuildKind {
    DEBUG = (1 << 0),
    RELEASE = (1 << 1),
    OPTIMIZED = (1 << 2),
} BuildKind;

typedef enum BuildFlags {
    ASAN = (1 << 0),
    TSAN = (1 << 1),
    USE_BEAR_TO_GENERATE_COMPILE_COMMANDS,
} BuildFlags;

typedef struct BuildConfig {
    BuildKind buildKind;
} BuildConfig;

static BuildConfig config = {

};

/*
    Ninja
*/
typedef struct KeyValuePair {
    char *key;
    char *value;
} KeyValuePair;

typedef struct NinjaRule {
    char *name;
    char *command;
    KeyValuePair vars[4096];
} NinjaRule;

typedef struct NinjaBuild {
    char *name;
    char *outputs[4096];
    char *inputs[4096];
    char *dependencies[4096];
    KeyValuePair params[4096];
} NinjaBuild;

static void ninja_lex_write_escape(FILE *f, char *s, bool escape_colon);
static void ninja_add_var(FILE *file, char *key, char *value);
static void ninja_add_rule(FILE *file, NinjaRule *r);
static void ninja_add_cc_rule(FILE *file, const char *name, const char *command);
static void ninja_add_build(FILE *file, NinjaBuild *b);
static void ninja_add_default(FILE *file, char *target);

static void ninja_setup_rules(FILE *file, char *build_dir);
static int utils_resolve_path(const char *exe, char out[PATH_MAX]);
size_t shquote(const char *input, char *output, size_t outputsize);

/*
    UTILS
*/
static int utils_is_executable_on_path(const char *exe_name);
static int utils_create_directory(const char *path, mode_t mode);

static int init();

int main(int argc, char **argv) {

    int port = 8080;
    bool verbose = false;
    const char *name = "world";

    ta_option opts[] = {
        TA_INT   ("port",    'p', &port,    "server port"),
        TA_BOOL  ("verbose", 'v', &verbose, "enable verbose output"),
        TA_STR   ("name",    'n', &name,    "name to greet"),
    };

    if (ta_parse(argc, argv, opts, 3, argv[0]) < 0)
        return 1;


    int rc;
    if ((rc = init()) != 0) {
        return rc;
    }
    if (!utils_resolve_path("ninja", ninja_path)) {
        fprintf(stderr, "Ninja was not found. Please make sure Ninja is installed: https://ninja-build.org/");
        return EXIT_FAILURE;
    }

    FILE *file = fopen(build_ninja_path, "w");
    ninja_setup_rules(file, build_dir);
    ninja_add_build(file, &(NinjaBuild){.outputs = {"helloworld"},
                                        .name = "cc",
                                        .inputs = {"helloworld.c", "src/sum.c"},
                                        .dependencies = {0}});
    // ninja_add_default(file, "main");

    fclose(file);


    printf("\n");


    char *ninja_pathname = ninja_path[0] ? ninja_path  : "/usr/bin/ninja";
    char *ninja_command_with_args[] = {
        ninja_pathname,
        "-C", build_dir,
        "-f", build_ninja_path,
        // "--verbose",
        NULL
    };

    for (int i = 0; ninja_command_with_args[i]; i++) {
        char buf[4096] = {0};
        shquote(ninja_command_with_args[i], buf, sizeof(buf));
        if (i == 0) {
            printf("+ %s", buf);
        } else {
            printf(" %s", buf);
        }
    }
    printf("\n");

    // Execute Ninja
    return execv(
        ninja_pathname,
        ninja_command_with_args
    );
}

int init() {
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("[build.c] getcwd() error");
        return EXIT_FAILURE;
    }

    snprintf(build_dir, sizeof(build_dir), "%s/build", cwd);
    snprintf(build_ninja_path, sizeof(build_ninja_path), "%s/build.ninja", build_dir);

    printf("[build.c] CWD: %s\n", cwd);
    printf("[build.c] Build dir: %s\n", build_dir);
    printf("[build.c] build.ninja: %s\n", build_ninja_path);

    if (utils_create_directory(build_dir, 0755) != 0) {
        perror("[build.c] mkdir failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int utils_resolve_path(const char *exe, char output[PATH_MAX]) {
    if (!exe || strlen(exe) == 0) {
        return 0;
    }

    char *path_env = getenv("PATH");
    if (!path_env) {
        return 0;
    }

    // Duplicate PATH to avoid modifying the original environment variable
    char *path_dup = strdup(path_env);
    if (!path_dup) {
        return 0;
    }

    char *dir = strtok(path_dup, ":");
    while (dir) {
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, exe);

        // Check if the file exists and is executable
        if (access(full_path, X_OK) == 0) {
            free(path_dup);
            if (output != NULL) {
                snprintf(output, PATH_MAX, "%s", full_path);
            }
            return 1;
        }

        dir = strtok(NULL, ":");
    }

    free(path_dup);
    return 0;
}

int utils_is_executable_on_path(const char *exe_name) {
    return utils_resolve_path(exe_name, NULL);
}

size_t shquote(const char *input, char *output, size_t out_numbytes) {
    size_t l = strlen(input);

    size_t num_quotes = 0;
    for (size_t i = 0; input[i]; i++) {
        if (input[i] == '\'') {
            num_quotes += 1;
        }
    }

    size_t required_size = l + (num_quotes == 0 ? 0 : 2) + num_quotes * 5;

    if (output) {
        size_t off = 0;
        if (num_quotes != 0) {
            if (out_numbytes - off > 0) {
                output[off++] = '\''; // open quote
            }
        }
        for (int32_t i = 0; input[i]; i++) {
            if (input[i] == '\'') {
                if (out_numbytes - off - 5 > 0) {
                    output[off + 0] = '\'';
                    output[off + 1] = '"';
                    output[off + 2] = '\'';
                    output[off + 3] = '"';
                    output[off + 4] = '\'';
                }
                off += 5;
            } else {
                if (out_numbytes - off > 0) {
                    output[off++] = input[i];
                }
            }
        }
        if (num_quotes != 0) {
            if (out_numbytes - off > 0) {
                output[off++] = '\''; // closing quote
            }
        }
        if (out_numbytes - off > 0) {
            output[off++] = '\0';
        }
    }

    // Guarantee null terminator
    if (output && out_numbytes >= 1) {
        output[out_numbytes - 1] = '\0';
    }

    return required_size;
}


void ninja_lex_write_escape(FILE *f, char *s, bool escape_colon) {
    char buf[4096] = {0};
}

void ninja_setup_rules(FILE *file, char *build_dir) {
    ninja_add_var(file, "builddir", build_dir);
    ninja_add_var(file, "ninja_required_version", "1.10");
    fprintf(file, "\n");
    ninja_add_var(file, "CFLAGS", "-Wall -Werror");
    ninja_add_var(file, "IDIRS", "");
    ninja_add_var(file, "LDFLAGS", "");

    {
        char compile_command[4096];
        snprintf(compile_command, sizeof(compile_command),
                 "%scc -MMD -MF $out.d -o $out $CFLAGS $IDIRS $in",
                 utils_is_executable_on_path("bear") ? "bear -- " : "");

        ninja_add_rule(file, &(NinjaRule){.name = "cc", .command = compile_command, .vars = {
            { .key = "description", .value = "cc $in"},
        }});
    }

    // {
    //     ninja_add_rule(file, &(NinjaRule){.name = "build.ninja", .command = "cc -o $out $in", .vars = {}});
    // }
    //
    // ninja_add_build(file, &(NinjaBuild){.outputs = {build_ninja_path},
    //                                     .name = "build.ninja",
    //                                     .inputs = {"build.c"},
    //                                     .dependencies = {""}});


}

int utils_create_directory(const char *path, mode_t mode) {
    if (mkdir(path, mode) == 0 || errno == EEXIST) {
        return 0; // Success or directory already exists
    }
    return -1; // Other errors
}

void ninja_add_var(FILE *file, char *key, char *value) {
    if (key && key[0]) {
        fprintf(file, "%s = %s\n", key, value ? value : "");
    }
}

void ninja_add_rule(FILE *file, NinjaRule *r) {
    fprintf(file, "rule %s\n", r->name);
    fprintf(file, "  command = %s\n", r->command);
    if (r->vars[0].key) {
        for (int32_t i = 0; r->vars[i].key; i++) {
            fprintf(file, "  %s = %s\n", r->vars[i].key, r->vars[i].value ? r->vars[i].value : "");
        }
    }
}

void ninja_add_cc_rule(FILE *file, const char *name, const char *command) {
    fprintf(file, "rule %s\n  command = %s\n", name, command);
}

void ninja_add_build(FILE *file, NinjaBuild *b) {
    if (b->outputs[0]) {
        fprintf(file, "build");
        for (int32_t i = 0; b->outputs[i]; i++) {
            fprintf(file, " %s", b->outputs[i]);
        }
        fprintf(file, ": %s", b->name);
        if (b->inputs[0]) {
            for (int32_t i = 0; b->inputs[i]; i++) {
                fprintf(file, " %s/%s", cwd, b->inputs[i]);
            }
        }

        fprintf(file, " |");
        {
            char path[4096] = {0};
            snprintf(path, sizeof(path), "%s/build.c", cwd);
            fprintf(file, " %s", path);
        }
        if (b->dependencies[0]) {
            for (int32_t i = 0; b->dependencies[i]; i++) {
                fprintf(file, " %s", b->dependencies[i]);
            }
        }

        fprintf(file, "\n");
        if (b->params[0].key) {
            for (int32_t i = 0; b->params[i].key; i++) {
                fprintf(file, "  %s = %s\n", b->params[i].key,
                        b->params[i].value ? b->params[i].value : "");
            }
        }
    }
}

void ninja_add_default(FILE *file, char *target) { fprintf(file, "default %s\n", target); }
