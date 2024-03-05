// SPDX-FileCopyrightText: 2024 Davide Paro
//
// SPDX-License-Identifier: MIT

#include "html5.h"
#include <stdint.h>
#include <string.h>

void html5_render_escaped(FILE *fstream, const char *string) {
    if (string == NULL) {
        return;
    }

    const char *escaped_chars[UINT8_MAX] = {
        ['&'] = "&amp;", ['"'] = "&quot;", ['\''] = "&#x27;", ['<'] = "&lt;", ['>'] = "&gt;",
    };

    const char *s = string;
    while (*s) {
        char *p = strpbrk(s, "&\"'<>");
        if (p == NULL) {
            break;
        } else {
            fprintf(fstream, "%.*s", (int)(p - s), s);
            fprintf(fstream, "%s", escaped_chars[(int)*p]);
            s = p + 1;
        }
    }

    fprintf(fstream, "%s", s);
}


static void render_attrs(HtmlRenderer *const r, size_t num_attribs,
                         const HtmlAttrib attribs[]) {
    for (size_t i = 0; i < num_attribs /* attribs[i].key */; i++) {
        const char *key = attribs[i].key;
        const char *value = attribs[i].value;

        if (key && value) {
            html5_render_escaped(r->fstream, key);
            fprintf(r->fstream, "=\"");
            html5_render_escaped(r->fstream, value);
            fprintf(r->fstream, "\" ");
        } else if (key) {
            // NOTE(d.paro): HTML allows to have keys with no associated values, i.e: <option
            // value="foo" selected />
            html5_render_escaped(r->fstream, key);
        }
    }
}

void html5_render_self_closing(HtmlRenderer *const r, const char *tag,
                              size_t num_attribs,
                              const HtmlAttrib attribs[]) {
    tag = tag ? tag : "";
    if (r->depth >= HTML_RENDERER_MAX_DEPTH || strlen(tag) >= HTML_RENDERER_MAX_TAG_LEN) {
        return;
    }

    if (tag != NULL && *tag != '\0') {
        fprintf(r->fstream, "<");
        html5_render_escaped(r->fstream, tag);
        fprintf(r->fstream, " ");
        render_attrs(r, num_attribs, attribs);
        fprintf(r->fstream, ">");
    }
    return;
}

int html5_render_begin(HtmlRenderer *const r, const char *tag, size_t num_attribs,
                       const HtmlAttrib attribs[]) {
    tag = tag ? tag : "";
    if (r->depth >= HTML_RENDERER_MAX_DEPTH || strlen(tag) >= HTML_RENDERER_MAX_TAG_LEN) {
        return 0;
    }

    strncpy(r->stack[r->depth], tag, HTML_RENDERER_MAX_TAG_LEN + 1);
    ++r->depth;

    if (tag != NULL && *tag != '\0') {
        fprintf(r->fstream, "<");
        html5_render_escaped(r->fstream, tag);
        fprintf(r->fstream, " ");
        render_attrs(r, num_attribs, attribs);
        fprintf(r->fstream, "/>");
    }

    return 0;
}

void html5_render_end(HtmlRenderer *const r) {
    if (r->depth <= 0) {
        return;
    }

    --r->depth;

    const char *tag = r->stack[r->depth];

    if (tag != NULL && *tag != '\0') {
        fprintf(r->fstream, "</");
        html5_render_escaped(r->fstream, tag);
        fprintf(r->fstream, ">");
    }
}
