// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagfilewatcher.h"
#include "private/tagfilewatcher_p.h"
#include "utils/taghelper.h"

#include <dfm-base/utils/universalutils.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

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

void TagFileWatcher::onFilesTagged(const QVariantMap &fileAndTags)
{
    QString tagName = TagHelper::instance()->getTagNameFromUrl(dptr->url);

    auto iter = fileAndTags.begin();
    while (iter != fileAndTags.end()) {
        if (iter.value().toStringList().contains(tagName)) {
            QUrl fileUrl = QUrl::fromLocalFile(iter.key());

            emit AbstractFileWatcher::subfileCreated(fileUrl);
        }
        ++iter;
    }
}

void TagFileWatcher::onFilesUntagged(const QVariantMap &fileAndTags)
{
    QString tagName = TagHelper::instance()->getTagNameFromUrl(dptr->url);

    auto iter = fileAndTags.begin();
    while (iter != fileAndTags.end()) {
        if (iter.value().toStringList().contains(tagName)) {
            QUrl fileUrl = QUrl::fromLocalFile(iter.key());

            emit AbstractFileWatcher::fileDeleted(fileUrl);
        }
        ++iter;
    }
}

void TagFileWatcher::onFilesHidden(const QVariantMap &fileAndTags)
{
    QString tagName = TagHelper::instance()->getTagNameFromUrl(dptr->url);

    auto iter = fileAndTags.begin();
    while (iter != fileAndTags.end()) {
        if (iter.value().toStringList().contains(tagName)) {
            QUrl fileUrl = QUrl::fromLocalFile(iter.key());

            emit AbstractFileWatcher::fileAttributeChanged(fileUrl);
        }
        ++iter;
    }
}
