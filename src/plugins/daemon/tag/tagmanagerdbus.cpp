// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagmanagerdbus.h"
#include "daemonplugin_tag_global.h"
#include "tagdbhandler.h"

DAEMONPTAG_USE_NAMESPACE

TagManagerDBus::TagManagerDBus(QObject *parent)
    : QObject(parent)
{
    fmInfo() << "TagManagerDBus: Initializing tag manager DBus service";
    initConnect();
    fmInfo() << "TagManagerDBus: Tag manager DBus service initialized successfully";
}

void TagManagerDBus::initConnect()
{
    fmDebug() << "TagManagerDBus: Setting up signal connections with TagDbHandler";
    connect(TagDbHandler::instance(), &TagDbHandler::newTagsAdded, this, &TagManagerDBus::NewTagsAdded);
    connect(TagDbHandler::instance(), &TagDbHandler::tagsDeleted, this, &TagManagerDBus::TagsDeleted);
    connect(TagDbHandler::instance(), &TagDbHandler::tagsColorChanged, this, &TagManagerDBus::TagsColorChanged);
    connect(TagDbHandler::instance(), &TagDbHandler::tagsNameChanged, this, &TagManagerDBus::TagsNameChanged);
    connect(TagDbHandler::instance(), &TagDbHandler::filesWereTagged, this, &TagManagerDBus::FilesTagged);
    connect(TagDbHandler::instance(), &TagDbHandler::filesUntagged, this, &TagManagerDBus::FilesUntagged);
    fmDebug() << "TagManagerDBus: Signal connections established successfully";
}

QDBusVariant TagManagerDBus::Query(int opt, const QStringList value)
{
    QDBusVariant dbusVar {};
    QueryOpts queryOpt { opt };

    fmDebug() << "TagManagerDBus::Query: Processing query request - option:" << opt << "value count:" << value.size();

    switch (queryOpt) {
    case QueryOpts::kTags:
        fmDebug() << "TagManagerDBus::Query: Retrieving all tags";
        dbusVar.setVariant(TagDbHandler::instance()->getAllTags());
        break;
    case QueryOpts::kFilesWithTags:
        fmDebug() << "TagManagerDBus::Query: Retrieving all files with tags";
        dbusVar.setVariant(TagDbHandler::instance()->getAllFileWithTags());
        break;
    case QueryOpts::kTagsOfFile:
        fmDebug() << "TagManagerDBus::Query: Retrieving tags for files - file count:" << value.size();
        dbusVar.setVariant(TagDbHandler::instance()->getTagsByUrls(value));
        break;
    case QueryOpts::kFilesOfTag:
        fmDebug() << "TagManagerDBus::Query: Retrieving files for tags - tag count:" << value.size();
        dbusVar.setVariant(TagDbHandler::instance()->getFilesByTag(value));
        break;
    case QueryOpts::kColorOfTags:
        fmDebug() << "TagManagerDBus::Query: Retrieving colors for tags - tag count:" << value.size();
        dbusVar.setVariant(TagDbHandler::instance()->getTagsColor(value));
        break;
    case QueryOpts::kTagIntersectionOfFiles:
        fmDebug() << "TagManagerDBus::Query: Retrieving tag intersection for files - file count:" << value.size();
        dbusVar.setVariant(TagDbHandler::instance()->getSameTagsOfDiffUrls(value));
        break;
    default:
        fmWarning() << "TagManagerDBus::Query: Unknown query option:" << opt;
        break;
    }

    fmDebug() << "TagManagerDBus::Query: Query completed successfully for option:" << opt;
    return dbusVar;
}

bool TagManagerDBus::Insert(int opt, const QVariantMap value)
{
    InsertOpts insetOpt { opt };
    bool result = false;

    fmInfo() << "TagManagerDBus::Insert: Processing insert request - option:" << opt << "data count:" << value.size();

    switch (insetOpt) {
    case InsertOpts::kTags:
        fmInfo() << "TagManagerDBus::Insert: Adding tag properties - tag count:" << value.size();
        result = TagDbHandler::instance()->addTagProperty(value);
        break;
    case InsertOpts::kTagOfFiles:
        fmInfo() << "TagManagerDBus::Insert: Adding tags for files - file count:" << value.size();
        result = TagDbHandler::instance()->addTagsForFiles(value);
        break;
    default:
        fmWarning() << "TagManagerDBus::Insert: Unknown insert option:" << opt;
        return false;
    }

    return false;
}

bool TagManagerDBus::Delete(int opt, const QVariantMap value)
{
    DeleteOpts deleteOpt { opt };
    bool result = false;

    fmInfo() << "TagManagerDBus::Delete: Processing delete request - option:" << opt << "data count:" << value.size();

    switch (deleteOpt) {
    case DeleteOpts::kTags:
        fmInfo() << "TagManagerDBus::Delete: Deleting tags - tag count:" << value.first().toStringList().size();
        result = TagDbHandler::instance()->deleteTags(value.first().toStringList());
        break;
    case DeleteOpts::kFiles:
        fmInfo() << "TagManagerDBus::Delete: Deleting files - file count:" << value.keys().size();
        result = TagDbHandler::instance()->deleteFiles(value.keys());
        break;
    case DeleteOpts::kTagOfFiles:
        fmInfo() << "TagManagerDBus::Delete: Removing tags from files - file count:" << value.size();
        result = TagDbHandler::instance()->removeTagsOfFiles(value);
        break;
    default:
        fmWarning() << "TagManagerDBus::Delete: Unknown delete option:" << opt;
        return false;
    }

    return false;
}

bool TagManagerDBus::Update(int opt, const QVariantMap value)
{
    UpdateOpts updateOpt { opt };
    bool result = false;

    fmInfo() << "TagManagerDBus::Update: Processing update request - option:" << opt << "data count:" << value.size();

    switch (updateOpt) {
    case UpdateOpts::kColors:
        fmInfo() << "TagManagerDBus::Update: Updating tag colors - tag count:" << value.size();
        result = TagDbHandler::instance()->changeTagColors(value);
        break;
    case UpdateOpts::kTagsNameWithFiles:
        fmInfo() << "TagManagerDBus::Update: Updating tag names with files - tag count:" << value.size();
        result = TagDbHandler::instance()->changeTagNamesWithFiles(value);
        break;
    case UpdateOpts::kFilesPaths:
        fmInfo() << "TagManagerDBus::Update: Updating file paths - file count:" << value.size();
        result = TagDbHandler::instance()->changeFilePaths(value);
        break;
    default:
        fmWarning() << "TagManagerDBus::Update: Unknown update option:" << opt;
        return false;
    }

    return false;
}
