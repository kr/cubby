// http.h -- http handlers header

#ifndef http_h
#define http_h

#include <event2/http.h>

#include "spht.h"

void http_handle_generic(struct evhttp_request *req, void *dir);

void http_handle_root(struct evhttp_request *req, void *dir);
void http_handle_about_json(struct evhttp_request *req, void *mgr);

#endif //http_h
