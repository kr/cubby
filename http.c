// http.c -- http handlers

#include "config.h"

#include <string.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/http_struct.h>

#include "http.h"
#include "blob.h"
#include "region.h"
#include "dirent.h"
#include "manager.h"
#include "sha512.h"
#include "util.h"

// For some reason, libevent defines some of these and not others.
#ifndef HTTP_FORBIDDEN
#define HTTP_FORBIDDEN 403
#endif
#ifndef HTTP_INTERNAL_ERROR
#define HTTP_INTERNAL_ERROR 500
#endif

extern const char root_html[];
extern const int root_html_size;

static void
http_handle_blob_get(struct evhttp_request *req, manager mgr)
{
    uint32_t k[3] = {};
    char *duri;
    char *name;
    dirent de;
    struct evkeyvalq *out_headers;
    struct evbuffer *out;
    int r;
    blob bl;

    duri = evhttp_decode_uri(req->uri);
    name = duri + 1; // skip the leading slash

    sha512(name, strlen(name), k, 12);
    raw_warnx("k = %8x.%8x.%8x", k[2], k[1], k[0]);
    free(duri);
    duri = name = 0;

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
}

static void
http_handle_blob_put(struct evhttp_request *req, manager mgr)
{
    struct evbuffer *in;
    size_t len;
    uint32_t k[3] = {};
    dirent de;
    region reg;
    char *duri;
    char *name;
    blob bl;
    int r;

    duri = evhttp_decode_uri(req->uri);
    name = duri + 1; // skip the leading slash

    raw_warnx("name (%d bytes) is %s", strlen(name), name);
    sha512(name, strlen(name), k, 12);
    raw_warnx("k = %8x.%8x.%8x", k[2], k[1], k[0]);
    free(duri);
    duri = name = 0;

    in = evhttp_request_get_input_buffer(req);
    len = evbuffer_get_length(in);
    raw_warnx("got %zu bytes", len);

    de = spht_get(mgr->directory, k);
    if (de) {
        evhttp_send_reply(req, HTTP_FORBIDDEN, "Already stored", 0);
        return;
    }

    reg = manager_pick_region(mgr, len);
    if (!reg) {
        evhttp_send_reply(req, HTTP_INTERNAL_ERROR, "No free region", 0);
        return;
    }

    bl = region_allocate_blob(reg, len);
    if (!bl) { // can't happen
        evhttp_send_reply(req, HTTP_INTERNAL_ERROR, "No space for blob", 0);
        return;
    }

    // We only allocate space for one rdesc because we probably won't be
    // responsible for this dirent. We're only concerned with blob storage
    // right now.
    de = make_dirent(k, 1);
    if (!de) {
        region_unallocate_blob(reg, bl); // put the space back
        evhttp_send_reply(req, HTTP_INTERNAL_ERROR, "Out of memory", 0);
        return;
    }

    dirent_set_rdesc_local(de, 0, reg, bl);

    spht_set(mgr->directory, de);

    /* Okay! Now actually copy the data in. Finally. */
    r = evbuffer_remove(in, bl->data, len);
    if (r == -1) {
        region_unallocate_blob(reg, bl); // put the space back
        spht_rm(mgr->directory, de->key);
        free(de);
        evhttp_send_reply(req, HTTP_INTERNAL_ERROR, "can't drain buffer", 0);
        return;
    }
    bl->key[0] = k[0];
    bl->key[1] = k[1];
    bl->key[2] = k[2];
    bl->size = len;
    bl->w = 3; // TODO pull this from the headers

    // TODO msync

    // Oh snap! We just got a file!
    evhttp_send_reply(req, HTTP_OK, "Got it!", 0);
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
}

static void
http_handle_admin(struct evhttp_request *req, manager mgr)
{
    evhttp_send_reply(req, HTTP_NOTFOUND, "Not found", 0);
}

static void
http_handle_blob(struct evhttp_request *req, manager mgr)
{
    if (req->type == EVHTTP_REQ_GET) return http_handle_blob_get(req, mgr);
    if (req->type == EVHTTP_REQ_PUT) return http_handle_blob_put(req, mgr);

    // TODO add allow header
    evhttp_send_reply(req, 405, "Method not allowed", 0);
}

void
http_handle_generic(struct evhttp_request *req, void *mgr)
{
    if (startswith(req->uri, "/$/")) {
        http_handle_admin(req, mgr);
    } else {
        http_handle_blob(req, mgr);
    }
}
