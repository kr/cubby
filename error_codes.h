// error_codes.h -- Cubby's error codes

#ifndef error_codes_h
#define error_codes_h

typedef enum error_codes {
    error_code_success,
    error_code_no_mem,
    error_code_insufficient_nodes,
    error_code_rank_mismatch,
    error_code_add_links,
    error_code_missing_dirent,
    error_code_no_reply,
} error_code;

#endif //error_codes_h
