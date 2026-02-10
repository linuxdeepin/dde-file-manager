// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILETAGCACHE_H
#define FILETAGCACHE_H

#include <QObject>
#include <QThread>
#include <QSharedPointer>

#define FileTagCacheIns FileTagCacheController::instance()

namespace dfmplugin_tag {

class FileTagCachePrivate;
class FileTagCacheWorker : public QObject
{
    Q_OBJECT
    friend class FileTagCacheController;

public:
    ~FileTagCacheWorker() override;

public Q_SLOTS:
    void loadFileTagsFromDatabase();
    void onTagAdded(const QVariantMap &tags);
    void onTagDeleted(const QVariant &tags);
    void onTagsColorChanged(const QVariantMap &tagAndColorName);
    void onTagsNameChanged(const QVariantMap &oldAndNew);
    void onFilesTagged(const QVariantMap &fileAndTags);
    void onFilesUntagged(const QVariantMap &fileAndTags);
    void onTrashFileTagsChanged();

private:
    explicit FileTagCacheWorker(QObject *parent = nullptr);
};

class FileTagCache : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileTagCache)

    using TagColorMap = QMap<QString, QColor>;
    friend class FileTagCacheWorker;
    friend class FileTagCacheController;

public:
    virtual ~FileTagCache() override;

    //query
    QStringList getTagsByFiles(const QStringList &paths) const;
    TagColorMap getTagsColor(const QStringList &tags) const;
    QHash<QString, QStringList> findChildren(const QString &parentPath) const;

private:
    explicit FileTagCache(QObject *parent = nullptr);
    static FileTagCache &instance();
    void loadFileTagsFromDatabase();

    void addTags(const QVariantMap &tags);
    void deleteTags(const QStringList &tags);
    void changeTagColor(const QVariantMap &tagAndColorName);
    void changeTagName(const QVariantMap &oldAndNew);
    void changeFilesTagName(const QString &oldName, const QString &newName);
    void taggeFiles(const QVariantMap &fileAndTags);
    void untaggeFiles(const QVariantMap &fileAndTags);

    QStringList getTrashTags(const QString &path, qint64 inode) const;
    void reloadTrashFileTagsCache();

private:
    QScopedPointer<FileTagCachePrivate> d;
};

class FileTagCacheController : public QObject
{
    Q_DISABLE_COPY(FileTagCacheController)
    Q_OBJECT
public:
    virtual ~FileTagCacheController() override;
    static FileTagCacheController &instance();

    //query
    QStringList getTagsByFiles(const QStringList &paths);
    QStringList getTagsByFile(const QString &path);
    QMap<QString, QColor> getCacheTagsColor(const QStringList &tags);
    QHash<QString, QStringList> findChildren(const QString &parentPath) const;

    QStringList getTrashFileTags(const QString &path, qint64 inode);

Q_SIGNALS:
    void initLoadTagInfos();

    void filesTagged(const QVariantMap &fileAndTags);
    void filesUntagged(const QVariantMap &fileAndTags);
    void newTagsAdded(const QVariantMap &tags);
    void tagsColorChanged(const QVariantMap &oldAndNew);
    void tagsDeleted(const QStringList &tags);
    void tagsNameChanged(const QVariantMap &oldAndNew);

private:
    explicit FileTagCacheController(QObject *parent = nullptr);
    void init();

private:
    QSharedPointer<QThread> updateThread { nullptr };
    QSharedPointer<FileTagCacheWorker> cacheWorker { nullptr };
};
}

#endif   // FILETAGCACHE_H
