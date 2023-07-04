// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILETAGCACHE_H
#define FILETAGCACHE_H

#include <QObject>
#include <QThread>
#include <QSharedPointer>

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

Q_SIGNALS:
    void initLoadTagInfos();

private:
    explicit FileTagCacheController(QObject *parent = nullptr);
    void init();

private:
    QSharedPointer<QThread> updateThread { nullptr };
    QSharedPointer<FileTagCacheWorker> cacheWorker { nullptr };
};
}

#endif   // FILETAGCACHE_H
