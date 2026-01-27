// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/filetagcache_p.h"

#include "dfmplugin_tag_global.h"
#include "data/tagproxyhandle.h"

#include <QVariant>
#include <QColor>
#include <QDebug>
#include <QSet>

DPTAG_USE_NAMESPACE

FileTagCacheWorker::FileTagCacheWorker(QObject *parent)
    : QObject(parent)
{
}

FileTagCacheWorker::~FileTagCacheWorker()
{
}

void FileTagCacheWorker::loadFileTagsFromDatabase()
{
    FileTagCache::instance().loadFileTagsFromDatabase();
}

void FileTagCacheWorker::onTagAdded(const QVariantMap &tags)
{
    FileTagCache::instance().addTags(tags);
    emit FileTagCacheIns.newTagsAdded(tags);
}

void FileTagCacheWorker::onTagDeleted(const QVariant &tags)
{
    FileTagCache::instance().deleteTags(tags.toStringList());
    emit FileTagCacheIns.tagsDeleted(tags.toStringList());
}

void FileTagCacheWorker::onTagsColorChanged(const QVariantMap &tagAndColorName)
{
    FileTagCache::instance().changeTagColor(tagAndColorName);
    emit FileTagCacheIns.tagsColorChanged(tagAndColorName);
}

void FileTagCacheWorker::onTagsNameChanged(const QVariantMap &oldAndNew)
{
    FileTagCache::instance().changeTagName(oldAndNew);
    auto &&map { oldAndNew.toStdMap() };
    for (auto [oldName, newName] : map)
        FileTagCache::instance().changeFilesTagName(oldName, newName.toString());
    emit FileTagCacheIns.tagsNameChanged(oldAndNew);
}

void FileTagCacheWorker::onFilesTagged(const QVariantMap &fileAndTags)
{
    FileTagCache::instance().taggeFiles(fileAndTags);
    emit FileTagCacheIns.filesTagged(fileAndTags);
}

void FileTagCacheWorker::onFilesUntagged(const QVariantMap &fileAndTags)
{
    FileTagCache::instance().untaggeFiles(fileAndTags);
    emit FileTagCacheIns.filesUntagged(fileAndTags);
}

void FileTagCacheWorker::onTrashFileTagsChanged()
{
    FileTagCache::instance().reloadTrashFileTagsCache();
}

FileTagCachePrivate::FileTagCachePrivate(FileTagCache *qq)
    : q(qq)
{
}

FileTagCachePrivate::~FileTagCachePrivate()
{
}

FileTagCache::FileTagCache(QObject *parent)
    : QObject(parent), d(new FileTagCachePrivate(this))
{
}

FileTagCache &FileTagCache::instance()
{
    static FileTagCache cache;
    return cache;
}

void FileTagCache::loadFileTagsFromDatabase()
{
    fmInfo() << "Start initilize FileTagCache";
    // 加载数据库所有文件标记,和标记属性到缓存
    if (!TagProxyHandle::instance()->isValid())
        fmWarning() << "tagService is inValid";
    d->fileTagsCache = TagProxyHandle::instance()->getAllFileWithTags();
    const auto &tagsColor = TagProxyHandle::instance()->getAllTags();
    auto it = tagsColor.begin();
    for (; it != tagsColor.end(); ++it)
        d->tagProperty.insert(it.key(), QColor(it.value().toString()));

    // 加载回收站标记数据
    d->trashFileTagsCache = TagProxyHandle::instance()->getAllTrashFileTags();
}

void FileTagCache::addTags(const QVariantMap &tags)
{
    auto it = tags.begin();
    for (; it != tags.end(); ++it) {
        if (d->tagProperty.contains(it.key()))
            continue;
        d->tagProperty.insert(it.key(), QColor(it.value().toString()));
    }
}

void FileTagCache::deleteTags(const QStringList &tags)
{
    QVariantMap map {};
    for (const QString &tag : tags) {
        d->tagProperty.remove(tag);

        auto iter = d->fileTagsCache.begin();
        while (iter != d->fileTagsCache.end()) {
            if (iter.value().toStringList().contains(tag)) {
                QStringList fileTags = map[iter.key()].toStringList();
                fileTags.append(tag);
                map[iter.key()] = fileTags;
            }

            ++iter;
        }
    }

    if (!map.isEmpty())
        untaggeFiles(map);
}

void FileTagCache::changeTagColor(const QVariantMap &tagAndColorName)
{
    auto it = tagAndColorName.begin();
    for (; it != tagAndColorName.end(); ++it) {
        if (d->tagProperty.contains(it.key()))
            d->tagProperty[it.key()] = QColor(it.value().toString());
    }
}

void FileTagCache::changeTagName(const QVariantMap &oldAndNew)
{
    auto it = oldAndNew.begin();
    for (; it != oldAndNew.end(); ++it) {
        const QString &oldName { it.key() };
        const QString &newName { it.value().toString() };
        if (d->tagProperty.contains(oldName)) {
            const auto &color { d->tagProperty.value(oldName) };
            d->tagProperty.remove(oldName);
            d->tagProperty.insert(newName, color);
        }
    }
}

void FileTagCache::changeFilesTagName(const QString &oldName, const QString &newName)
{
    std::for_each(d->fileTagsCache.begin(), d->fileTagsCache.end(), [oldName, newName](QVariant &var) {
        QStringList tagNames { var.toStringList() };
        auto result { std::find(tagNames.begin(), tagNames.end(), oldName) };
        if (result != tagNames.end()) {
            int index { result - tagNames.begin() };
            tagNames.replace(index, newName);
            var.setValue(tagNames);
        }
    });
}

void FileTagCache::taggeFiles(const QVariantMap &fileAndTags)
{
    auto it = fileAndTags.begin();
    for (; it != fileAndTags.end(); ++it) {
        if (!d->fileTagsCache.contains(it.key())) {
            d->fileTagsCache.insert(it.key(), it.value().toStringList());
        } else {
            const auto &lst = it.value().toStringList();
            auto cacheLst = d->fileTagsCache.value(it.key()).toStringList();
            for (const QString &tag : lst)
                if (!cacheLst.contains(tag))
                    cacheLst.append(tag);

            d->fileTagsCache[it.key()] = cacheLst;
        }
    }
}

QStringList FileTagCache::getTrashTags(const QString &path, qint64 inode) const
{
    QReadLocker locker(&d->lock);
    QString key = QString("%1:%2").arg(path).arg(inode);
    return d->trashFileTagsCache.value(key).toStringList();
}

void FileTagCache::reloadTrashFileTagsCache()
{
    QWriteLocker locker(&d->lock);
    d->trashFileTagsCache = TagProxyHandle::instance()->getAllTrashFileTags();
}

void FileTagCache::untaggeFiles(const QVariantMap &fileAndTags)
{
    auto it = fileAndTags.begin();
    for (; it != fileAndTags.end(); ++it) {
        if (d->fileTagsCache.contains(it.key())) {
            const auto &lst = it.value().toStringList();
            auto cacheLst = d->fileTagsCache.value(it.key()).toStringList();
            for (const QString &tag : lst)
                if (cacheLst.contains(tag))
                    cacheLst.removeOne(tag);

            if (cacheLst.isEmpty()) {
                d->fileTagsCache.remove(it.key());
            } else {
                d->fileTagsCache[it.key()] = cacheLst;
            }
        }
    }
}

FileTagCache::~FileTagCache()
{
}

/**
 * @brief A single file gets its own tags, and multiple files are their intersection
 * @param paths is a collection of file paths
 * @return QStringList intersectionTags
 */
QStringList FileTagCache::getTagsByFiles(const QStringList &paths) const
{
    if (paths.isEmpty())
        return {};

    QReadLocker wlk(&d->lock);
    QStringList intersectionTags = d->fileTagsCache.value(paths.first()).toStringList();

    for (const QString &path : paths) {
        QStringList tags = d->fileTagsCache.value(path).toStringList();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        intersectionTags = intersectionTags.toSet().intersect(tags.toSet()).values();
#else
        auto intersectionSet = QSet<QString> { intersectionTags.begin(), intersectionTags.end() };
        auto tagsSet = QSet<QString> { tags.begin(), tags.end() };
        intersectionTags = intersectionSet.intersect(tagsSet).values();
#endif
        if (intersectionTags.isEmpty())
            break;
    }
    return intersectionTags;
}

QHash<QString, QStringList> FileTagCache::findChildren(const QString &parentPath) const
{
    QHash<QString, QStringList> children;
    QString normalizedParent = parentPath;
    if (!normalizedParent.endsWith('/'))
        normalizedParent += '/';

    QReadLocker wlk(&d->lock);
    for (auto it = d->fileTagsCache.cbegin(); it != d->fileTagsCache.cend(); ++it) {
        const QString &filePath = it.key();
        if (filePath.startsWith(normalizedParent))
            children.insert(filePath, it.value().toStringList());
    }

    return children;
}

FileTagCache::TagColorMap FileTagCache::getTagsColor(const QStringList &tags) const
{
    if (tags.isEmpty())
        return {};

    QReadLocker wlk(&d->lock);
    TagColorMap tagsColor;
    for (const auto &tag : tags) {
        if (d->tagProperty.contains(tag))
            tagsColor.insert(tag, d->tagProperty.value(tag));
    }

    return tagsColor;
}

FileTagCacheController &FileTagCacheController::instance()
{
    static FileTagCacheController cacheController;
    return cacheController;
}

QStringList FileTagCacheController::getTagsByFiles(const QStringList &paths)
{
    return FileTagCache::instance().getTagsByFiles(paths);
}

QStringList FileTagCacheController::getTagsByFile(const QString &path)
{
    return FileTagCache::instance().getTagsByFiles({ path });
}

QMap<QString, QColor> FileTagCacheController::getCacheTagsColor(const QStringList &tags)
{
    return FileTagCache::instance().getTagsColor(tags);
}

QHash<QString, QStringList> FileTagCacheController::findChildren(const QString &parentPath) const
{
    return FileTagCache::instance().findChildren(parentPath);
}

QStringList FileTagCacheController::getTrashFileTags(const QString &path, qint64 inode)
{
    return FileTagCache::instance().getTrashTags(path, inode);
}

FileTagCacheController::~FileTagCacheController()
{
    updateThread->quit();
    updateThread->wait();
}

FileTagCacheController::FileTagCacheController(QObject *parent)
    : QObject(parent), updateThread(new QThread), cacheWorker(new FileTagCacheWorker)
{
    init();
}

void FileTagCacheController::init()
{
    connect(this, &FileTagCacheController::initLoadTagInfos, cacheWorker.data(), &FileTagCacheWorker::loadFileTagsFromDatabase);
    connect(TagProxyHandleIns, &TagProxyHandle::newTagsAdded, cacheWorker.data(), &FileTagCacheWorker::onTagAdded);
    connect(TagProxyHandleIns, &TagProxyHandle::tagsDeleted, cacheWorker.data(), &FileTagCacheWorker::onTagDeleted);
    connect(TagProxyHandleIns, &TagProxyHandle::tagsColorChanged, cacheWorker.data(), &FileTagCacheWorker::onTagsColorChanged);
    connect(TagProxyHandleIns, &TagProxyHandle::tagsNameChanged, cacheWorker.data(), &FileTagCacheWorker::onTagsNameChanged);
    connect(TagProxyHandleIns, &TagProxyHandle::filesTagged, cacheWorker.data(), &FileTagCacheWorker::onFilesTagged);
    connect(TagProxyHandleIns, &TagProxyHandle::filesUntagged, cacheWorker.data(), &FileTagCacheWorker::onFilesUntagged);
    connect(TagProxyHandleIns, &TagProxyHandle::trashFileTagsChanged, cacheWorker.data(), &FileTagCacheWorker::onTrashFileTagsChanged);

    cacheWorker->moveToThread(updateThread.data());
    updateThread->start();
}
