// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagdiriterator_p.h"
#include "utils/tagmanager.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/fileutils.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

TagDirIteratorPrivate::TagDirIteratorPrivate(TagDirIterator *qq)
    : q(qq)
{
}

TagDirIteratorPrivate::~TagDirIteratorPrivate()
{
}

void TagDirIteratorPrivate::loadTagsUrls(const QUrl &url)
{
    if (url == TagManager::instance()->rootUrl()) {
        QMap<QString, QColor> tagsMap = TagManager::instance()->getAllTags();
        QMap<QString, QColor>::const_iterator it = tagsMap.begin();
        while (it != tagsMap.end()) {
            QUrl tagUrl;
            tagUrl.setScheme(TagManager::scheme());
            tagUrl.setPath("/" + it.key());

            const FileInfoPointer &info = InfoFactory::create<FileInfo>(tagUrl);
            tagNodes.insert(tagUrl, info);
            urlList.append(tagUrl);
            ++it;
        }
    } else {
        const QString &tagName = url.path().mid(1, url.path().length() - 1);
        QStringList pathList = TagManager::instance()->getFilesByTag(tagName);

        for (const QString &path : pathList) {
            QUrl tagUrl = QUrl::fromLocalFile(path);
            const FileInfoPointer &info = InfoFactory::create<FileInfo>(tagUrl);
            if (!info || !info->exists())
                continue;

            tagNodes.insert(tagUrl, info);
            urlList.append(tagUrl);
        }
    }
}
