// SPDX-FileCopyrightText: 2023 Davide Paro
// SPDX-FileCopyrightText: 2024 Davide Paro
//
// SPDX-License-Identifier: MIT

#pragma once

//
// WORK in progress.
//

#if __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "std.h"
#include "builtins.h"

#define HTML_ATTRIB_KEY_MAX_SIZE 32
#define HTML_MAX_NUM_ATTRS 64
#define HTML_ATTR_BUF_MAX_SIZE (16 * 1024)

typedef struct HtmlElem {
    char tag[32];
    char *text;

    struct {
        uint32_t num_keys;
        uint16_t val_buf_offsets[HTML_MAX_NUM_ATTRS + 1];
        char keys[HTML_ATTRIB_KEY_MAX_SIZE][HTML_MAX_NUM_ATTRS];
        char val_buf[HTML_ATTR_BUF_MAX_SIZE];
    } attrs;
} HtmlElem;

typedef struct HtmlTree {
    HtmlElem elem;
    int32_t num_childs;
    struct HtmlElem *childs[1024];
} HtmlTree;

typedef struct HtmlDoc {
    // <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"
    // "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
    char *doctype;
    HtmlElem *root;
} HtmlDoc;

static void html_push_attr(HtmlElem *elem, const char *key, const char *value) {
    // Check boundaries for key
    if (elem->attrs.num_keys >= HTML_MAX_NUM_ATTRS || strlen(key) >= HTML_ATTRIB_KEY_MAX_SIZE) {
        // TODO
        return;
    }

    size_t len = strlen(value);
    uint16_t curr_offset = elem->attrs.val_buf_offsets[elem->attrs.num_keys];
    uint16_t new_offset = curr_offset + (uint16_t)len;

    // Check boundaries for values buffer

    if (new_offset < HTML_ATTRIB_KEY_MAX_SIZE) {
        // TODO
        return;
    }

    // Copy key
    strcpy(elem->attrs.keys[elem->attrs.num_keys], key);
    // Copy value into the buffer
    elem->attrs.val_buf_offsets[elem->attrs.num_keys + 1] = new_offset;
    strcpy(&elem->attrs.val_buf[new_offset], value);
    // Update number of keys
    ++elem->attrs.num_keys;
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////

#define HTML_RENDERER_MAX_DEPTH 128
#define HTML_RENDERER_MAX_TAG_LEN 63
typedef struct HtmlRenderer {
    FILE *fstream;
    int32_t depth;
    char stack[HTML_RENDERER_MAX_DEPTH][HTML_RENDERER_MAX_TAG_LEN + 1];
} HtmlRenderer;

typedef struct HtmlAttrib {
    const char *key;
    const char *value;
} HtmlAttrib;

// https://html.spec.whatwg.org/multipage/syntax.html#cdata-rcdata-restrictions
void html5_render_raw_text(HtmlRenderer *r, const char *string);
void html5_render_escaped(HtmlRenderer *r, const char *string);
void html5_render_elem_end(HtmlRenderer *r);
void html5_render_elem_begin(HtmlRenderer *r, const char *tag, size_t num_attribs,
                            const HtmlAttrib attribs[num_attribs]);
void html5_render_void_elem(HtmlRenderer *r, const char *tag, size_t num_attribs,
                            const HtmlAttrib attribs[num_attribs]);

#define HTML_VOID_ELEM(r, tag, ...)                                                                \
    do {                                                                                           \
        html5_render_void_elem(r, (tag), ARRAY_LEN(((const HtmlAttrib[]){__VA_ARGS__})),           \
                               (const HtmlAttrib[]){__VA_ARGS__});                                 \
    } while (0)

#define HTML_ELEM(r, tag, ...)                                                                     \
    for (int _html_elem_inner_loop_it_##__LINE__ =                                                            \
             (html5_render_elem_begin(r, (tag), ARRAY_LEN(((const HtmlAttrib[]){__VA_ARGS__})),    \
                                      (const HtmlAttrib[]){__VA_ARGS__}),                          \
              0);                                                                                  \
         !_html_elem_inner_loop_it_##__LINE__; _html_elem_inner_loop_it_##__LINE__ = 1, html5_render_elem_end(r))

#define DIV_IF(r, cond, ...) HTML_ELEM(r, (cond) ? "div" : NULL, __VA_ARGS__)
#define H1_IF(r, cond, ...) HTML_ELEM(r, (cond) ? "h1" : NULL, __VA_ARGS__)
#define P_IF(r, cond, ...) HTML_ELEM(r, (cond) ? "p" : NULL, __VA_ARGS__)
#define B_IF(r, cond, ...) HTML_ELEM(r, (cond) ? "b" : NULL, __VA_ARGS__)
#define HTML_IF(r, cond, ...) HTML_ELEM(r, (cond) ? "html" : NULL, __VA_ARGS__)
#define HEAD_IF(r, cond, ...) HTML_ELEM(r, (cond) ? "head" : NULL, __VA_ARGS__)
#define BODY_IF(r, cond, ...) HTML_ELEM(r, (cond) ? "body" : NULL, __VA_ARGS__)

#define DIV(r, ...) DIV_IF(r, true, __VA_ARGS__)
#define H1(r, ...) H1_IF(r, true, __VA_ARGS__)
#define P(r, ...) P_IF(r, true, __VA_ARGS__)
#define B(r, ...) B_IF(r, true, __VA_ARGS__)
#define HTML(r, ...) HTML_IF(r, true, __VA_ARGS__)
#define HEAD(r, ...) HEAD_IF(r, true, __VA_ARGS__)
#define BODY(r, ...) BODY_IF(r, true, __VA_ARGS__)

// https://html.spec.whatwg.org/multipage/syntax.html#cdata-rcdata-restrictions
#define SCRIPT(r, content, ...)                                                                    \
    HTML_ELEM(r, "script", __VA_ARGS__) do { html5_render_raw_text((r), (content)); }              \
    while (0)
#define STYLE(r, content, ...)                                                                     \
    HTML_ELEM(r, "style", __VA_ARGS__) do { html5_render_raw_text((r), (content)); }               \
    while (0)
#define TITLE(r, content, ...)                                                                     \
    HTML_ELEM(r, "title", __VA_ARGS__) do { html5_render_escaped((r), (content)); }                \
    while (0)
#define TEXTAREA(r, content, ...)                                                                  \
    HTML_ELEM(r, "textarea", __VA_ARGS__) do { html5_render_escaped((r), (content)); }             \
    while (0)

// https://developer.mozilla.org/en-US/docs/Glossary/Void_element
// https://html.spec.whatwg.org/#void-elements
#define AREA(r, ...) HTML_VOID_ELEM(r, "area", __VA_ARGS__)
#define BASE(r, ...) HTML_VOID_ELEM(r, "base", __VA_ARGS__)
#define BR(r, ...) HTML_VOID_ELEM(r, "br", __VA_ARGS__)
#define COL(r, ...) HTML_VOID_ELEM(r, "col", __VA_ARGS__)
#define EMBED(r, ...) HTML_VOID_ELEM(r, "embed", __VA_ARGS__)
#define HR(r, ...) HTML_VOID_ELEM(r, "hr", __VA_ARGS__)
#define IMG(r, ...) HTML_VOID_ELEM(r, "img", __VA_ARGS__)
#define INPUT(r, ...) HTML_VOID_ELEM(r, "input", __VA_ARGS__)
#define LINK(r, ...) HTML_VOID_ELEM(r, "link", __VA_ARGS__)
#define META(r, ...) HTML_VOID_ELEM(r, "meta", __VA_ARGS__)
#define SOURCE(r, ...) HTML_VOID_ELEM(r, "source", __VA_ARGS__)
#define TRACK(r, ...) HTML_VOID_ELEM(r, "track", __VA_ARGS__)
#define WBR(r, ...) HTML_VOID_ELEM(r, "wbr", __VA_ARGS__)

#define HTML5_RAW_STRING(r, s)                                                                     \
    do {                                                                                           \
        html5_render_escaped(r->fstream, s);                                                       \
    } while (0)

#if __cplusplus
}
#endif
