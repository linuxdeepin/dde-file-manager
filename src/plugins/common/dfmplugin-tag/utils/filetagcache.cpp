#include "private/filetagcache_p.h"

#include "dfmplugin_tag_global.h"
#include "data/tagdbhandle.h"

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
}

void FileTagCacheWorker::onTagDeleted(const QVariant &tags)
{
    FileTagCache::instance().deletedTags(tags.toStringList());
}

void FileTagCacheWorker::onTagColorChanged(const QVariantMap &tagAndColorName)
{
    FileTagCache::instance().changedTagColor(tagAndColorName);
}

void FileTagCacheWorker::onTagNameChanged(const QVariantMap &oldAndNew)
{
    FileTagCache::instance().changedTagName(oldAndNew);
}

void FileTagCacheWorker::onFilesTagged(const QVariantMap &fileAndTags)
{
    FileTagCache::instance().taggedFiles(fileAndTags);
}

void FileTagCacheWorker::onFilesUntagged(const QVariantMap &fileAndTags)
{
    FileTagCache::instance().untaggedFiles(fileAndTags);
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
    // 加载数据库所有文件标记,和标记属性到缓存
    d->fileTagsCache = TagDbHandle::instance()->getAllFileWithTags();
    const auto &tagsColor = TagDbHandle::instance()->getAllTags();
    QVariantMap::const_iterator it = tagsColor.begin();
    for (; it != tagsColor.end(); ++it)
        d->tagProperty.insert(it.key(), QColor(it.value().toString()));
}

void FileTagCache::addTags(const QVariantMap &tags)
{
    QVariantMap::const_iterator it = tags.begin();
    for (; it != tags.end(); ++it) {
        if (d->tagProperty.contains(it.key()))
            continue;
        d->tagProperty.insert(it.key(), QColor(it.value().toString()));
    }
}

void FileTagCache::deletedTags(const QStringList &tags)
{
    for (const QString &tag : tags)
        d->tagProperty.remove(tag);
}

void FileTagCache::changedTagColor(const QVariantMap &tagAndColorName)
{
    QVariantMap::const_iterator it = tagAndColorName.begin();
    for (; it != tagAndColorName.end(); ++it) {
        if (d->tagProperty.contains(it.key()))
            d->tagProperty[it.key()] = QColor(it.value().toString());
    }
}

void FileTagCache::changedTagName(const QVariantMap &oldAndNew)
{
    QVariantMap::const_iterator it = oldAndNew.begin();
    for (; it != oldAndNew.end(); ++it) {
        if (d->tagProperty.contains(it.key())) {
            const auto &temp = d->tagProperty.value(it.key());
            d->tagProperty.remove(it.key());
            d->tagProperty.insert(it.value().toString(), temp);
        }
    }
}

void FileTagCache::taggedFiles(const QVariantMap &fileAndTags)
{
    QVariantMap::const_iterator it = fileAndTags.begin();
    for (; it != fileAndTags.end(); ++it) {
        if (!d->fileTagsCache.contains(it.key())) {
            d->fileTagsCache.insert(it.key(), it.value().toStringList());
        } else {
            const auto &lst = it.value().toStringList();
            auto cacheLst = d->fileTagsCache.value(it.key());
            for (const QString &tag : lst)
                if (!cacheLst.contains(tag))
                    cacheLst.append(tag);

            d->fileTagsCache[it.key()] = cacheLst;
        }
    }
}

void FileTagCache::untaggedFiles(const QVariantMap &fileAndTags)
{
    QVariantMap::const_iterator it = fileAndTags.begin();
    for (; it != fileAndTags.end(); ++it) {
        if (d->fileTagsCache.contains(it.key())) {
            const auto &lst = it.value().toStringList();
            auto cacheLst = d->fileTagsCache.value(it.key());
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

QStringList FileTagCache::getCacheFileTags(const QString &path)
{
    if (path.isEmpty())
        return {};

    QReadLocker wlk(&d->lock);
    return d->fileTagsCache.value(path);
}

FileTagCache::TagColorMap FileTagCache::getTagsColor(const QStringList &tags)
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

QStringList FileTagCacheController::getCacheFileTags(const QString &path)
{
    return FileTagCache::instance().getCacheFileTags(path);
}

QMap<QString, QColor> FileTagCacheController::getCacheTagsColor(const QStringList &tags)
{
    return FileTagCache::instance().getTagsColor(tags);
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
    connect(TagDbHandle::instance(), &TagDbHandle::addedNewTags, cacheWorker.data(), &FileTagCacheWorker::onTagAdded);
    connect(TagDbHandle::instance(), &TagDbHandle::deletedTags, cacheWorker.data(), &FileTagCacheWorker::onTagDeleted);
    connect(TagDbHandle::instance(), &TagDbHandle::changedTagColor, cacheWorker.data(), &FileTagCacheWorker::onTagColorChanged);
    connect(TagDbHandle::instance(), &TagDbHandle::changedTagName, cacheWorker.data(), &FileTagCacheWorker::onTagNameChanged);
    connect(TagDbHandle::instance(), &TagDbHandle::filesWereTagged, cacheWorker.data(), &FileTagCacheWorker::onFilesTagged);
    connect(TagDbHandle::instance(), &TagDbHandle::untagFiles, cacheWorker.data(), &FileTagCacheWorker::onFilesUntagged);

    cacheWorker->moveToThread(updateThread.data());
    updateThread->start();
}
