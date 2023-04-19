// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGDBHANDLER_H
#define TAGDBHANDLER_H

#include "serverplugin_tagdaemon_global.h"

#include <dfm-base/base/db/sqlitehandle.h>

#include <QObject>

SERVERTAGDAEMON_BEGIN_NAMESPACE

class TagDbHandler : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TagDbHandler)

public:
    enum SqlType {
        kNone = 0,
        kGetAllTags,
        kGetTagsThroughFile,
        kGetFilesThroughTag,
        kGetTagColor,
        kGetTagCount,
        kInsertTag
    };

    enum DbFileExistState {
        kExist = 0,
        kNoExist,
        kNoThisDir,
        kPlaceHolder,
        kFailedExecSql
    };

public:
    static TagDbHandler *instance();
    QVariantMap getAllTags();
    QVariantMap getTagsColor(const QStringList &tags);
    QVariantMap getTagsByUrls(const QStringList &urlList);
    QVariant getSameTagsOfDiffUrls(const QStringList &urlList);
    QVariantMap getFilesByTag(const QStringList &tags);
    QVariantHash getAllFileWithTags();

    bool addTagProperty(const QVariantMap &data);
    bool addTagsForFiles(const QVariantMap &data);
    bool removeTagsOfFiles(const QVariantMap &data);
    bool deleteTags(const QStringList &tags);
    bool deleteFiles(const QStringList &urls);
    bool changeTagColors(const QVariantMap &data);
    bool changeTagNamesWithFiles(const QVariantMap &data);
    bool changeFilePaths(const QVariantMap &data);

    QString lastError() const;

private:
    explicit TagDbHandler(QObject *parent = nullptr);
    void initialize();
    bool createTable(const QString &tableName);
    bool checkTag(const QString &tag);
    bool insertTagProperty(const QString &name, const QVariant &value);
    bool tagFile(const QString &file, const QVariant &tags);
    bool removeSpecifiedTagOfFile(const QString &url, const QVariant &val);
    bool changeTagColor(const QString &tagName, const QString &newTagColor);
    bool changeTagNameWithFile(const QString &tagName, const QString &newName);
    bool changeFilePath(const QString &oldPath, const QString &newPath);

Q_SIGNALS:
    void newTagsAdded(const QVariantMap &newTags);
    void tagsDeleted(const QStringList &beDeletedTags);
    void tagsColorChanged(const QVariantMap &oldAndNewColor);
    void tagsNameChanged(const QVariantMap &oldAndNewName);
    void filesWereTagged(const QVariantMap &filesWereTagged);
    void filesUntagged(const QVariantMap &delTagsOfFile);

private:
    QScopedPointer<DFMBASE_NAMESPACE::SqliteHandle> handle;
    QString lastErr;
};

SERVERTAGDAEMON_END_NAMESPACE

#endif   // TAGDBHANDLER_H
