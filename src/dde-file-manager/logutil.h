// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGUTIL_H
#define LOGUTIL_H


class LogUtil
{
public:
    LogUtil();
    ~LogUtil();

    static void registerLogger();
};

#endif // LOGUTIL_H
