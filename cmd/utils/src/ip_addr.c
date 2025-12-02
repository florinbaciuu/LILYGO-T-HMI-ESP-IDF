/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <sys/socket.h>    /* AF_INET, AF_INET6 */
#include <netinet/in.h>    /* sockaddr_in, sockaddr_in6 */
#include <arpa/inet.h>     /* inet_pton, inet_ntop */
#include "qa_utils.h"

int qa_utils_ip2str(uint32_t ip, char* str, int str_len)
{
    if (str == NULL || str_len < 16) {
        return -1;
    }

    return inet_ntop(AF_INET, &ip, str, str_len) == NULL ? -1 : 0;
}

int qa_utils_str2ip(const char* str, uint32_t* ip)
{
    if (str == NULL || ip == NULL) {
        return -1;
    }

    struct in_addr in_addr;
    if (inet_pton(AF_INET, str, &in_addr) != 1) {
        return -1;
    }

    *ip = in_addr.s_addr;
    return 0;
}
