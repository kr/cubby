// http.h -- http handlers header

#ifndef http_h
#define http_h

#include <evhttp.h>

void http_handle_generic(struct evhttp_request *req, void *data);

#endif //http_h
