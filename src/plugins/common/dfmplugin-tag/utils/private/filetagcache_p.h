#ifndef FILETAGCACHE_P_H
#define FILETAGCACHE_P_H

#include "utils/filetagcache.h"
#include <QReadWriteLock>
#include <QMutex>

namespace dfmplugin_tag {
class FileTagCachePrivate
{
    friend class FileTagCache;
    FileTagCache *const q;

    QHash<QString, QStringList> fileTagsCache;   // file path ->  tag name list
    QHash<QString, QColor> tagProperty;   // tag name -> QColor
    QReadWriteLock lock;

public:
    explicit FileTagCachePrivate(FileTagCache *qq);
    virtual ~FileTagCachePrivate();
};
}

#endif   // FILETAGCACHE_P_H
