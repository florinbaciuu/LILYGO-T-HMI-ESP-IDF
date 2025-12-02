/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/* https://en.cppreference.com/w/c/string/byte/atoi.html */

#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "qa_utils.h"


int qa_utils_str2int(const char* str)
{
    if (str == NULL) {return -1;}
    // return atoi(str);
    return (int)strtol(str, NULL, 0);
}

int qa_utils_hex2data(const char* str, uint8_t* data, int data_len)
{
    if (str == NULL || data == NULL) {return -1;}
    char *pos = (char *)str;
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        pos += 2;
    }

    memset(data, 0, data_len);
    int hexstr_len = strlen(pos);
    if (hexstr_len % 2 != 0 || hexstr_len / 2 > data_len) {
        QA_TEST_ERROR("hex2data:%s", str);
        return -1;
    }

    for (int i = 0; i < hexstr_len / 2; i++) {
        char ch1 = pos[i * 2];
        char ch2 = pos[i * 2 + 1];
        if (!isxdigit(ch1) || !isxdigit(ch2)) {
            QA_TEST_ERROR("hex2data:%s", str);
            return -1;
        }
        data[i] = (isdigit(ch1) ? ch1 - '0' : tolower(ch1) - 'a' + 10) << 4;
        data[i] |= (isdigit(ch2) ? ch2 - '0' : tolower(ch2) - 'a' + 10);
    }
    return 0;
}


int qa_utils_str2mac(const char* str, uint8_t* mac, int mac_len)
{
    if (str == NULL || mac == NULL) {return -1;}
    memset(mac, 0, mac_len);
    int str_len = strlen(str);
    /* must be like 12:34:56:78:90:AB  (6 or 8 bytes) */
    if ((str_len != 17 && str_len != 23) || mac_len != (str_len+1)/3) {
        QA_TEST_ERROR("str2mac:%s,%d", str, mac_len);
        return -1;
    }

    for (int i=0; i < mac_len; i++) {
        char ch1 = str[i * 3];
        char ch2 = str[i * 3 + 1];
        if (!isxdigit(ch1) || !isxdigit(ch2)) {
            QA_TEST_ERROR("hex2data:%s", str);
            return -1;
        }
        mac[i] = (isdigit(ch1) ? ch1 - '0' : tolower(ch1) - 'a' + 10) << 4;
        mac[i] |= (isdigit(ch2) ? ch2 - '0' : tolower(ch2) - 'a' + 10);

    }
    return 0;
}


int qa_utils_mac2str(const uint8_t* mac, int mac_len, char* str, int str_len)
{
    if (mac == NULL || str == NULL) {return -1;}
    /* 6 or 8 bytes, str_len include '\0' */
    if ((mac_len != 6 && mac_len != 8) || str_len < mac_len * 3 ) {
        QA_TEST_ERROR("mac2str:%d,%d", mac_len, str_len);
        return -1;
    }
    memset(str, 0, str_len);
    for (int i = 0; i < mac_len; i ++) {
        sprintf(str+i*3, "%02x", mac[i]);
        str[i*3+2] = ':';
    }
    str[mac_len*3-1] = '\0';
    return 0;
}
