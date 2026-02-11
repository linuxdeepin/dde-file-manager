// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SORTFILEINFO_P_H
#define SORTFILEINFO_P_H

#include <dfm-base/interfaces/sortfileinfo.h>

#include <QPointer>
#include <QMutex>

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
    QVariantMap customData;

    // time info
    qint64 lastRead { 0 };
    qint64 lastModifed { 0 };
    qint64 create { 0 };

    QString highlightContent { "" };   // 存储文件的高亮内容

    // 信息完整性标记
    bool infoCompleted { false };   // 标记详细信息是否已获取

    // 线程安全
    mutable QMutex mutex;
};

}

#endif   // SORTFILEINFO_P_H
