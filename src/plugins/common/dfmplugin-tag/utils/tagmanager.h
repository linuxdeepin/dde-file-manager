// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGMANAGER_H
#define TAGMANAGER_H

#include <QObject>
#include <QPainter>
#include <QMap>

class TagDBus;

namespace dfmplugin_tag {

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
    bool pasteHandle(quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to);
    bool paintListTagsHandle(int role, const QUrl &url, QPainter *painter, QRectF *rect);
    bool paintIconTagsHandle(int role, const QUrl &url, QPainter *painter, QRectF *rect);
    bool fileDropHandle(const QList<QUrl> &fromUrls, const QUrl &toUrl);
    bool fileDropHandleWithAction(const QList<QUrl> &fromUrls, const QUrl &toUrl, Qt::DropAction *action);
    bool sepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup);
    bool registerTagColor(const QString &tagName, const QString &color);
    QString getTagIconName(const QString &tag) const;

    // qurey
    TagColorMap getAllTags();
    TagColorMap getTagsColor(const QStringList &tags) const;
    QVariant getTagsByUrls(const QList<QUrl> &filePaths, bool same = true) const;
    QStringList getFilesByTag(const QString &tag);

    // modify
    bool setTagsForFiles(const QStringList &tags, const QList<QUrl> &files);
    bool addTagsForFiles(const QList<QString> &tags, const QList<QUrl> &files);
    bool removeTagsOfFiles(const QList<QString> &tags, const QList<QUrl> &files);
    void deleteTags(const QStringList &tags);
    void deleteFiles(const QList<QUrl> &urls);
    bool changeTagColor(const QString &tagName, const QString &newTagColor);
    bool changeTagName(const QString &tagName, const QString &newName);

    static void contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);
    static void renameHandle(quint64 windowId, const QUrl &url, const QString &name);

    QMap<QString, QColor> assignColorToTags(const QStringList &tagList) const;

Q_SIGNALS:
    void tagDeleted(const QString &tagName);
    void filesTagged(const QMap<QString, QStringList> &fileAndTags);
    void filesUntagged(const QMap<QString, QStringList> &fileAndTags);

public Q_SLOTS:
    void onTagAdded(const QStringList &tags);
    void onTagDeleted(const QStringList &tags);
    void onTagColorChanged(const QMap<QString, QString> &tagAndColorName);
    void onTagNameChanged(const QMap<QString, QString> &oldAndNew);
    void onFilesTagged(const QMap<QString, QStringList> &fileAndTags);
    void onFilesUntagged(const QMap<QString, QStringList> &fileAndTags);

private:
    explicit TagManager(QObject *parent = nullptr);
    ~TagManager();
    void initializeConnection();

    QMap<QString, QString> getTagsColorName(const QStringList &tags) const;
    bool deleteTagData(const QStringList &data, const uint8_t &type);

private:
    QMap<QString, QString> tagColorMap;   // tag--color
    TagDBus *tagDbus { nullptr };
};

}

#endif   // TAGMANAGER_H
