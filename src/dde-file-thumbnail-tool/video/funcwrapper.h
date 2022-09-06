// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FUNCWRAPPER_H
#define FUNCWRAPPER_H

enum Base64Option {
    Base64Encoding = 0,
    Base64UrlEncoding = 1,

    KeepTrailingEquals = 0,
    OmitTrailingEquals = 2
};

char *toBase64(const unsigned char *data, int size, int options);

#endif // FUNCWRAPPER_H

