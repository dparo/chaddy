#include <string.h>
#include <stdint.h>

#define MOD_HASH 1024
#define HASH_COLLISION_UPPER_BOUND 4

typedef struct {
    size_t size;
    char *data;
} BAKED_FILE_HANDLE;

typedef uint16_t hash_t;

static hash_t adler32(char *input) {
    return 0;
}

/* Type your code here, or load an example. */
int square(char *input) {

    if (input != NULL && input[0] == '/' ) {
        input = input + 1;
    } else if (input != NULL && input[0] == '.' && input[1] == '/') {
        input = input + 2;
    }

    hash_t hash = adler32(input);

}

typedef struct hash_entry {
    int32_t cnt;
    struct {
        hash_t hash;
        size_t path_len;
        char *path;
        char *mimetype;
    } enties[HASH_COLLISION_UPPER_BOUND];
} hash_entry;


static hash_entry hashes[UINT16_MAX] = {
    [10] = {1, { {10, strlen("foobar"), "foobar", "text/html; charset=utf-8"} }},
};
