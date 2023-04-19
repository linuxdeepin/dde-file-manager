// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagmanagerdbus.h"
#include "serverplugin_tagdaemon_global.h"
#include "tagdbhandler.h"

SERVERTAGDAEMON_USE_NAMESPACE

TagManagerDBus::TagManagerDBus(QObject *parent)
    : QObject(parent)
{
    initConnect();
}

void TagManagerDBus::initConnect()
{
    connect(TagDbHandler::instance(), &TagDbHandler::newTagsAdded, this, &TagManagerDBus::NewTagsAdded);
    connect(TagDbHandler::instance(), &TagDbHandler::tagsDeleted, this, &TagManagerDBus::TagsDeleted);
    connect(TagDbHandler::instance(), &TagDbHandler::tagsColorChanged, this, &TagManagerDBus::TagsColorChanged);
    connect(TagDbHandler::instance(), &TagDbHandler::tagsNameChanged, this, &TagManagerDBus::TagsNameChanged);
    connect(TagDbHandler::instance(), &TagDbHandler::filesWereTagged, this, &TagManagerDBus::FilesTagged);
    connect(TagDbHandler::instance(), &TagDbHandler::filesUntagged, this, &TagManagerDBus::FilesUntagged);
}

QDBusVariant TagManagerDBus::Query(int opt, const QStringList value)
{
    QDBusVariant dbusVar {};
    QueryOpts queryOpt { opt };

    switch (queryOpt) {
    case QueryOpts::kTags:
        dbusVar.setVariant(TagDbHandler::instance()->getAllTags());
        break;
    case QueryOpts::kFilesWithTags:
        dbusVar.setVariant(TagDbHandler::instance()->getAllFileWithTags());
        break;
    case QueryOpts::kTagsOfFile:
        dbusVar.setVariant(TagDbHandler::instance()->getTagsByUrls(value));
        break;
    case QueryOpts::kFilesOfTag:
        dbusVar.setVariant(TagDbHandler::instance()->getFilesByTag(value));
        break;
    case QueryOpts::kColorOfTags:
        dbusVar.setVariant(TagDbHandler::instance()->getTagsColor(value));
        break;
    case QueryOpts::kTagIntersectionOfFiles:
        dbusVar.setVariant(TagDbHandler::instance()->getSameTagsOfDiffUrls(value));
        break;
    }

    return dbusVar;
}

bool TagManagerDBus::Insert(int opt, const QVariantMap value)
{
    InsertOpts insetOpt { opt };

    switch (insetOpt) {
    case InsertOpts::kTags:
        return TagDbHandler::instance()->addTagProperty(value);
    case InsertOpts::kTagOfFiles:
        return TagDbHandler::instance()->addTagsForFiles(value);
    }

    return false;
}

bool TagManagerDBus::Delete(int opt, const QVariantMap value)
{
    DeleteOpts deleteOpt { opt };

    switch (deleteOpt) {
    case DeleteOpts::kTags:
        return TagDbHandler::instance()->deleteTags(value.first().toStringList());
    case DeleteOpts::kFiles:
        return TagDbHandler::instance()->deleteFiles(value.keys());
    case DeleteOpts::kTagOfFiles:
        return TagDbHandler::instance()->removeTagsOfFiles(value);
    }

    return false;
}

bool TagManagerDBus::Update(int opt, const QVariantMap value)
{
    UpdateOpts updateOpt { opt };

    switch (updateOpt) {
    case UpdateOpts::kColors:
        return TagDbHandler::instance()->changeTagColors(value);
    case UpdateOpts::kTagsNameWithFiles:
        return TagDbHandler::instance()->changeTagNamesWithFiles(value);
    case UpdateOpts::kFilesPaths:
        return TagDbHandler::instance()->changeFilePaths(value);
    }

    return false;
}
