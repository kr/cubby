// http.c -- http handlers

#include "config.h"

#include <string.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/http_struct.h>

#include "http.h"
#include "blob.h"
#include "region.h"
#include "prot.h"
#include "dirent.h"
#include "manager.h"
#include "sha512.h"
#include "key.h"
#include "util.h"

// For some reason, libevent defines some of these and not others.
#ifndef HTTP_FORBIDDEN
#define HTTP_FORBIDDEN 403
#endif
#ifndef HTTP_INTERNAL_ERROR
#define HTTP_INTERNAL_ERROR 500
#endif

#define DEBUG_JSON "{\n" \
  "\"version\": \"%s\"\n" \
"}\n"

extern const char root_html[];
extern const int root_html_size;

static void
http_handle_blob_get(struct evhttp_request *req, manager mgr, uint32_t *k)
{
    dirent de;
    struct evkeyvalq *out_headers;
    struct evbuffer *out;
    int r;
    blob bl;

    de = spht_get(mgr->directory, k);
    if (!de) {
        evhttp_send_reply(req, HTTP_NOTFOUND, "Not found", 0);
        return;
    }

    out_headers = evhttp_request_get_output_headers(req);
    if (!out_headers) {
        evhttp_send_reply(req, HTTP_INTERNAL_ERROR, "can't get headers", 0);
        return;
    }

    r = evhttp_add_header(out_headers, "Content-Type", "image/jpeg");
    if (r == -1) {
        evhttp_send_reply(req, HTTP_INTERNAL_ERROR, "can't add header", 0);
        return;
    }

    bl = manager_get_blob(mgr, de);

    out = evbuffer_new();
    if (!out) {
        evhttp_send_reply(req, HTTP_INTERNAL_ERROR, "can't make buffer", 0);
        return;
    }

    evbuffer_add_reference(out, &bl->data, bl->size, 0, 0);
    evhttp_send_reply(req, HTTP_OK, "OK", out);
    evbuffer_free(out);
}

static void
http_put_done(manager m, uint32_t *key, int error, void *req)
{
    if (error == 0) {
        // Oh snap! We just got a file!
        evhttp_send_reply(req, HTTP_OK, "Got it!", 0);
    } else {
        evhttp_send_reply(req, HTTP_INTERNAL_ERROR, "could not send links", 0);
    }
}

static void
http_handle_blob_put(struct evhttp_request *req, manager mgr, uint32_t *k)
{
    struct evbuffer *in;
    size_t len;
    dirent de;
    blob bl;
    int r;

    in = evhttp_request_get_input_buffer(req);
    len = evbuffer_get_length(in);
    raw_warnx("got %zu bytes", len);

    de = spht_get(mgr->directory, k);
    if (de) {
        evhttp_send_reply(req, HTTP_FORBIDDEN, "Already stored", 0);
        return;
    }

    // We only allocate space for one rdesc because we probably won't be
    // responsible for this dirent. We're only concerned with blob storage
    // right now.
    de = make_dirent(k, 1, 3);
    if (!de) {
        evhttp_send_reply(req, HTTP_INTERNAL_ERROR, "Out of memory", 0);
        return;
    }

    bl = manager_allocate_blob(mgr, de, len);
    if (!bl) {
        free(de);
        evhttp_send_reply(req, HTTP_INTERNAL_ERROR, "No space for blob", 0);
        return;
    }

    spht_set(mgr->directory, de);

    /* Okay! Now actually copy the data in. Finally. */
    r = evbuffer_remove(in, bl->data, len);
    if (r < len) {
        manager_delete_blob(mgr, de); // put the space back
        spht_rm(mgr->directory, de->key);
        free(de);
        evhttp_send_reply(req, HTTP_INTERNAL_ERROR, "can't drain buffer", 0);
        return;
    }
    bl->key[0] = k[0];
    bl->key[1] = k[1];
    bl->key[2] = k[2];
    bl->size = len;
    de->num_copies = bl->num_copies = 3; // TODO extract this from HTTP headers

    // TODO msync

    prot_send_primary_link(mgr, de, http_put_done, req);
}

void
http_handle_root(struct evhttp_request *req, void *mgr)
{
    struct evbuffer *out;

    if (req->type != EVHTTP_REQ_GET) {
        // TODO add allow header
        evhttp_send_reply(req, 405, "Method not allowed", 0);
    }

    out = evbuffer_new();
    evbuffer_add_reference(out, root_html, root_html_size, 0, 0);
    evhttp_send_reply(req, HTTP_OK, "OK", out);
    evbuffer_free(out);
}

void
http_handle_about_json(struct evhttp_request *req, void *mgr)
{
    struct evbuffer *out;

    if (req->type != EVHTTP_REQ_GET) {
        // TODO add allow header
        evhttp_send_reply(req, 405, "Method not allowed", 0);
    }

    out = evbuffer_new();
    evbuffer_add_printf(out, DEBUG_JSON, VERSION);
    evhttp_send_reply(req, HTTP_OK, "OK", out);
    evbuffer_free(out);
}

static void
http_handle_admin(struct evhttp_request *req, manager mgr)
{
    evhttp_send_reply(req, HTTP_NOTFOUND, "Not found", 0);
}

static void
key_from_req(struct evhttp_request *req, uint32_t *k)
{
    char *decoded_uri;
    char *name;

    decoded_uri = evhttp_decode_uri(req->uri);
    name = decoded_uri + 6; // Skip the leading "/file/"
    sha512(name, strlen(name), k, 12);
    free(decoded_uri);
}

static void
http_handle_blob(struct evhttp_request *req, manager mgr)
{
    uint32_t k[3] = {};

    key_from_req(req, k);

    switch (req->type) {
        case EVHTTP_REQ_GET:
            raw_warnx("get");
            http_handle_blob_get(req, mgr, k);
            break;

        case EVHTTP_REQ_PUT:
            raw_warnx("put");
            http_handle_blob_put(req, mgr, k);
            break;

        default:
            raw_warnx("other");
            // TODO add allow header
            evhttp_send_reply(req, 405, "Method not allowed", 0);
    }
}

void
http_handle_generic(struct evhttp_request *req, void *mgr)
{
    if (startswith(req->uri, "/admin/")) {
        http_handle_admin(req, mgr);
    } else if (startswith(req->uri, "/file/")) {
        http_handle_blob(req, mgr);
    } else {
        evhttp_send_reply(req, HTTP_NOTFOUND, "Not found", 0);
    }
}
