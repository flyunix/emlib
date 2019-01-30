/*
 * File:    sock.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   TEST socket API.
 *
 * Copyright (c) Liu HongLiang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*
 * History:
 * ================================================================
 * 2019-01-30 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include "emlib.h"
#include "test.h"


/**
 * \page page_emlib_sock_test Test: Socket
 *
 * This file provides implementation of \b sock_test(). It tests the
 * various aspects of the socket API.
 *
 * \section sock_test_scope_sec Scope of the Test
 *
 * The scope of the test:
 *  - verify the validity of the address structs.
 *  - verify that address manipulation API works.
 *  - simple socket creation and destruction.
 *  - simple socket send/recv and sendto/recvfrom.
 *  - UDP connect()
 *  - send/recv big data.
 *  - all for both UDP and TCP.
 *
 * The APIs tested in this test:
 *  - em_inet_aton()
 *  - em_inet_ntoa()
 *  - em_inet_pton()  (only if IPv6 is enabled)
 *  - em_inet_ntop()  (only if IPv6 is enabled)
 *  - em_gethostname()
 *  - em_sock_socket()
 *  - em_sock_close()
 *  - em_sock_send()
 *  - em_sock_sendto()
 *  - em_sock_recv()
 *  - em_sock_recvfrom()
 *  - em_sock_bind()
 *  - em_sock_connect()
 *  - em_sock_listen()
 *  - em_sock_accept()
 *  - em_gethostbyname()
 *
 *
 * This file is <b>emlib-test/sock.c</b>
 *
 * \include emlib-test/sock.c
 */

const static char* module = "SOCK_TEST";

#if INCLUDE_SOCK_TEST

#define UDP_PORT	51234
#define TCP_PORT        (UDP_PORT+10)
#define BIG_DATA_LEN	8192
#define ADDRESS		"127.0.0.1"

static char bigdata[BIG_DATA_LEN];
static char bigbuffer[BIG_DATA_LEN];

/* Macro for checking the value of "sin_len" member of sockaddr
 * (it must always be zero).
 */
#if defined(EM_SOCKADDR_HAS_LEN) && EM_SOCKADDR_HAS_LEN!=0
#   define CHECK_SA_ZERO_LEN(addr, ret) \
    if (((em_addr_hdr*)(addr))->sa_zero_len != 0) \
return ret
#else
#   define CHECK_SA_ZERO_LEN(addr, ret)
#endif


static int format_test(void)
{
    em_str_t s = em_str(ADDRESS);
    unsigned char *p;
    em_in_addr addr;
    char zero[64];
    em_sockaddr_in addr2;
    const em_str_t *hostname;
    const unsigned char A[] = {127, 0, 0, 1};

    EM_LOG(3, "...format_test()");

    /* em_inet_aton() */
    if (em_inet_aton(&s, &addr) != 1)
        return -10;

    /* Check the result. */
    p = (unsigned char*)&addr;
    if (p[0]!=A[0] || p[1]!=A[1] || p[2]!=A[2] || p[3]!=A[3]) {
        EM_LOG(3, "  error: mismatched address. p0=%d, p1=%d, "
                    "p2=%d, p3=%d", p[0] & 0xFF, p[1] & 0xFF, 
                    p[2] & 0xFF, p[3] & 0xFF);
        return -15;
    }

    /* em_inet_ntoa() */
    p = (unsigned char*) em_inet_ntoa(addr);
    if (!p)
        return -20;

    if (em_strcmp2(&s, (char*)p) != 0)
        return -22;

#if defined(EM_HAS_IPV6) && EM_HAS_IPV6!=0
    /* em_inet_pton() */
    /* em_inet_ntop() */
    {
        const em_str_t s_ipv4 = em_str("127.0.0.1");
        const em_str_t s_ipv6 = em_str("fe80::2ff:83ff:fe7c:8b42");
        char buf_ipv4[EM_INET_ADDRSTRLEN];
        char buf_ipv6[EM_INET6_ADDRSTRLEN];
        em_in_addr ipv4;
        em_in6_addr ipv6;

        if (em_inet_pton(em_AF_INET(), &s_ipv4, &ipv4) != EM_SUCC)
            return -24;

        p = (unsigned char*)&ipv4;
        if (p[0]!=A[0] || p[1]!=A[1] || p[2]!=A[2] || p[3]!=A[3]) {
            return -25;
        }

        if (em_inet_pton(em_AF_INET6(), &s_ipv6, &ipv6) != EM_SUCC)
            return -26;

        p = (unsigned char*)&ipv6;
        if (p[0] != 0xfe || p[1] != 0x80 || p[2] != 0 || p[3] != 0 ||
                p[4] != 0 || p[5] != 0 || p[6] != 0 || p[7] != 0 ||
                p[8] != 0x02 || p[9] != 0xff || p[10] != 0x83 || p[11] != 0xff ||
                p[12]!=0xfe || p[13]!=0x7c || p[14] != 0x8b || p[15]!=0x42)
        {
            return -27;
        }

        if (em_inet_ntop(em_AF_INET(), &ipv4, buf_ipv4, sizeof(buf_ipv4)) != EM_SUCC)
            return -28;
        if (em_stricmp2(&s_ipv4, buf_ipv4) != 0)
            return -29;

        if (em_inet_ntop(em_AF_INET6(), &ipv6, buf_ipv6, sizeof(buf_ipv6)) != EM_SUCC)
            return -30;
        if (em_stricmp2(&s_ipv6, buf_ipv6) != 0)
            return -31;
    }

#endif	/* EM_HAS_IPV6 */

    /* Test that em_sockaddr_in_init() initialize the whole structure, 
     * including sin_zero.
     */
    em_sockaddr_in_init(&addr2, 0, 1000);
    em_bzero(zero, sizeof(zero));
    if (em_memcmp(addr2.sin_zero, zero, sizeof(addr2.sin_zero)) != 0)
        return -35;

    /* em_gethostname() */
    hostname = em_gethostname();
    if (!hostname || !hostname->ptr || !hostname->slen)
        return -40;

    EM_LOG(3, "....hostname is %.*s", 
                (int)hostname->slen, hostname->ptr);

    /* em_gethostaddr() */

    /* Various constants */
#if !defined(EM_SYMBIAN) || EM_SYMBIAN==0
    if (EM_AF_INET==0xFFFF) return -5500;
    if (EM_AF_INET6==0xFFFF) return -5501;

    /* 0xFFFF could be a valid SOL_SOCKET (e.g: on some Win or Mac) */
    //if (EM_SOL_SOCKET==0xFFFF) return -5503;

    if (EM_SOL_IP==0xFFFF) return -5502;
    if (EM_SOL_TCP==0xFFFF) return -5510;
    if (EM_SOL_UDP==0xFFFF) return -5520;
    if (EM_SOL_IPV6==0xFFFF) return -5530;

    if (EM_SO_TYPE==0xFFFF) return -5540;
    if (EM_SO_RCVBUF==0xFFFF) return -5550;
    if (EM_SO_SNDBUF==0xFFFF) return -5560;
    if (EM_TCP_NODELAY==0xFFFF) return -5570;
    if (EM_SO_REUSEADDR==0xFFFF) return -5580;

    if (EM_MSG_OOB==0xFFFF) return -5590;
    if (EM_MSG_PEEK==0xFFFF) return -5600;
#endif

    return 0;
}

static int parse_test(void)
{
#define IPv4	1
#define IPv6	2

    struct test_t {
        const char  *input;
        int	     result_af;
        const char  *result_ip;
        em_uint16_t  result_port;
    };
    struct test_t valid_tests[] = 
    {
        /* IPv4 */
        { "10.0.0.1:80", IPv4, "10.0.0.1", 80},
        { "10.0.0.1", IPv4, "10.0.0.1", 0},
        { "10.0.0.1:", IPv4, "10.0.0.1", 0},
        { "10.0.0.1:0", IPv4, "10.0.0.1", 0},
        { ":80", IPv4, "0.0.0.0", 80},
        { ":", IPv4, "0.0.0.0", 0},
#if !EM_SYMBIAN
        { "localhost", IPv4, "127.0.0.1", 0},
        { "localhost:", IPv4, "127.0.0.1", 0},
        { "localhost:80", IPv4, "127.0.0.1", 80},
#endif

#if defined(EM_HAS_IPV6) && EM_HAS_IPV6
        { "fe::01:80", IPv6, "fe::01:80", 0},
        { "[fe::01]:80", IPv6, "fe::01", 80},
        { "fe::01", IPv6, "fe::01", 0},
        { "[fe::01]", IPv6, "fe::01", 0},
        { "fe::01:", IPv6, "fe::01", 0},
        { "[fe::01]:", IPv6, "fe::01", 0},
        { "::", IPv6, "::0", 0},
        { "[::]", IPv6, "::", 0},
        { ":::", IPv6, "::", 0},
        { "[::]:", IPv6, "::", 0},
        { ":::80", IPv6, "::", 80},
        { "[::]:80", IPv6, "::", 80},
#endif
    };
    struct test_t invalid_tests[] = 
    {
        /* IPv4 */
        { "10.0.0.1:abcd", IPv4},   /* port not numeric */
        { "10.0.0.1:-1", IPv4},	    /* port contains illegal character */
        { "10.0.0.1:123456", IPv4}, /* port too big	*/
        //this actually is fine on my Mac OS 10.9
        //it will be resolved with gethostbyname() and something is returned!
        //{ "1.2.3.4.5:80", IPv4},    /* invalid IP */
        { "10:0:80", IPv4},	    /* hostname has colon */

#if defined(EM_HAS_IPV6) && EM_HAS_IPV6
        { "[fe::01]:abcd", IPv6},   /* port not numeric */
        { "[fe::01]:-1", IPv6},	    /* port contains illegal character */
        { "[fe::01]:123456", IPv6}, /* port too big	*/
        { "fe::01:02::03:04:80", IPv6},	    /* invalid IP */
        { "[fe::01:02::03:04]:80", IPv6},   /* invalid IP */
        { "[fe:01", IPv6},	    /* Unterminated bracket */
#endif
    };

    unsigned i;

    EM_LOG(3, "...IP address parsing");

    for (i=0; i<EM_ARRAY_SIZE(valid_tests); ++i) {
        emlib_ret_t status;
        em_str_t input;
        em_sockaddr addr, result;

        switch (valid_tests[i].result_af) {
            case IPv4:
                valid_tests[i].result_af = EM_AF_INET;
                break;
            case IPv6:
                valid_tests[i].result_af = EM_AF_INET6;
                break;
            default:
                EMLIB_ASSERT(!"Invalid AF!");
                continue;
        }

        /* Try parsing with EM_AF_UNSPEC */
        status = em_sockaddr_parse(EM_AF_UNSPEC, 0, 
                em_cstr(&input, valid_tests[i].input), 
                &addr);
        if (status != EM_SUCC) {
            EM_LOG(1, ".... failed when parsing %s (i=%d)", 
                        valid_tests[i].input, i);
            return -10;
        }

        /* Check "sin_len" member of parse result */
        CHECK_SA_ZERO_LEN(&addr, -20);

        /* Build the correct result */
        status = em_sockaddr_init(valid_tests[i].result_af,
                &result,
                em_cstr(&input, valid_tests[i].result_ip), 
                valid_tests[i].result_port);
        if (status != EM_SUCC) {
            EM_LOG(1, ".... error building IP address %s", 
                        valid_tests[i].input);
            return -30;
        }

        /* Compare the result */
        if (em_sockaddr_cmp(&addr, &result) != 0) {
            EM_LOG(1, ".... parsed result mismatched for %s", 
                        valid_tests[i].input);
            return -40;
        }

        /* Parse again with the specified af */
        status = em_sockaddr_parse(valid_tests[i].result_af, 0, 
                em_cstr(&input, valid_tests[i].input), 
                &addr);
        if (status != EM_SUCC) {
            EM_LOG(1,".... failed when parsing %s", 
                        valid_tests[i].input);
            return -50;
        }

        /* Check "sin_len" member of parse result */
        CHECK_SA_ZERO_LEN(&addr, -55);

        /* Compare the result again */
        if (em_sockaddr_cmp(&addr, &result) != 0) {
            EM_LOG(1,".... parsed result mismatched for %s", 
                        valid_tests[i].input);
            return -60;
        }
    }

    for (i=0; i<EM_ARRAY_SIZE(invalid_tests); ++i) {
        emlib_ret_t status;
        em_str_t input;
        em_sockaddr addr;

        switch (invalid_tests[i].result_af) {
            case IPv4:
                invalid_tests[i].result_af = EM_AF_INET;
                break;
            case IPv6:
                invalid_tests[i].result_af = EM_AF_INET6;
                break;
            default:
                EMLIB_ASSERT(!"Invalid AF!");
                continue;
        }

        /* Try parsing with EM_AF_UNSPEC */
        status = em_sockaddr_parse(EM_AF_UNSPEC, 0, 
                em_cstr(&input, invalid_tests[i].input), 
                &addr);
        if (status == EM_SUCC) {
            EM_LOG(1,".... expecting failure when parsing %s", 
                        invalid_tests[i].input);
            return -100;
        }
    }

    return 0;
}

static int purity_test(void)
{
    EM_LOG(3, "...purity_test()");

#if defined(EM_SOCKADDR_HAS_LEN) && EM_SOCKADDR_HAS_LEN!=0
    /* Check on "sin_len" member of sockaddr */
    {
        const em_str_t str_ip = {"1.1.1.1", 7};
        em_sockaddr addr[16];
        em_addrinfo ai[16];
        unsigned cnt;
        emlib_ret_t rc;

        /* em_enum_ip_interface() */
        cnt = EM_ARRAY_SIZE(addr);
        rc = em_enum_ip_interface(em_AF_UNSPEC(), &cnt, addr);
        if (rc == EM_SUCC) {
            while (cnt--)
                CHECK_SA_ZERO_LEN(&addr[cnt], -10);
        }

        /* em_gethostip() on IPv4 */
        rc = em_gethostip(em_AF_INET(), &addr[0]);
        if (rc == EM_SUCC)
            CHECK_SA_ZERO_LEN(&addr[0], -20);

        /* em_gethostip() on IPv6 */
        rc = em_gethostip(em_AF_INET6(), &addr[0]);
        if (rc == EM_SUCC)
            CHECK_SA_ZERO_LEN(&addr[0], -30);

        /* em_getdefaultipinterface() on IPv4 */
        rc = em_getdefaultipinterface(em_AF_INET(), &addr[0]);
        if (rc == EM_SUCC)
            CHECK_SA_ZERO_LEN(&addr[0], -40);

        /* em_getdefaultipinterface() on IPv6 */
        rc = em_getdefaultipinterface(em_AF_INET6(), &addr[0]);
        if (rc == EM_SUCC)
            CHECK_SA_ZERO_LEN(&addr[0], -50);

        /* em_getaddrinfo() on a host name */
        cnt = EM_ARRAY_SIZE(ai);
        rc = em_getaddrinfo(em_AF_UNSPEC(), em_gethostname(), &cnt, ai);
        if (rc == EM_SUCC) {
            while (cnt--)
                CHECK_SA_ZERO_LEN(&ai[cnt].ai_addr, -60);
        }

        /* em_getaddrinfo() on an IP address */
        cnt = EM_ARRAY_SIZE(ai);
        rc = em_getaddrinfo(em_AF_UNSPEC(), &str_ip, &cnt, ai);
        if (rc == EM_SUCC) {
            while (cnt--)
                CHECK_SA_ZERO_LEN(&ai[cnt].ai_addr, -70);
        }
    }
#endif

    return 0;
}

static int simple_sock_test(void)
{
    int types[2];
    em_sock_t sock;
    int i;
    emlib_ret_t rc = EM_SUCC;

    types[0] = em_SOCK_STREAM();
    types[1] = em_SOCK_DGRAM();

    EM_LOG(3, "...simple_sock_test()");

    for (i=0; i<(int)(sizeof(types)/sizeof(types[0])); ++i) {

        rc = em_sock_socket(em_AF_INET(), types[i], 0, &sock);
        if (rc != EM_SUCC) {
            app_perror("...error: unable to create socket", rc);
            break;
        } else {
            rc = em_sock_close(sock);
            if (rc != 0) {
                app_perror("...error: close socket", rc);
                break;
            }
        }
    }
    return rc;
}


static int send_recv_test(int sock_type,
        em_sock_t ss, em_sock_t cs,
        em_sockaddr_in *dstaddr, em_sockaddr_in *srcaddr, 
        int addrlen)
{
    enum { DATA_LEN = 16 };
    char senddata[DATA_LEN+4], recvdata[DATA_LEN+4];
    em_ssize_t sent, received, total_received;
    emlib_ret_t rc;

    EM_LOG(EM_LOG_TRACE, "....create_random_string()");
    em_create_random_string(senddata, DATA_LEN);
    senddata[DATA_LEN-1] = '\0';

    /*
     * Test send/recv small data.
     */
    EM_LOG(EM_LOG_TRACE, "....sendto()");
    if (dstaddr) {
        sent = DATA_LEN;
        rc = em_sock_sendto(cs, senddata, &sent, 0, dstaddr, addrlen);
        if (rc != EM_SUCC || sent != DATA_LEN) {
            app_perror("...sendto error", rc);
            rc = -140; goto on_error;
        }
    } else {
        sent = DATA_LEN;
        rc = em_sock_send(cs, senddata, &sent, 0);
        if (rc != EM_SUCC || sent != DATA_LEN) {
            app_perror("...send error", rc);
            rc = -145; goto on_error;
        }
    }

    EM_LOG(EM_LOG_TRACE,  "....recv()");
    if (srcaddr) {
        em_sockaddr_in addr;
        int srclen = sizeof(addr);

        em_bzero(&addr, sizeof(addr));

        received = DATA_LEN;
        rc = em_sock_recvfrom(ss, recvdata, &received, 0, &addr, &srclen);
        if (rc != EM_SUCC || received != DATA_LEN) {
            app_perror("...recvfrom error", rc);
            rc = -150; goto on_error;
        }
        if (srclen != addrlen)
            return -151;
        if (em_sockaddr_cmp(&addr, srcaddr) != 0) {
            char srcaddr_str[32], addr_str[32];
            strcpy(srcaddr_str, em_inet_ntoa(srcaddr->sin_addr));
            strcpy(addr_str, em_inet_ntoa(addr.sin_addr));
            EM_LOG(3, "...error: src address mismatch (original=%s, "
                        "recvfrom addr=%s)", 
                        srcaddr_str, addr_str);
            return -152;
        }

    } else {
        /* Repeat recv() until all data is received.
         * This applies only for non-UDP of course, since for UDP
         * we would expect all data to be received in one packet.
         */
        total_received = 0;
        do {
            received = DATA_LEN-total_received;
            rc = em_sock_recv(ss, recvdata+total_received, &received, 0);
            if (rc != EM_SUCC) {
                app_perror("...recv error", rc);
                rc = -155; goto on_error;
            }
            if (received <= 0) {
                EM_LOG(3,"...error: socket has closed! (received=%d)",
                            received);
                rc = -156; goto on_error;
            }
            if (received != DATA_LEN-total_received) {
                if (sock_type != em_SOCK_STREAM()) {
                    EM_LOG(3, "...error: expecting %u bytes, got %u bytes",
                                DATA_LEN-total_received, received);
                    rc = -157; goto on_error;
                }
            }
            total_received += received;
        } while (total_received < DATA_LEN);
    }

    EM_LOG(EM_LOG_TRACE,"....memcmp()");
    if (em_memcmp(senddata, recvdata, DATA_LEN) != 0) {
        EM_LOG(3,"...error: received data mismatch "
                    "(got:'%s' expecting:'%s'",
                    recvdata, senddata);
        rc = -160; goto on_error;
    }

    /*
     * Test send/recv big data.
     */
    EM_LOG(EM_LOG_TRACE,"....sendto()");
    if (dstaddr) {
        sent = BIG_DATA_LEN;
        rc = em_sock_sendto(cs, bigdata, &sent, 0, dstaddr, addrlen);
        if (rc != EM_SUCC || sent != BIG_DATA_LEN) {
            app_perror("...sendto error", rc);
            rc = -161; goto on_error;
        }
    } else {
        sent = BIG_DATA_LEN;
        rc = em_sock_send(cs, bigdata, &sent, 0);
        if (rc != EM_SUCC || sent != BIG_DATA_LEN) {
            app_perror("...send error", rc);
            rc = -165; goto on_error;
        }
    }

    EM_LOG(EM_LOG_TRACE,"....recv()");

    /* Repeat recv() until all data is received.
     * This applies only for non-UDP of course, since for UDP
     * we would expect all data to be received in one packet.
     */
    total_received = 0;
    do {
        received = BIG_DATA_LEN-total_received;
        rc = em_sock_recv(ss, bigbuffer+total_received, &received, 0);
        if (rc != EM_SUCC) {
            app_perror("...recv error", rc);
            rc = -170; goto on_error;
        }
        if (received <= 0) {
            EM_LOG(3, "...error: socket has closed! (received=%d)",
                        received);
            rc = -173; goto on_error;
        }
        if (received != BIG_DATA_LEN-total_received) {
            if (sock_type != em_SOCK_STREAM()) {
                EM_LOG(3, "...error: expecting %u bytes, got %u bytes",
                            BIG_DATA_LEN-total_received, received);
                rc = -176; goto on_error;
            }
        }
        total_received += received;
    } while (total_received < BIG_DATA_LEN);

    EM_LOG(EM_LOG_TRACE,"....memcmp()");
    if (em_memcmp(bigdata, bigbuffer, BIG_DATA_LEN) != 0) {
        EM_LOG(3, "...error: received data has been altered!");
        rc = -180; goto on_error;
    }

    rc = 0;

on_error:
    return rc;
}

static int udp_test(void)
{
    em_sock_t cs = EM_INVALID_SOCKET, ss = EM_INVALID_SOCKET;
    em_sockaddr_in dstaddr, srcaddr;
    em_str_t s;
    emlib_ret_t rc = 0, retval;

    EM_LOG(3, "...udp_test()");

    rc = em_sock_socket(em_AF_INET(), em_SOCK_DGRAM(), 0, &ss);
    if (rc != 0) {
        app_perror("...error: unable to create socket", rc);
        return -100;
    }

    rc = em_sock_socket(em_AF_INET(), em_SOCK_DGRAM(), 0, &cs);
    if (rc != 0)
        return -110;

    /* Bind server socket. */
    em_bzero(&dstaddr, sizeof(dstaddr));
    dstaddr.sin_family = em_AF_INET();
    dstaddr.sin_port = em_htons(UDP_PORT);
    dstaddr.sin_addr = em_inet_addr(em_cstr(&s, ADDRESS));

    if ((rc=em_sock_bind(ss, &dstaddr, sizeof(dstaddr))) != 0) {
        app_perror("...bind error udp:"ADDRESS, rc);
        rc = -120; goto on_error;
    }

    /* Bind client socket. */
    em_bzero(&srcaddr, sizeof(srcaddr));
    srcaddr.sin_family = em_AF_INET();
    srcaddr.sin_port = em_htons(UDP_PORT-1);
    srcaddr.sin_addr = em_inet_addr(em_cstr(&s, ADDRESS));

    if ((rc=em_sock_bind(cs, &srcaddr, sizeof(srcaddr))) != 0) {
        app_perror("...bind error", rc);
        rc = -121; goto on_error;
    }

    /* Test send/recv, with sendto */
    rc = send_recv_test(em_SOCK_DGRAM(), ss, cs, &dstaddr, NULL, 
            sizeof(dstaddr));
    if (rc != 0)
        goto on_error;

    /* Test send/recv, with sendto and recvfrom */
    rc = send_recv_test(em_SOCK_DGRAM(), ss, cs, &dstaddr, 
            &srcaddr, sizeof(dstaddr));
    if (rc != 0)
        goto on_error;

    /* connect() the sockets. */
    rc = em_sock_connect(cs, &dstaddr, sizeof(dstaddr));
    if (rc != 0) {
        app_perror("...connect() error", rc);
        rc = -122; goto on_error;
    }

    /* Test send/recv with send() */
    rc = send_recv_test(em_SOCK_DGRAM(), ss, cs, NULL, NULL, 0);
    if (rc != 0)
        goto on_error;

    /* Test send/recv with send() and recvfrom */
    rc = send_recv_test(em_SOCK_DGRAM(), ss, cs, NULL, &srcaddr, 
            sizeof(srcaddr));
    if (rc != 0)
        goto on_error;

on_error:
    retval = rc;
    if (cs != EM_INVALID_SOCKET) {
        rc = em_sock_close(cs);
        if (rc != EM_SUCC) {
            app_perror("...error in closing socket", rc);
            return -1000;
        }
    }
    if (ss != EM_INVALID_SOCKET) {
        rc = em_sock_close(ss);
        if (rc != EM_SUCC) {
            app_perror("...error in closing socket", rc);
            return -1010;
        }
    }

    return retval;
}

static int tcp_test(void)
{
    em_sock_t cs, ss;
    emlib_ret_t rc = 0, retval;

    EM_LOG(3, "...tcp_test()");

    rc = app_socketpair(em_AF_INET(), em_SOCK_STREAM(), 0, &ss, &cs);
    if (rc != EM_SUCC) {
        app_perror("...error: app_socketpair():", rc);
        return -2000;
    }

    /* Test send/recv with send() and recv() */
    retval = send_recv_test(em_SOCK_STREAM(), ss, cs, NULL, NULL, 0);

    rc = em_sock_close(cs);
    if (rc != EM_SUCC) {
        app_perror("...error in closing socket", rc);
        return -2000;
    }

    rc = em_sock_close(ss);
    if (rc != EM_SUCC) {
        app_perror("...error in closing socket", rc);
        return -2010;
    }

    return retval;
}

static int ioctl_test(void)
{
    return 0;
}

static int gethostbyname_test(void)
{
    em_str_t host;
    em_hostent he;
    emlib_ret_t status;

    EM_LOG(3, "...gethostbyname_test()");

    /* Testing em_gethostbyname() with invalid host */
    host = em_str("an-invalid-host-name");
    status = em_gethostbyname(&host, &he);

    /* Must return failure! */
    if (status != EM_SUCC)
        return -20100;
    else
        return 0;
}

#if 0
#include "../em/os_symbian.h"
static int connect_test()
{
    RSocketServ rSockServ;
    RSocket rSock;
    TInetAddr inetAddr;
    TRequestStatus reqStatus;
    char buffer[16];
    TPtrC8 data((const TUint8*)buffer, (TInt)sizeof(buffer));
    int rc;

    rc = rSockServ.Connect();
    if (rc != KErrNone)
        return rc;

    rc = rSock.Open(rSockServ, KAfInet, KSockDatagram, KProtocolInetUdp);
    if (rc != KErrNone) 
    {    		
        rSockServ.Close();
        return rc;
    }

    inetAddr.Init(KAfInet);
    inetAddr.Input(_L("127.0.0.1"));
    inetAddr.SetPort(80);

    rSock.Connect(inetAddr, reqStatus);
    User::WaitForRequest(reqStatus);

    if (reqStatus != KErrNone) {
        rSock.Close();
        rSockServ.Close();
        return rc;
    }

    rSock.Send(data, 0, reqStatus);
    User::WaitForRequest(reqStatus);

    if (reqStatus!=KErrNone) {
        rSock.Close();
        rSockServ.Close();
        return rc;
    }

    rSock.Close();
    rSockServ.Close();
    return KErrNone;
}
#endif

emlib_ret_t sock_test()
{
    int rc;

    em_create_random_string(bigdata, BIG_DATA_LEN);

    // Enable this to demonstrate the error with S60 3rd Edition MR2
#if 0
    rc = connect_test();
    if (rc != 0)
        return rc;
#endif

    rc = format_test();
    if (rc != 0)
        return rc;

    rc = parse_test();
    if (rc != 0)
        return rc;

    rc = purity_test();
    if (rc != 0)
        return rc;

    rc = gethostbyname_test();
    if (rc != 0)
        return rc;

    rc = simple_sock_test();
    if (rc != 0)
        return rc;

    rc = ioctl_test();
    if (rc != 0)
        return rc;

    rc = udp_test();
    if (rc != 0)
        return rc;

    rc = tcp_test();
    if (rc != 0)
        return rc;

    return 0;
}


#else
/* To prevent warning about "translation unit is empty"
 * when this test is disabled. 
 */
int dummy_sock_test;
#endif	/* INCLUDE_SOCK_TEST */

