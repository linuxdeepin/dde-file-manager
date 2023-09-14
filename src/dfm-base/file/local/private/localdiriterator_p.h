// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCALFILEDIRITERATOR_P_H
#define LOCALFILEDIRITERATOR_P_H

#include "file/local/localdiriterator.h"
#include <dfm-base/base/urlroute.h>

#include <QDirIterator>
#include <QPointer>
#include <QDebug>

#include <dfm-io/denumerator.h>

USING_IO_NAMESPACE
namespace dfmbase {
class LocalDirIterator;
class LocalDirIteratorPrivate : public QObject
{
    friend class LocalDirIterator;
    class LocalDirIterator *const q;

public:
    struct InitQuerierAsyncOp
    {
        QPointer<LocalDirIteratorPrivate> me;
        QUrl url;
    };

    explicit LocalDirIteratorPrivate(const QUrl &url,
                                     const QStringList &nameFilters,
                                     QDir::Filters filters,
                                     QDirIterator::IteratorFlags flags,
                                     LocalDirIterator *q);
    virtual ~LocalDirIteratorPrivate() override;

    FileInfoPointer fileInfo();
    FileInfoPointer fileInfo(const QSharedPointer<DFileInfo> dfmInfo);
    QList<FileInfoPointer> fileInfos();

private:
    QSharedPointer<dfmio::DEnumerator> dfmioDirIterator = nullptr;   // dfmio的文件迭代器
    QUrl currentUrl;   // 当前迭代器所在位置文件的url
    QSet<QString> hideFileList;
    bool isLocalDevice = false;
    bool isCdRomDevice = false;
};
}
#endif   // ABSTRACTDIRITERATOR_P_H
