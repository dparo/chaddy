// SPDX-FileCopyrightText: 2024 Davide Paro
//
// SPDX-License-Identifier: MIT

#include "html5.h"
#include <stdint.h>
#include <string.h>

void html5_render_raw_text(HtmlRenderer *const r, const char *string) {
    if (string && *string != '\0')
        fprintf(r->fstream, "%s", string);
}

void html5_render_escaped(HtmlRenderer *const r, const char *string) {
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
            fprintf(r->fstream, "%.*s", (int)(p - s), s);
            fprintf(r->fstream, "%s", escaped_chars[(int)*p]);
            s = p + 1;
        }
    }

    fprintf(r->fstream, "%s", s);
}

static void render_attrs(HtmlRenderer *const r, size_t num_attribs, const HtmlAttrib attribs[num_attribs]) {
    if (num_attribs > 0)
        fprintf(r->fstream, " ");
    for (size_t i = 0; i < num_attribs /* attribs[i].key */; i++) {
        const char *key = attribs[i].key;
        const char *value = attribs[i].value;

        if (key && value) {
            html5_render_escaped(r, key);
            fprintf(r->fstream, "=\"");
            html5_render_escaped(r, value);
            fprintf(r->fstream, "\" ");
        } else if (key) {
            // NOTE(d.paro): HTML allows to have keys with no associated values, i.e: <option
            // value="foo" selected />
            html5_render_escaped(r, key);
            fprintf(r->fstream, " ");
        }
    }
}

// https://developer.mozilla.org/en-US/docs/Glossary/Void_element
// https://html.spec.whatwg.org/#void-elements
void html5_render_void_elem(HtmlRenderer *const r, const char *tag, size_t num_attribs,
                            const HtmlAttrib attribs[num_attribs]) {
    tag = tag ? tag : "";
    if (r->depth >= HTML_RENDERER_MAX_DEPTH || strlen(tag) >= HTML_RENDERER_MAX_TAG_LEN) {
        return;
    }

    if (tag != NULL && *tag != '\0') {
        fprintf(r->fstream, "<");
        html5_render_escaped(r, tag);
        render_attrs(r, num_attribs, attribs);
        fprintf(r->fstream, "/>");
    }
    return;
}

int html5_render_elem_begin(HtmlRenderer *const r, const char *tag, size_t num_attribs,
                            const HtmlAttrib attribs[num_attribs]) {
    tag = tag ? tag : "";
    if (r->depth >= HTML_RENDERER_MAX_DEPTH || strlen(tag) >= HTML_RENDERER_MAX_TAG_LEN) {
        return 0;
    }

    strncpy(r->stack[r->depth], tag, HTML_RENDERER_MAX_TAG_LEN + 1);
    ++r->depth;

    if (tag != NULL && *tag != '\0') {
        fprintf(r->fstream, "<");
        html5_render_escaped(r, tag);
        render_attrs(r, num_attribs, attribs);
        fprintf(r->fstream, ">");
    }

    return 0;
}

void html5_render_elem_end(HtmlRenderer *const r) {
    if (r->depth <= 0) {
        return;
    }

    --r->depth;

    const char *tag = r->stack[r->depth];

    if (tag != NULL && *tag != '\0') {
        fprintf(r->fstream, "</");
        html5_render_escaped(r, tag);
        fprintf(r->fstream, ">");
    }
}
