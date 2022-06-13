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
#include "tagfilewatcher.h"
#include "private/tagfilewatcher_p.h"
#include "utils/taghelper.h"

#include "dfm-base/utils/universalutils.h"

DFMBASE_USE_NAMESPACE
DPTAG_USE_NAMESPACE

TagFileWatcher::TagFileWatcher(const QUrl &url, QObject *parent)
    : AbstractFileWatcher(new TagFileWatcherPrivate(url, this), parent)
{
    dptr = static_cast<TagFileWatcherPrivate *>(d.data());
    dptr->initFileWatcher();
    dptr->initConnect();
}

TagFileWatcher::~TagFileWatcher()
{
}

void TagFileWatcher::setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled)
{
}

void TagFileWatcher::onTagRemoved(const QString &tagName)
{
    if (UniversalUtils::urlEquals(TagHelper::instance()->makeTagUrlByTagName(tagName), dptr->url))
        emit AbstractFileWatcher::fileDeleted(dptr->url);
}

void TagFileWatcher::onFilesTagged(const QMap<QString, QList<QString>> &fileAndTags)
{
    QString tagName = TagHelper::instance()->getTagNameFromUrl(dptr->url);

    QMap<QString, QList<QString>>::const_iterator iter = fileAndTags.begin();
    while (iter != fileAndTags.end()) {
        if (iter.value().contains(tagName)) {
            QUrl fileUrl = QUrl::fromLocalFile(iter.key());

            emit AbstractFileWatcher::subfileCreated(fileUrl);
            return;
        }
        ++iter;
    }
}

void TagFileWatcher::onFilesUntagged(const QMap<QString, QList<QString>> &fileAndTags)
{
    QString tagName = TagHelper::instance()->getTagNameFromUrl(dptr->url);

    QMap<QString, QList<QString>>::const_iterator iter = fileAndTags.begin();
    while (iter != fileAndTags.end()) {
        if (iter.value().contains(tagName)) {
            QUrl fileUrl = QUrl::fromLocalFile(iter.key());

            emit AbstractFileWatcher::fileDeleted(fileUrl);
            return;
        }
        ++iter;
    }
}

void TagFileWatcher::addWatcher(const QUrl &url)
{
}

void TagFileWatcher::removeWatcher(const QUrl &url)
{
}

void TagFileWatcher::onFileDeleted(const QUrl &url)
{
}

void TagFileWatcher::onFileAttributeChanged(const QUrl &url)
{
}
