/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "funcwrapper.h"

#include <iostream>
#include <sstream>
#include <cstring>

char *toBase64(const unsigned char *data, int size, int options)
{
    const char alphabet_base64[] = "ABCDEFGH" "IJKLMNOP" "QRSTUVWX" "YZabcdef"
                                   "ghijklmn" "opqrstuv" "wxyz0123" "456789+/";
    const char alphabet_base64url[] = "ABCDEFGH" "IJKLMNOP" "QRSTUVWX" "YZabcdef"
                                      "ghijklmn" "opqrstuv" "wxyz0123" "456789-_";
    const char *const alphabet = (options & Base64UrlEncoding) ? alphabet_base64url : alphabet_base64;
    const char padchar = '=';
    int padlen = 0;

    int tmp_size = (size + 2) / 3 * 4;
    char *tmp = new char[static_cast<unsigned int>(tmp_size) + 1];
    tmp[tmp_size] = '\0';

    int i = 0;
    char *out = tmp;
    while (i < size) {
        // encode 3 bytes at a time
        int chunk = 0;
        chunk |= int((data[i++])) << 16;
        if (i == size) {
            padlen = 2;
        } else {
            chunk |= int((data[i++])) << 8;
            if (i == size)
                padlen = 1;
            else
                chunk |= int((data[i++]));
        }

        int j = (chunk & 0x00fc0000) >> 18;
        int k = (chunk & 0x0003f000) >> 12;
        int l = (chunk & 0x00000fc0) >> 6;
        int m = (chunk & 0x0000003f);
        *out++ = alphabet[j];
        *out++ = alphabet[k];

        if (padlen > 1) {
            if ((options & OmitTrailingEquals) == 0)
                *out++ = padchar;
        } else {
            *out++ = alphabet[l];
        }
        if (padlen > 0) {
            if ((options & OmitTrailingEquals) == 0)
                *out++ = padchar;
        } else {
            *out++ = alphabet[m];
        }
    }

    if (!((options & OmitTrailingEquals) || (out == tmp_size + tmp))) {
        std::abort();
    }

    if (options & OmitTrailingEquals) {
        int new_size = static_cast<int>(out - tmp);
        char *new_tmp = new char[static_cast<unsigned int>(new_size) + 1];

        memcpy(new_tmp, tmp, static_cast<unsigned int>(new_size));
        delete[] tmp;

        tmp = new_tmp;
    }

    return tmp;
}
