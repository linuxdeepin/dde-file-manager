#ifndef RECENTDIRITERATOR_H
#define RECENTDIRITERATOR_H

#include "dfm-base/base/abstractdiriterator.h"

DFMBASE_USE_NAMESPACE

class RecentDirIterator : public AbstractDirIterator
{
public:
    explicit RecentDirIterator(const QUrl &url,
                               const QStringList &nameFilters = QStringList(),
                               QDir::Filters filters = QDir::NoFilter,
                               QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    virtual QUrl next() override;

    virtual bool hasNext() const override;

    virtual QString fileName() const override;

    virtual QUrl fileUrl() const override;

    virtual const AbstractFileInfoPointer fileInfo() const override;

    virtual QUrl url() const override;
};

#endif // RECENTDIRITERATOR_H
