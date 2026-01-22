// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGMANAGER_H
#define TAGMANAGER_H

#include "dfmplugin_tag_global.h"

#include <dfm-base/utils/elidetextlayout.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QObject>
#include <QPainter>
#include <QMap>
#include <QDBusVariant>

namespace dfmplugin_tag {

class TagPainter;
class TagManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TagManager)

    using TagColorMap = QMap<QString, QColor>;

public:
    static TagManager *instance();

    inline static QString scheme()
    {
        return "tag";
    }

    static QUrl rootUrl();

    bool canTagFile(const QUrl &url) const;
    bool canTagFile(const FileInfoPointer &info) const;
    bool pasteHandle(quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to);
    bool paintListTagsHandle(int role, const FileInfoPointer &info, QPainter *painter, QRectF *rect);
    bool addIconTagsHandle(const FileInfoPointer &info, dfmbase::ElideTextLayout *layout);
    bool fileDropHandle(const QList<QUrl> &fromUrls, const QUrl &toUrl);
    bool fileDropHandleWithAction(const QList<QUrl> &fromUrls, const QUrl &toUrl, Qt::DropAction *action);
    bool sepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup);
    bool registerTagColor(const QString &tagName, const QString &color);
    QString getTagIconName(const QString &tag) const;
    void hideFiles(const QList<QString> &tags, const QList<QUrl> &files);

    // qurey
    TagColorMap getAllTags();
    TagColorMap getTagsColor(const QStringList &tags) const;
    QStringList getTagsByUrls(const QList<QUrl> &urls) const;
    QStringList getFilesByTag(const QString &tag);
    QHash<QString, QStringList> findChildren(const QString &parentPath) const;

    // modify
    bool setTagsForFiles(const QStringList &tags, const QList<QUrl> &files);
    bool addTagsForFiles(const QList<QString> &tags, const QList<QUrl> &files);
    bool removeTagsOfFiles(const QList<QString> &tags, const QList<QUrl> &files);
    void deleteTags(const QStringList &tags);
    bool changeTagColor(const QString &tagName, const QString &newTagColor);
    bool changeTagName(const QString &tagName, const QString &newName);
    bool removeChildren(const QString &parentPath);

    // tag trash
    bool saveTrashFileTags(const QString &originalPath, qint64 fileInode, const QStringList &tagNames);
    QStringList getTrashFileTags(const QString &originalPath, qint64 fileInode);
    bool removeTrashFileTags(const QString &originalPath, qint64 fileInode);
    bool clearAllTrashTags();

    static void contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);
    static void renameHandle(quint64 windowId, const QUrl &url, const QString &name);

    QMap<QString, QColor> assignColorToTags(const QStringList &tagList) const;

Q_SIGNALS:
    void tagDeleted(const QString &tagName);
    void filesTagged(const QVariantMap &fileAndTags);
    void filesUntagged(const QVariantMap &fileAndTags);
    void filesHidden(const QVariantMap &fileAndTags);

public Q_SLOTS:
    void onTagAdded(const QVariantMap &tags);
    void onTagDeleted(const QStringList &tags);
    void onTagColorChanged(const QVariantMap &tagAndColorName);
    void onTagNameChanged(const QVariantMap &oldAndNew);
    void onFilesTagged(const QVariantMap &fileAndTags);
    void onFilesUntagged(const QVariantMap &fileAndTags);

private:
    explicit TagManager(QObject *parent = nullptr);
    ~TagManager();
    void initializeConnection();

    QMap<QString, QString> getTagsColorName(const QStringList &tags) const;
    bool deleteTagData(const QStringList &data, const DeleteOpts &type);
    bool localFileCanTagFilter(const FileInfoPointer &info) const;
    QVariant transformQueryData(const QDBusVariant &var) const;

private:
    int textObjectType;
    TagPainter *tagPainter;

    QMap<QString, QString> tagColorMap;   // tag--color
};

}

#endif   // TAGMANAGER_H
