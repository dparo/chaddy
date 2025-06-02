#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

typedef struct {
    void *(*alloc)(size_t size, size_t alignment, size_t lifetime);
    void (*clear)(size_t lifetime, size_t reserve); // [0, MAX_INT32]
} MArena_VTable_t;

typedef enum {
    TEMP_LIFETIME,
    REQUEST_LIFETIME,
    SESSION_LIFETIME,

    // Connection based
    SOCKET_LIFETIME,
    // TODO(d.paro): Handle websockets
} WebServerMemArenaLifetimes;


// T[N]     |    T[N + 1]          | T[N + 2]
// 4 MB  -> 256K, 10 MB   -> 256K

// TEMP_ALLOCATOR
// 14MB -> Ti prendi l'output e lo allochi


// TODO(d.paro): Capire modellazione della generazione
#define MAX_GENERATIONS 10
#define MAX_LIFETIMES 32
typedef struct {
    size_t generation_start[MAX_LIFETIMES];
    size_t generation_end[MAX_LIFETIMES];

    void *buffer[MAX_LIFETIMES][MAX_GENERATIONS + 1];

    size_t num_fress[MAX_LIFETIMES][MAX_GENERATIONS + 1];
    void *frees[MAX_LIFETIMES][MAX_GENERATIONS + 1][8192];
    void (*frees_fns[MAX_LIFETIMES][MAX_GENERATIONS + 1][8192])(void *ptr);

    MArena_VTable_t vtable;
} MArena_t;

#define ALLOC_TYPE(vtable, T, lifetime) (vtable).alloc(sizeof(T), alignof(T), (lifetime))

int main(int argc, char **argv) {
    MArena_t temp_arena = {0};
    MArena_t request_arena = {0};
    MArena_t session_arena = {0};

    MArena_t web_server_mem_arena = {0};

    return 0;
}
