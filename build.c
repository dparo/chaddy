#include <stdlib.h>
#if 0
#/*
set -x
exe="$(mktemp)"
cc build.c -o "$exe" -lcurl -larchive -lssl -lcrypto && "$exe"
exit "$?"
# */
#endif

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

char cwd[PATH_MAX];
char build_dir[PATH_MAX];
char build_ninja_path[PATH_MAX];

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

static void ninja_add_var(FILE *file, char *key, char *value);
static void ninja_add_rule(FILE *file, NinjaRule *r);
static void ninja_add_cc_rule(FILE *file, const char *name, const char *command);
static void ninja_add_build(FILE *file, NinjaBuild *b);
static void ninja_add_default(FILE *file, char *target);

static void ninja_setup_rules(FILE *file);

/*
    UTILS
*/
static int utils_is_executable_on_path(const char *exe_name);
static int utils_create_directory(const char *path, mode_t mode);

static int init();

int main(int argc, char **argv) {
    int rc;
    if (!(rc = init())) {
        return rc;
    }

    FILE *file = fopen(build_ninja_path, "w");
    ninja_setup_rules(file);
    ninja_add_build(file, &(NinjaBuild){.outputs = {"helloworld"},
                                        .name = "cc",
                                        .inputs = {"helloworld.c", "src/sum.c"},
                                        .dependencies = {0}});
    // ninja_add_default(file, "main");

    fclose(file);

    return execl("/usr/bin/ninja", "/usr/bin/ninja", "-C", build_dir, "-f", build_ninja_path, NULL);
}

int init() {
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s\n", cwd);
    } else {
        perror("getcwd() error");
        return EXIT_FAILURE;
    }

    snprintf(build_dir, sizeof(build_dir), "%s/build", cwd);
    snprintf(build_ninja_path, sizeof(build_ninja_path), "%s/build.ninja", build_ninja_path);

    if (utils_create_directory(build_dir, 0755) != 0) {
        perror("mkdir failed");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int utils_is_executable_on_path(const char *exe_name) {
    if (!exe_name || strlen(exe_name) == 0) {
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
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, exe_name);

        // Check if the file exists and is executable
        if (access(full_path, X_OK) == 0) {
            free(path_dup);
            return 1;
        }

        dir = strtok(NULL, ":");
    }

    free(path_dup);
    return 0;
}

void ninja_setup_rules(FILE *file) {
    ninja_add_var(file, "CFLAGS", "-Wall -Werror");
    ninja_add_var(file, "IDIRS", "");
    ninja_add_var(file, "LDFLAGS", "");
    {
        char compile_command[4096];
        snprintf(compile_command, sizeof(compile_command),
                 "%scc -MD -MF $out.d -o $out $CFLAGS $IDIRS $in",
                 utils_is_executable_on_path("bear") ? "bear -- " : "");

        ninja_add_rule(file, &(NinjaRule){.name = "cc", .command = compile_command, .vars = {}});
    }
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
