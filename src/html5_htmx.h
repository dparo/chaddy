#pragma once

#if __cplusplus
extern "C" {
#endif

#include "html5.h"

typedef enum {
    HX_SWAP_NONE = 0,
    HX_SWAP_DELETE,
    HX_SWAP_MORPH,
    HX_SWAP_INNER_HTML,
    HX_SWAP_OUTER_HTML,
    HX_SWAP_TEXT_CONTENT,
    HX_SWAP_BEFORE_BEGIN,
    HX_SWAP_AFTER_BEGIN,
    HX_SWAP_BEFORE_END,
    HX_SWAP_AFTER_END,
} HtmxSwapStrategy;

static const HtmlAttrib HX_SWAP_NONE_ATTRIB = {"hx-swap", "none"};
static const HtmlAttrib HX_SWAP_DELETE_ATTRIB = {"hx-swap", "delete"};
static const HtmlAttrib HX_SWAP_MORPH_ATTRIB = {"hx-swap", "morph"};
static const HtmlAttrib HX_SWAP_INNER_HTML_ATTRIB = {"hx-swap", "innerHTML"};
static const HtmlAttrib HX_SWAP_OUTER_HTML_ATTRIB = {"hx-swap", "outerHTML"};
static const HtmlAttrib HX_SWAP_TEXT_CONTENT_ATTRIB = {"hx-swap", "textContent"};
static const HtmlAttrib HX_SWAP_BEFORE_BEGIN_ATTRIB = {"hx-swap", "beforebegin"};
static const HtmlAttrib HX_SWAP_AFTER_BEGIN_ATTRIB = {"hx-swap", "afterbegin"};
static const HtmlAttrib HX_SWAP_BEFORE_END_ATTRIB = {"hx-swap", "beforeend"};
static const HtmlAttrib HX_SWAP_AFTER_END_ATTRIB = {"hx-swap", "afterend"};

static const HtmlAttrib HX_PRELOAD_EXT_MOUSEDOWN_ATTRIB = {"preload", "mousedown"};

static inline HtmlAttrib hx_swap_attrib(char *s) {
    const HtmlAttrib result = {.key = "hx-swap", .value = s};
    return result;
}

static inline HtmlAttrib hx_target_attrib(char *s) {
    const HtmlAttrib result = {.key = "hx-target", .value = s};
    return result;
}

#if __cplusplus
}
#endif
