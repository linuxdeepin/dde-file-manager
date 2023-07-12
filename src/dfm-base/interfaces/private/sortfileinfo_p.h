// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SORTFILEINFO_P_H
#define SORTFILEINFO_P_H

#include <dfm-base/interfaces/sortfileinfo.h>

#include <QPointer>

namespace dfmbase {
class SortFileInfoPrivate
{
public:
    explicit SortFileInfoPrivate(SortFileInfo *qq);
    ~SortFileInfoPrivate();

public:
    SortFileInfo *const q;   // SortFileInfo实例对象
    QUrl url;
    qint64 filesize { 0 };
    bool file { false };
    bool dir { false };
    bool symLink { false };
    bool hide { false };
    bool readable { false };
    bool writeable { false };
    bool executable { false };
};

}

#endif   // SORTFILEINFO_P_H
