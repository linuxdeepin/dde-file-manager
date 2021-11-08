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
    return QUrl();
}

bool RecentDirIterator::hasNext() const
{
    return false;
}

QString RecentDirIterator::fileName() const
{
    return "";
}

QUrl RecentDirIterator::fileUrl() const
{
    return QUrl();
}

const AbstractFileInfoPointer RecentDirIterator::fileInfo() const
{
    return {};
}

QUrl RecentDirIterator::url() const
{
    return RecentUtil::onlyRootUrl();
}
