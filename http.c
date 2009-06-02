// http.c -- http handlers

#include "config.h"

#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/http_struct.h>

#include "http.h"
#include "util.h"

#define ROOT_RESPONSE ("This is " PACKAGE_STRING ".")
#define ROOT_RESPONSE_LEN (sizeof(ROOT_RESPONSE) - 1)

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
    evbuffer_add_reference(out, ROOT_RESPONSE, ROOT_RESPONSE_LEN, 0, 0);
    evhttp_send_reply(req, HTTP_OK, "OK", out);
}
