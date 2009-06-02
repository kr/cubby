// http.c -- http handlers

#include "config.h"

#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/http_struct.h>

#include "http.h"
#include "util.h"

extern const char root_html[];
extern const int root_html_size;

void
http_handle_generic(struct evhttp_request *req, void *data)
{
    evhttp_send_reply(req, HTTP_NOTFOUND, "Not found", 0);
}

void
http_handle_root(struct evhttp_request *req, void *data)
{
    struct evbuffer *out;

    out = evbuffer_new();
    evbuffer_add_reference(out, root_html, root_html_size, 0, 0);
    evhttp_send_reply(req, HTTP_OK, "OK", out);
}
