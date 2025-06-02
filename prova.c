#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *quote_for_bash(const char *input) {
    size_t len = strlen(input);
    size_t new_len = len + 2; // For surrounding single quotes

    // Count the number of single quotes in the input
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '\'') {
            new_len += 4; // Each single quote requires closing, escaping, and reopening
        }
    }

    // Allocate memory for the new string
    char *quoted = malloc(new_len + 1);
    if (!quoted) {
        return NULL;
    }

    char *ptr = quoted;
    *ptr++ = '\''; // Opening quote

    for (size_t i = 0; i < len; i++) {
        if (input[i] == '\'') {
            *ptr++ = '\'';  // Close current quote
            *ptr++ = '\\';  // Escape character
            *ptr++ = '\'';  // The actual single quote
            *ptr++ = '\'';  // Reopen quotes
        } else {
            *ptr++ = input[i];
        }
    }

    *ptr++ = '\''; // Closing quote
    *ptr = '\0';

    return quoted;
}

int main() {
    const char *test_str = "This is 'quoted' for bash";
    char *quoted = quote_for_bash(test_str);
    if (quoted) {
        printf("Quoted: %s\n", quoted);
        free(quoted);
    }
    return 0;
}
