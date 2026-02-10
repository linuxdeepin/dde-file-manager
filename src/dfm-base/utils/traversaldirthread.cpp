// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "traversaldirthread.h"
#include <dfm-base/base/schemefactory.h>

#include <QElapsedTimer>
#include <QDebug>

using namespace dfmbase;
USING_IO_NAMESPACE

TraversalDirThread::TraversalDirThread(const QUrl &url,
                                       const QStringList &nameFilters,
                                       QDir::Filters filters,
                                       QDirIterator::IteratorFlags flags,
                                       QObject *parent)
    : QThread(parent), dirUrl(url), nameFilters(nameFilters), filters(filters), flags(flags)
{
    if (dirUrl.isValid()) {
        auto path = url.path();
        if (path != "/" && path != QDir::separator() && path.endsWith(QDir::separator()))
            path = path.left(path.length() - 1);
        dirUrl.setPath(path);
        dirIterator = DirIteratorFactory::create<AbstractDirIterator>(dirUrl, nameFilters, filters, flags);
        if (!dirIterator) {
            qCWarning(logDFMBase) << "Failed to create directory iterator for URL:" << url;
            return;
        }
    }
}

TraversalDirThread::~TraversalDirThread()
{
    quit();
    wait();
}

void TraversalDirThread::stop()
{
    if (stopFlag)
        return;

    stopFlag = true;
    if (dirIterator)
        dirIterator->close();
}

void TraversalDirThread::quit()
{
    stop();
    QThread::quit();
}

void TraversalDirThread::stopAndDeleteLater()
{
    stop();

    if (!isRunning()) {
        deleteLater();
    } else {
        disconnect(this, &TraversalDirThread::finished, this, &TraversalDirThread::deleteLater);
        connect(this, &TraversalDirThread::finished, this, &TraversalDirThread::deleteLater);
    }
}

void TraversalDirThread::setQueryAttributes(const QString &fileAttributes)
{
    if (fileInfoQueryAttributes == fileAttributes)
        return;
    fileInfoQueryAttributes = fileAttributes;
    if (fileInfoQueryAttributes.isEmpty() && fileInfoQueryAttributes != "*")
        dirIterator->setProperty("QueryAttributes", fileInfoQueryAttributes);
}

void TraversalDirThread::run()
{
    if (dirIterator.isNull())
        return;

    QElapsedTimer timer;
    timer.start();

    qCInfo(logDFMBase) << "Directory traversal started for URL:" << dirUrl;
    dirIterator->cacheBlockIOAttribute();
    qCInfo(logDFMBase) << "Directory IO attributes cached for URL:" << dirUrl << "elapsed:" << timer.elapsed() << "ms";

    if (stopFlag)
        return;

    while (dirIterator->hasNext()) {
        if (stopFlag)
            break;

        // 调用一次fileinfo进行文件缓存
        const auto &fileUrl = dirIterator->next();
        if (!fileUrl.isValid())
            continue;

        emit updateChild(fileUrl);
        childrenList.append(fileUrl);
    }
    stopFlag = true;
    emit updateChildren(childrenList);

    qCInfo(logDFMBase) << "Directory traversal completed - file count:" << childrenList.size() 
                      << "URL:" << dirUrl << "total elapsed:" << timer.elapsed() << "ms";
}
