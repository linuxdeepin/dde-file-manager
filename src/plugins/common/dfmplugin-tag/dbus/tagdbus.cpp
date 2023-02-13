// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagdbus.h"
#include "dfmplugin_tag_global.h"
#include "data/tagdbhandle.h"
#include "utils/tagmanager.h"

#include "dfm-base/base/schemefactory.h"

DPTAG_USE_NAMESPACE

TagDBus::TagDBus(QObject *parent)
    : QObject(parent)
{
    initconnect();
}

TagDBus::~TagDBus()
{
}

QDBusVariant TagDBus::Query(const quint32 &type, const QStringList value)
{
    if (0 == type || (type > 1 && value.isEmpty())) {
        lastErr = "input parameter is empty!";
        return {};
    }

    QDBusVariant dbusVar {};
    switch (TagActionType(type)) {
    case TagActionType::kGetAllTags:
        dbusVar.setVariant(getAllTags());
        break;
    case TagActionType::kGetTagsThroughFile:
        dbusVar.setVariant(getTagsThroughFile(value));
        break;
    case TagActionType::kGetSameTagsOfDiffFiles:
        dbusVar.setVariant(getSameTagsOfDiffFiles(value));
        break;
    case TagActionType::kGetFilesThroughTag:
        dbusVar.setVariant(getFilesThroughTag(value));
        break;
    case TagActionType::kGetTagsColor:
        dbusVar.setVariant(getTagsColor(value));
        break;
    default:
        break;
    }

    return dbusVar;
}

bool TagDBus::Insert(const quint32 &type, const QVariantMap value)
{
    if (value.isEmpty() || (type == 0)) {
        lastErr = "input parameter is empty!";
        return false;
    }

    switch (TagActionType(type)) {
    case TagActionType::kAddTags:
        return TagDbHandle::instance()->addTagProperty(value);
    case TagActionType::kMakeFilesTags:
        return TagDbHandle::instance()->addTagsForFiles(value);
    default: {
        return false;
    }
    }
}

bool TagDBus::Delete(const quint32 &type, const QVariantMap value)
{
    if (value.isEmpty() || (type == 0)) {
        lastErr = "input parameter is empty!";
        return false;
    }

    switch (TagActionType(type)) {
    case TagActionType::kRemoveTagsOfFiles:
        return deleteFileTags(value);
    case TagActionType::kDeleteTags:
        return deleteTags(value);
    case TagActionType::kDeleteFiles:
        return deleteFiles(value);
    default: {
        return false;
    }
    }
}

bool TagDBus::Update(const quint32 &type, const QVariantMap value)
{
    if (value.isEmpty() || (type == 0)) {
        lastErr = "input parameter is empty!";
        return false;
    }

    switch (TagActionType(type)) {
    case TagActionType::kChangeTagsColor:
        return changeTagColors(value);
    case TagActionType::kChangeTagsNameWithFiles:
        return changeTagNamesWithFiles(value);
    case TagActionType::kChangeFilesPaths:
        return changeFilePaths(value);
    default: {
        return false;
    }
    }
}

bool TagDBus::CanTagFile(const QString &filePath)
{
    return TagManager::instance()->canTagFile(QUrl(filePath));
}

QString TagDBus::LastError()
{
    return lastErr.isEmpty() ? lastErr : TagDbHandle::instance()->lastError();
}

QVariantMap TagDBus::getAllTags()
{
    return TagDbHandle::instance()->getAllTags();
}

QVariantMap TagDBus::getTagsThroughFile(const QStringList value)
{
    return TagDbHandle::instance()->getTagsByUrls(value);
}

QVariant TagDBus::getSameTagsOfDiffFiles(const QStringList value)
{
    return TagDbHandle::instance()->getSameTagsOfDiffUrls(value);
}

QVariantMap TagDBus::getFilesThroughTag(const QStringList value)
{
    return TagDbHandle::instance()->getFilesByTag(value);
}

QVariantMap TagDBus::getTagsColor(const QStringList value)
{
    return TagDbHandle::instance()->getTagsColor(value);
}

bool TagDBus::changeTagColors(const QVariantMap value)
{
    return TagDbHandle::instance()->changeTagColors(value);
}

bool TagDBus::changeTagNamesWithFiles(const QVariantMap value)
{
    return TagDbHandle::instance()->changeTagNamesWithFiles(value);
}

bool TagDBus::changeFilePaths(const QVariantMap value)
{
    return TagDbHandle::instance()->changeFilePaths(value);
}

bool TagDBus::deleteTags(const QVariantMap value)
{
    if (value.isEmpty() || value.first().isNull()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    return TagDbHandle::instance()->deleteTags(value.first().toStringList());
}

bool TagDBus::deleteFiles(const QVariantMap value)
{
    return TagDbHandle::instance()->deleteFiles(value.keys());
}

bool TagDBus::deleteFileTags(const QVariantMap value)
{
    return TagDbHandle::instance()->removeTagsOfFiles(value);
}

void TagDBus::initconnect()
{
    connect(TagDbHandle::instance(), &TagDbHandle::newTagsAdded, this, &TagDBus::NewTagsAdded);
    connect(TagDbHandle::instance(), &TagDbHandle::tagsDeleted, this, &TagDBus::TagsDeleted);
    connect(TagDbHandle::instance(), &TagDbHandle::tagsColorChanged, this, &TagDBus::TagsColorChanged);
    connect(TagDbHandle::instance(), &TagDbHandle::tagsNameChanged, this, &TagDBus::TagsNameChanged);
    connect(TagDbHandle::instance(), &TagDbHandle::filesWereTagged, this, &TagDBus::FilesTagged);
    connect(TagDbHandle::instance(), &TagDbHandle::filesUntagged, this, &TagDBus::FilesUntagged);
}
