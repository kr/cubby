// udp.h -- udp handlers header

#ifndef udp_h
#define udp_h

typedef struct cpkt *cpkt;

struct cpkt {
    uint8_t type;
    char data[];
};

void udp_recv(int fd, short which, void *mgr);

#endif //udp_h
