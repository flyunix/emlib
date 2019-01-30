#include "emlib.h"

const static char* module = "TEST_UTILS";

void app_perror(const char *msg, emlib_ret_t rc)
{
    char errbuf[EM_ERR_MSG_SIZE];

    EM_CHECK_STACK();

    em_strerror(rc, errbuf, sizeof(errbuf));
    EM_LOG(EM_LOG_INFO, "%s: [emlib_ret_t=%d] %s", msg, rc, errbuf);
}


#define SERVER 0
#define CLIENT 1

emlib_ret_t app_socket(int family, int type, int proto, int port,
        em_sock_t *ptr_sock)
{
    em_sockaddr_in addr;
    em_sock_t sock;
    emlib_ret_t rc;

    rc = em_sock_socket(family, type, proto, &sock);
    if (rc != EM_SUCC)
        return rc;

    em_bzero(&addr, sizeof(addr));
    addr.sin_family = (em_uint16_t)family;
    addr.sin_port = (short)(port!=-1 ? em_htons((em_uint16_t)port) : 0);
    rc = em_sock_bind(sock, &addr, sizeof(addr));
    if (rc != EM_SUCC)
        return rc;

#if EM_HAS_TCP
    if (type == em_SOCK_STREAM()) {
        rc = em_sock_listen(sock, 5);
        if (rc != EM_SUCC)
            return rc;
    }
#endif

    *ptr_sock = sock;
    return EM_SUCC;
}

emlib_ret_t app_socketpair(int family, int type, int protocol,
        em_sock_t *serverfd, em_sock_t *clientfd)
{
    int i;
    static unsigned short port = 11000;
    em_sockaddr_in addr;
    em_str_t s;
    emlib_ret_t rc = 0;
    em_sock_t sock[2];

    /* Create both sockets. */
    for (i=0; i<2; ++i) {
        rc = em_sock_socket(family, type, protocol, &sock[i]);
        if (rc != EM_SUCC) {
            if (i==1)
                em_sock_close(sock[0]);
            return rc;
        }
    }

    /* Retry bind */
    em_bzero(&addr, sizeof(addr));
    addr.sin_family = em_AF_INET();
    for (i=0; i<5; ++i) {
        addr.sin_port = em_htons(port++);
        rc = em_sock_bind(sock[SERVER], &addr, sizeof(addr));
        if (rc == EM_SUCC)
            break;
    }

    if (rc != EM_SUCC)
        goto on_error;

    /* For TCP, listen the socket. */
#if EM_HAS_TCP
    if (type == em_SOCK_STREAM()) {
        rc = em_sock_listen(sock[SERVER], EM_SOMAXCONN);
        if (rc != EM_SUCC)
            goto on_error;
    }
#endif

    /* Connect client socket. */
    addr.sin_addr = em_inet_addr(em_cstr(&s, "127.0.0.1"));
    rc = em_sock_connect(sock[CLIENT], &addr, sizeof(addr));
    if (rc != EM_SUCC)
        goto on_error;

    /* For TCP, must accept(), and get the new socket. */
#if EM_HAS_TCP
    if (type == em_SOCK_STREAM()) {
        em_sock_t newserver;

        rc = em_sock_accept(sock[SERVER], &newserver, NULL, NULL);
        if (rc != EM_SUCC)
            goto on_error;

        /* Replace server socket with new socket. */
        em_sock_close(sock[SERVER]);
        sock[SERVER] = newserver;
    }
#endif

    *serverfd = sock[SERVER];
    *clientfd = sock[CLIENT];

    return rc;

on_error:
    for (i=0; i<2; ++i)
        em_sock_close(sock[i]);
    return rc;
}

