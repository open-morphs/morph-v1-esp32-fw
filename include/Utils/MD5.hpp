#pragma once

#include "../esp32/rom/md5_hash.h"
#include <string>

using namespace std;
static const uint16_t MD5_32_LENGTH = 32;

inline string getMD5String(const unsigned char *in, const size_t length) {

    MD5Context ctx = {};
    MD5Init(&ctx);
    MD5Update(&ctx, in, length);

    unsigned char final[16];
    MD5Final(final, &ctx);

    static const char hexits[17] = "0123456789abcdef";
    char out[MD5_32_LENGTH + 1];

    for (int i = 0; i < 16; i++) {
        out[i * 2] = hexits[final[i] >> 4];
        out[(i * 2) + 1] = hexits[final[i] & 0x0F];
    }
    out[MD5_32_LENGTH] = '\0';
    return string(out);
}

inline string getMD5String(const string &contentString) {
    return getMD5String(reinterpret_cast<const unsigned char *>(contentString.c_str()), contentString.length());
}