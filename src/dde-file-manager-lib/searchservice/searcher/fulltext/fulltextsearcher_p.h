// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FULLTEXTSEARCHER_P_H
#define FULLTEXTSEARCHER_P_H

#include "abstractsearcher.h"

#include <lucene++/LuceneHeaders.h>

#include <QStandardPaths>
#include <QApplication>
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
    inline static QString indexStorePath()
    {
        static QString path = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()
                + "/deepin/dde-file-manager/index";
        return path;
    }

    Lucene::DocumentPtr fileDocument(const QString &file);
    QString dealKeyword(const QString &keyword);
    void doIndexTask(const Lucene::IndexReaderPtr &reader, const Lucene::IndexWriterPtr &writer, const QString &path, TaskType type);
    void indexDocs(const Lucene::IndexWriterPtr &writer, const QString &file, IndexType type);
    bool checkUpdate(const Lucene::IndexReaderPtr &reader, const QString &file, IndexType &type);
    void tryNotify();

    bool isUpdated = false;
    QAtomicInt status = AbstractSearcher::kReady;
    QList<DUrl> allResults;
    mutable QMutex mutex;
    static bool isIndexCreating;

    //计时
    QTime notifyTimer;
    int lastEmit = 0;

    FullTextSearcher *q = nullptr;
};

#endif   // FULLTEXTSEARCHER_P_H
