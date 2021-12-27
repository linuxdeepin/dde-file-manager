/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FULLTEXTSEARCHER_P_H
#define FULLTEXTSEARCHER_P_H

#include "search/searcher/abstractsearcher.h"

#include <lucene++/LuceneHeaders.h>

#include <QMutex>
#include <QTime>

class FullTextSearcher;
class FullTextSearcherPrivate : public QObject
{
    Q_OBJECT
    friend class FullTextSearcher;

public:
    enum WordType {
        kCn,
        kEn,
        kDigit,
        kSymbol
    };

    enum TaskType {
        kCreate,
        kUpdate
    };

    enum IndexType {
        kAddIndex,
        kUpdateIndex,
        kDeleteIndex
    };
    Q_ENUM(IndexType)

    explicit FullTextSearcherPrivate(FullTextSearcher *parent);
    ~FullTextSearcherPrivate();

private:
    Lucene::IndexWriterPtr newIndexWriter(bool create = false);
    Lucene::IndexReaderPtr newIndexReader();

    bool createIndex(const QString &path);
    bool updateIndex(const QString &path);
    bool doSearch(const QString &path, const QString &keyword);

    Lucene::DocumentPtr fileDocument(const QString &file);
    QString dealKeyword(const QString &keyword);
    void doIndexTask(const Lucene::IndexReaderPtr &reader, const Lucene::IndexWriterPtr &writer, const QString &path, TaskType type);
    void indexDocs(const Lucene::IndexWriterPtr &writer, const QString &file, IndexType type);
    bool checkUpdate(const Lucene::IndexReaderPtr &reader, const QString &file, IndexType &type);
    void tryNotify();

    QString indexStorePath;
    bool isUpdated = false;
    QAtomicInt status = AbstractSearcher::kReady;
    QStringList allResults;
    mutable QMutex mutex;

    //计时
    QTime notifyTimer;
    int lastEmit = 0;

    FullTextSearcher *q = nullptr;
};

#endif   // FULLTEXTSEARCHER_P_H
