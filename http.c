// http.c -- http handlers

#include "config.h"

#include <string.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/http_struct.h>

#include "http.h"
#include "dirent.h"
#include "spht.h"
#include "util.h"

// For some reason, libevent defines some of these and not others.
#ifndef HTTP_FORBIDDEN
#define HTTP_FORBIDDEN 403
#endif

extern const char root_html[];
extern const int root_html_size;

static void
http_handle_blob_get(struct evhttp_request *req, spht dir)
{
    evhttp_send_reply(req, HTTP_NOTFOUND, "Not found", 0);
}

static void
http_handle_blob_put(struct evhttp_request *req, spht dir)
{
    struct evbuffer *in;
    size_t len;
    uint32_t k[3] = {0, 0, 0};
    dirent de;

    in = evhttp_request_get_input_buffer(req);
    len = evbuffer_get_length(in);
    raw_warnx("got %zu bytes", len);

    de = spht_get(dir, k);
    if (de) {
        evhttp_send_reply(req, HTTP_FORBIDDEN, "Already stored", 0);
        return;
    }

    //bl = storage_allocate_blob(len);
    //evbuffer_remove(in, bl, len);
    evhttp_send_reply(req, HTTP_NOTFOUND, "Not found", 0);
}

void
http_handle_root(struct evhttp_request *req, void *dir)
{
    struct evbuffer *out;

    if (req->type != EVHTTP_REQ_GET) {
        // TODO add allow header
        evhttp_send_reply(req, 405, "Method not allowed", 0);
    }

    out = evbuffer_new();
    evbuffer_add_reference(out, root_html, root_html_size, 0, 0);
    evhttp_send_reply(req, HTTP_OK, "OK", out);
}

static void
http_handle_admin(struct evhttp_request *req, spht dir)
{
    evhttp_send_reply(req, HTTP_NOTFOUND, "Not found", 0);
}

static void
http_handle_blob(struct evhttp_request *req, spht dir)
{
    if (req->type == EVHTTP_REQ_GET) return http_handle_blob_get(req, dir);
    if (req->type == EVHTTP_REQ_PUT) return http_handle_blob_put(req, dir);

    // TODO add allow header
    evhttp_send_reply(req, 405, "Method not allowed", 0);
}

void
http_handle_generic(struct evhttp_request *req, void *dir)
{
    if (startswith(req->uri, "/$/")) {
        http_handle_admin(req, dir);
    } else {
        http_handle_blob(req, dir);
    }
}
