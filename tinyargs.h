/*
 * tinyargs.h
 *
 * Usage:
 *
 *   int port = 8080;
 *   bool verbose = false;
 *   const char *name = "world";
 *
 *   ta_option opts[] = {
 *       TA_INT("port",   'p', &port,    "listen port"),
 *       TA_BOOL("verbose",'v', &verbose,"be noisy"),
 *       TA_STR("name",   'n', &name,    "name to use"),
 *   };
 *
 *   if (ta_parse(argc, argv, opts, 3, argv[0]) < 0)
 *       return 1;
 */

#ifndef TINYARGS_H
#define TINYARGS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
    TA_INT,
    TA_BOOL,
    TA_STRING,
    TA_FLAG
} ta_type;

typedef struct {
    const char *long_name;
    char short_name;
    ta_type type;
    void *value;
    const char *help;
} ta_option;


/* ---------- option declarations ---------- */

#define TA_INT(name, short_name, ptr, help) \
    { name, short_name, TA_INT, ptr, help }

#define TA_BOOL(name, short_name, ptr, help) \
    { name, short_name, TA_BOOL, ptr, help }

#define TA_STR(name, short_name, ptr, help) \
    { name, short_name, TA_STRING, ptr, help }

#define TA_FLAG(name, short_name, ptr, help) \
    { name, short_name, TA_FLAG, ptr, help }


/* ---------- implementation ---------- */

static void ta_help(
    const char *program,
    const ta_option *opts,
    int nopts)
{
    int i;

    printf("Usage: %s [options]\n\n", program);

    printf("Options:\n");
    printf("  -h, --help              Show this help\n");

    for (i = 0; i < nopts; ++i) {
        const ta_option *o = &opts[i];

        printf("  ");

        if (o->short_name)
            printf("-%c", o->short_name);
        else
            printf("  ");

        if (o->long_name) {
            if (o->short_name)
                printf(", ");
            else
                printf("  ");

            printf("--%s", o->long_name);
        }

        switch (o->type) {
        case TA_INT:
            printf(" <int>");
            break;
        case TA_STRING:
            printf(" <string>");
            break;
        default:
            break;
        }

        if (o->help)
            printf("%*s%s",
                   24 - (int)strlen(o->long_name) -
                   (o->short_name ? 4 : 2),
                   "",
                   o->help);

        putchar('\n');
    }
}


static ta_option *ta_find_long(
    ta_option *opts,
    int nopts,
    const char *name)
{
    int i;

    for (i = 0; i < nopts; ++i) {
        if (opts[i].long_name &&
            strcmp(opts[i].long_name, name) == 0)
            return &opts[i];
    }

    return NULL;
}


static ta_option *ta_find_short(
    ta_option *opts,
    int nopts,
    char name)
{
    int i;

    for (i = 0; i < nopts; ++i) {
        if (opts[i].short_name == name)
            return &opts[i];
    }

    return NULL;
}


static int ta_set(
    ta_option *o,
    const char *arg)
{
    char *end;

    switch (o->type) {

    case TA_BOOL:
    case TA_FLAG:
        *(bool *)o->value = true;
        return 0;

    case TA_INT: {
        long v = strtol(arg, &end, 10);

        if (*arg == '\0' || *end != '\0') {
            fprintf(stderr,
                    "invalid integer for --%s: %s\n",
                    o->long_name, arg);
            return -1;
        }

        *(int *)o->value = (int)v;
        return 0;
    }

    case TA_STRING:
        *(const char **)o->value = arg;
        return 0;
    }

    return -1;
}


/*
 * Returns:
 *   0  success
 *  -1  error
 *
 * argc/argv are allowed to be modified internally.
 * Parsing stops at "--".
 *
 * Positional arguments are left in argv.
 * The return value does not give their count, so use
 * the returned argc if you want that behavior in your
 * own wrapper, or add a positional callback.
 */
static int ta_parse(
    int argc,
    char **argv,
    ta_option *opts,
    int nopts,
    const char *program)
{
    int i;

    for (i = 1; i < argc; ++i) {
        char *arg = argv[i];
        ta_option *o = NULL;
        const char *value = NULL;

        if (strcmp(arg, "--") == 0)
            break;

        if (strcmp(arg, "--help") == 0 ||
            strcmp(arg, "-h") == 0) {
            ta_help(program, opts, nopts);
            exit(0);
        }

        /*
         * Long option:
         *
         *   --foo bar
         *   --foo=bar
         */
        if (strncmp(arg, "--", 2) == 0) {
            char *name = arg + 2;
            char *equals = strchr(name, '=');

            if (equals) {
                *equals = '\0';
                value = equals + 1;
            }

            o = ta_find_long(opts, nopts, name);

            if (!o) {
                fprintf(stderr, "unknown option: --%s\n", name);
                return -1;
            }
        }

        /*
         * Short option:
         *
         *   -v
         *   -p 8080
         *   -p8080
         */
        else if (arg[0] == '-' && arg[1] != '\0') {
            char name = arg[1];

            o = ta_find_short(opts, nopts, name);

            if (!o) {
                fprintf(stderr, "unknown option: -%c\n", name);
                return -1;
            }

            if (arg[2] != '\0')
                value = arg + 2;
        }

        /*
         * Positional argument.
         *
         * We simply leave it alone.
         */
        else {
            continue;
        }

        if (o->type == TA_BOOL || o->type == TA_FLAG) {
            if (value) {
                fprintf(stderr,
                        "option does not take a value: %s\n",
                        arg);
                return -1;
            }

            if (ta_set(o, NULL) < 0)
                return -1;

            continue;
        }

        if (!value) {
            if (i + 1 >= argc) {
                fprintf(stderr,
                        "missing value for %s\n",
                        arg);
                return -1;
            }

            value = argv[++i];
        }

        if (ta_set(o, value) < 0)
            return -1;
    }

    return 0;
}

#endif /* TINYARGS_H */
