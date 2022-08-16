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
#ifndef TAGMANAGER_H
#define TAGMANAGER_H

#include "dfmplugin_tag_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QObject>

namespace dfmplugin_tag {

class TagDBusHelper;
class TagManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TagManager)

    using TagsMap = QMap<QString, QColor>;

public:
    static TagManager *instance();

    inline static QString scheme()
    {
        return "tag";
    }

    static QUrl rootUrl();

    // query
    TagsMap getAllTags();
    TagsMap getTagsColor(const QList<QString> &tags) const;

    QStringList getTagsByUrls(const QList<QUrl> &urlList) const;

    QString getTagColorName(const QString &tag) const;
    QString getTagIconName(const QString &tag) const;
    QString getTagIconName(const QColor &color) const;

    QStringList getFilesByTag(const QString &tag);

    // modify
    bool setTagsForFiles(const QList<QString> &tags, const QList<QUrl> &files);

    bool addTagsForFiles(const QList<QString> &tags, const QList<QUrl> &files);
    bool removeTagsOfFiles(const QList<QString> &tags, const QList<QUrl> &files);
    bool changeTagColor(const QString &tagName, const QString &newTagColor);
    bool changeTagName(const QString &tagName, const QString &newName);
    bool addTagsForFilesByColor(const QString &color, const QList<QUrl> &files);
    void deleteTags(const QList<QString> &tags);
    void deleteFiles(const QList<QUrl> &urls);

    bool canTagFile(const AbstractFileInfoPointer &fileInfo) const;
    bool paintListTagsHandle(int role, const QUrl &url, QPainter *painter, QRectF *rect);
    bool paintIconTagsHandle(int role, const QUrl &url, QPainter *painter, QRectF *rect);
    bool pasteHandle(quint64 winId, const QUrl &to);
    bool fileDropHandle(const QList<QUrl> &fromUrls, const QUrl &toUrl);
    bool fileDropHandleWithAction(const QList<QUrl> &fromUrls, const QUrl &toUrl, Qt::DropAction action);
    bool sepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup);

    static void contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);
    static void renameHandle(quint64 windowId, const QUrl &url, const QString &name);

public slots:
    void onTagAdded(const QStringList &tags);
    void onTagDeleted(const QStringList &tags);
    void onTagColorChanged(const QMap<QString, QString> &tagAndColorName);
    void onTagNameChanged(const QMap<QString, QString> &oldAndNew);
    void onFilesTagged(const QMap<QString, QList<QString>> &fileAndTags);
    void onFilesUntagged(const QMap<QString, QList<QString>> &fileAndTags);

signals:
    void tagDeleted(const QString &tagName);
    void filesTagged(const QMap<QString, QList<QString>> &fileAndTags);
    void filesUntagged(const QMap<QString, QList<QString>> &fileAndTags);

private:
    explicit TagManager(QObject *parent = nullptr);
    void initializeConnection();

    QMap<QString, QString> getTagsColorName(const QStringList &tags) const;
    QList<QColor> getTagColors(const QList<QUrl> &urls) const;

    TagDBusHelper *dbusHelper;
};

}

#endif   // TAGMANAGER_H
