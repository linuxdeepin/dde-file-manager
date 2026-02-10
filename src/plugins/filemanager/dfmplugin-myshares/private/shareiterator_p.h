// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREITERATOR_P_H
#define SHAREITERATOR_P_H

#include "dfmplugin_myshares_global.h"

#include <QUrl>

namespace dfmbase {
class LocalDirIterator;
}

namespace dfmplugin_myshares {

class ShareIterator;
class ShareIteratorPrivate
{
    friend class ShareIterator;

public:
    explicit ShareIteratorPrivate(ShareIterator *qq, const QUrl &url);
    ~ShareIteratorPrivate();

private:
    dfmbase::LocalDirIterator *proxy { nullptr };

    ShareIterator *q { nullptr };
    ShareInfoList shares;
    ShareInfo currentInfo;

    QUrl rootUrl;
};

}

#endif   // SHAREITERATOR_P_H
