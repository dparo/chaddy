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
    char *key;
    char *value;
} HtmlAttrib;

void html5_render_escaped(FILE *fstream, const char *string);
int html5_render_begin(HtmlRenderer *const r, const char *tag, size_t num_attribs,
                       const HtmlAttrib *attribs);
void html5_render_end(HtmlRenderer *const r);

#define HTML_ELEM(r, tag, ...)                                                                     \
    for (int _block_inner_cnt = 0,                                                                 \
             _ = html5_render_begin(r, tag, ARRAY_LEN(((HtmlAttrib[]){__VA_ARGS__})),              \
                                    (HtmlAttrib[]){__VA_ARGS__, {NULL, NULL}});                    \
         !_block_inner_cnt; _block_inner_cnt = 1, html5_render_end(r))

#define DIV(r, ...) HTML_ELEM(r, "div", __VA_ARGS__)

#define HTML5_RAW_STRING(r, s)                                                                     \
    do {                                                                                           \
        html5_render_escaped(r->fstream, s)                                                        \
    } while (0)

#if __cplusplus
}
#endif
