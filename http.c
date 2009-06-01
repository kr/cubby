// http.c -- http handlers

#include <event2/http.h>

#include "http.h"
#include "util.h"

void
http_handle_generic(struct evhttp_request *req, void *data)
{
    warnx("got a request");
}
