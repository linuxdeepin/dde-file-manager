// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTDIRITERATORPRIVATE_H
#define RECENTDIRITERATORPRIVATE_H

#include "dfmplugin_recent_global.h"
#include <dfm-base/interfaces/abstractdiriterator.h>

#include <QQueue>

namespace dfmplugin_recent {

class RecentDirIterator;

class RecentDirIteratorPrivate
{
    friend class RecentDirIterator;

public:
    explicit RecentDirIteratorPrivate(RecentDirIterator *qq);
    ~RecentDirIteratorPrivate();

private:
    QUrl currentUrl;
    QQueue<QUrl> urlList;
    QMap<QUrl, FileInfoPointer> recentNodes;
    RecentDirIterator *q { nullptr };
};

}
#endif   // RECENTDIRITERATORPRIVATE_H
