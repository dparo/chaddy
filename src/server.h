#pragma once

#if __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef enum ServerCfg_TlsMode {
    AUTODETECT,
    FORCE_TLS_ENABLED,
    FORCE_TLS_DISABLED,
} ServerCfg_TlsMode;

typedef struct ServerCfg {
    bool devmode;
    bool hotrealoding_enabled;
    char *shared_object; /// Location of the shared object to reload

    bool automation; /// Enable automation trough remote invocation (server quit)

    bool websocket_support_disabled;

    char *host;
    int16_t port;
    ServerCfg_TlsMode tls;
} ServerCfg;


typedef struct Server {
    int threads[8];
    int http_parsers[8];
    int coro_state[8];
} Server;

typedef struct HttpHeader {
    char key[64];
    char value[1024];
} HttpHeader;


#define MAX_NUM_HEADERS 128
typedef struct HttpRequest {
    char verb[16];
    char path[4096];

    char qparams[4096];
    // hashmap of http headers
    char header_buf[64 * 1024];
    int32_t num_headers;

    // NOTE(d.paro): This leads to a linear scan in order to retrieve header value by key
    char *header_keys[MAX_NUM_HEADERS];
    char *header_values[MAX_NUM_HEADERS];
} HttpRequest;


#if __cplusplus
}
#endif

