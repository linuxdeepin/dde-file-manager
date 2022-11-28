// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
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

QVariant TagDBus::Query(const quint8 &type, const QStringList value)
{
    if (0 == type || (type > 1 && value.isEmpty())) {
        lastErr = "input parameter is empty!";
        return {};
    }

    switch (TagActionType(type)) {
    case TagActionType::kGetAllTags:
        return getAllTags();
    case TagActionType::kGetTagsThroughFile:
        return getTagsThroughFile(value);
    case TagActionType::kGetSameTagsOfDiffFiles:
        return getSameTagsOfDiffFiles(value);
    case TagActionType::kGetFilesThroughTag:
        return getFilesThroughTag(value);
    case TagActionType::kGetTagsColor:
        return getTagsColor(value);
    default: {
        return {};
    }
    }
}

bool TagDBus::Insert(const quint8 &type, const QVariantMap value)
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

bool TagDBus::Delete(const quint8 &type, const QVariantMap value)
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

bool TagDBus::Update(const quint8 &type, const QVariantMap value)
{
    if (value.isEmpty() || (type == 0)) {
        lastErr = "input parameter is empty!";
        return false;
    }

    switch (TagActionType(type)) {
    case TagActionType::kChangeTagColor:
        return changeTagColor(value);
    case TagActionType::kChangeTagName:
        return changeTagName(value);
    case TagActionType::kChangeFilesName:
        return changeFilesName(value);
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

bool TagDBus::changeTagColor(const QVariantMap value)
{
    return TagDbHandle::instance()->changeTagColors(value);
}

bool TagDBus::changeTagName(const QVariantMap value)
{
    return TagDbHandle::instance()->changeTagNames(value);
}

bool TagDBus::changeFilesName(const QVariantMap value)
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
    connect(TagDbHandle::instance(), &TagDbHandle::addedNewTags, this, &TagDBus::onAddNewTags);
    connect(TagDbHandle::instance(), &TagDbHandle::deletedTags, this, &TagDBus::onDeleteTags);
    connect(TagDbHandle::instance(), &TagDbHandle::changedTagColor, this, &TagDBus::onChangeTagColor);
    connect(TagDbHandle::instance(), &TagDbHandle::changedTagName, this, &TagDBus::onChangeTagName);
    connect(TagDbHandle::instance(), &TagDbHandle::filesWereTagged, this, &TagDBus::onFilesWereTagged);
    connect(TagDbHandle::instance(), &TagDbHandle::untagFiles, this, &TagDBus::onUntagFiles);
}

void TagDBus::onAddNewTags(const QVariant &newTags)
{
    QStringList tags = newTags.toStringList();
    emit addedNewTags(tags);
}

void TagDBus::onChangeTagColor(const QVariantMap &oldAndNewColor)
{
    QMap<QString, QString> tagToNewColorName {};
    QMap<QString, QVariant>::const_iterator it = oldAndNewColor.begin();
    while (it != oldAndNewColor.end()) {
        tagToNewColorName[it.key()] = it.value().toString();
        ++it;
    }

    emit tagColorChanged(tagToNewColorName);
}

void TagDBus::onChangeTagName(const QVariantMap &oldAndNewName)
{
    QMap<QString, QString> oldToNewTagName {};
    QMap<QString, QVariant>::const_iterator it = oldAndNewName.begin();
    while (it != oldAndNewName.end()) {
        oldToNewTagName[it.key()] = it.value().toString();
        ++it;
    }

    emit tagNameChanged(oldToNewTagName);
}

void TagDBus::onDeleteTags(const QVariant &deletedTags)
{
    QStringList tags = deletedTags.toStringList();
    emit tagsDeleted(tags);
}

void TagDBus::onFilesWereTagged(const QVariantMap &taggedFiles)
{
    QMap<QString, QList<QString>> fileAndTags {};
    QMap<QString, QVariant>::const_iterator it = taggedFiles.begin();
    while (it != taggedFiles.end()) {
        fileAndTags[it.key()] = it.value().toStringList();
        ++it;
    }

    emit filesTagged(fileAndTags);
}

void TagDBus::onUntagFiles(const QVariantMap &untaggedFiles)
{
    QMap<QString, QList<QString>> fileAndTags {};
    QMap<QString, QVariant>::const_iterator it = untaggedFiles.begin();
    while (it != untaggedFiles.end()) {
        fileAndTags[it.key()] = it.value().toStringList();
        ++it;
    }

    emit filesUntagged(fileAndTags);
}
