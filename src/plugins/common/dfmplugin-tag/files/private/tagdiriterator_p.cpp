/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "tagdiriterator_p.h"
#include "utils/tagmanager.h"

#include "dfm-base/base/schemefactory.h"

DFMBASE_USE_NAMESPACE
DPTAG_USE_NAMESPACE

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

            const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(tagUrl);
            tagNodes.insert(tagUrl, info);
            urlList.append(tagUrl);
            ++it;
        }
    } else {
        const QString &tagName = url.path().mid(1, url.path().length() - 1);
        QStringList pathList = TagManager::instance()->getFilesByTag(tagName);

        for (const QString &path : pathList) {
            QUrl tagUrl;
            tagUrl.setScheme(TagManager::scheme());
            tagUrl.setPath("/" + tagName);
            tagUrl.setFragment(path);

            const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(tagUrl);
            if (!info->exists())
                continue;

            tagNodes.insert(tagUrl, info);
            urlList.append(tagUrl);
        }
    }
}
