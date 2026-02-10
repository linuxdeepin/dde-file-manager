// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHDIRITERATOR_H
#define SEARCHDIRITERATOR_H

#include "dfmplugin_search_global.h"

#include <dfm-base/interfaces/abstractdiriterator.h>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_search {

class SearchDirIteratorPrivate;
class SearchDirIterator : public AbstractDirIterator
{
    Q_OBJECT
    friend class SearchDirIteratorPrivate;

public:
    explicit SearchDirIterator(const QUrl &url,
                               const QStringList &nameFilters = QStringList(),
                               QDir::Filters filters = QDir::NoFilter,
                               QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);
    ~SearchDirIterator() override;
    virtual QUrl next() override;
    virtual bool hasNext() const override;
    virtual QString fileName() const override;
    virtual QUrl fileUrl() const override;
    virtual const FileInfoPointer fileInfo() const override;
    virtual QUrl url() const override;
    virtual void close() override;
    virtual QList<QSharedPointer<SortFileInfo>> sortFileInfoList() override;
    virtual bool oneByOne() override { return false; }
    virtual bool isWaitingForUpdates() const override;

signals:
    void sigSearch() const;
    void sigStopSearch() const;

private:
    SearchDirIteratorPrivate *const d { nullptr };
    void doCompleteSortInfo(SortInfoPointer sortInfo);
};

}

#endif   // SEARCHDIRITERATOR_H
