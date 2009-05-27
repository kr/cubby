// store.h -- Storage Header

#ifndef store_h
#define store_h

typedef struct blob {
    /*
    uint32_t len;
    uint8_t id[12];
    uint8_t min_copies;
    uint8_t reserved[12];
    uint8_t data[];
    */
} *blob;

void store_init();

#endif //store_h
