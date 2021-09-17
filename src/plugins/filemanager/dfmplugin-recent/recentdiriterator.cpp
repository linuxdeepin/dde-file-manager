#include "recentdiriterator.h"
#include "recentutil.h"

RecentDirIterator::RecentDirIterator(const QUrl &url,
                                     const QStringList &nameFilters,
                                     QDir::Filters filters,
                                     QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags)
{

}

QUrl RecentDirIterator::next()
{

}

bool RecentDirIterator::hasNext() const
{

}

QString RecentDirIterator::fileName() const
{

}

QUrl RecentDirIterator::fileUrl() const
{

}

const AbstractFileInfoPointer RecentDirIterator::fileInfo() const
{

}

QUrl RecentDirIterator::url() const
{
    return RecentUtil::onlyRootUrl();
}
