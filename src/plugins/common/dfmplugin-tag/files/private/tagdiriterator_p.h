// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGDIRITERATOR_P_H
#define TAGDIRITERATOR_P_H

#include "dfmplugin_tag_global.h"
#include <dfm-base/interfaces/abstractdiriterator.h>

#include <QQueue>

namespace dfmplugin_tag {

class TagDirIterator;
class TagDirIteratorPrivate
{
    friend class TagDirIterator;

public:
    explicit TagDirIteratorPrivate(TagDirIterator *qq);
    ~TagDirIteratorPrivate();

    void loadTagsUrls(const QUrl &url);

private:
    QUrl rootUrl;
    QUrl currentUrl;
    QQueue<QUrl> urlList;
    QMap<QUrl, FileInfoPointer> tagNodes;
    TagDirIterator *q { nullptr };
};

}

#endif   // TAGDIRITERATOR_P_H
