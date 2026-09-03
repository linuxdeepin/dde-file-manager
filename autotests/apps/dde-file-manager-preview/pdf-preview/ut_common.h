// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PDF_PREVIEW_UT_COMMON_H
#define PDF_PREVIEW_UT_COMMON_H

#include "pagerenderthread.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QTemporaryDir>

#include <functional>

namespace ut_utils {

inline const char *kMultiPagePdfSrc = "/usr/share/doc/shared-mime-info/shared-mime-info-spec.pdf";
inline const char *kSinglePagePdfSrc = "/usr/share/cups/data/default.pdf";

inline QString prepareFile(const QString &source)
{
    static QTemporaryDir dir;
    QString dst = dir.path() + "/" + QFileInfo(source).fileName();
    if (!QFile::exists(dst))
        QFile::copy(source, dst);
    return dst;
}

inline void processEventsUntil(const std::function<bool()> &cond, int timeoutMs = 5000)
{
    QElapsedTimer timer;
    timer.start();
    while (!cond()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        if (timer.elapsed() > timeoutMs)
            break;
    }
}

inline void drainEvents(int ms = 200)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < ms)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

inline void waitRenderIdle(int extraMs = 300)
{
    plugin_filepreview::PageRenderThread *thread = plugin_filepreview::PageRenderThread::instance();
    if (thread) {
        processEventsUntil([&thread]() {
            {
                QMutexLocker closeLocker(&thread->closeMutex);
                if (!thread->closeTasks.isEmpty()) {
                    QMutexLocker normalLocker(&thread->pageNormalImageMutex);
                    if (thread->pageNormalImageTasks.isEmpty())
                        thread->pageNormalImageTasks.append(plugin_filepreview::DocPageNormalImageTask());
                }
            }
            QMutexLocker normalLocker(&thread->pageNormalImageMutex);
            QMutexLocker thumbLocker(&thread->pageThumbnailMutex);
            QMutexLocker openLocker(&thread->openMutex);
            QMutexLocker closeLocker(&thread->closeMutex);
            return thread->pageNormalImageTasks.isEmpty()
                    && thread->pageThumbnailTasks.isEmpty()
                    && thread->openTasks.isEmpty()
                    && thread->closeTasks.isEmpty();
        },
                           8000);
        QMutexLocker sliceLocker(&thread->pageSliceImageMutex);
        thread->pageSliceImageTasks.clear();
    }
    drainEvents(extraMs);
}

}

#endif
