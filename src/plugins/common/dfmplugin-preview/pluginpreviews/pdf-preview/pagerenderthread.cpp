/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#include "pagerenderthread.h"
#include "browserpage.h"
#include "sheetbrowser.h"
#include "docsheet.h"
#include "sheetrenderer.h"
#include "sidebarimageviewmodel.h"

#include <QTime>
#include <QDebug>
#include <QMetaType>
#include <QFileInfo>

PREVIEW_USE_NAMESPACE
PageRenderThread *PageRenderThread::pageRenderThread = nullptr;   //由于pdfium不支持多线程，暂时单线程进行

bool PageRenderThread::quitForever = false;

PageRenderThread::PageRenderThread(QObject *parent)
    : QThread(parent)
{
    qRegisterMetaType<Document *>("Document *");
    qRegisterMetaType<QList<Page *>>("QList<Page *>");
    qRegisterMetaType<Document::Error>("Document::Error");

    qRegisterMetaType<DocPageNormalImageTask>("DocPageNormalImageTask");
    qRegisterMetaType<DocPageSliceImageTask>("DocPageSliceImageTask");
    qRegisterMetaType<DocPageThumbnailTask>("DocPageThumbnailTask");
    qRegisterMetaType<DocOpenTask>("DocOpenTask");

    connect(this, &PageRenderThread::sigDocPageNormalImageTaskFinished, this, &PageRenderThread::onDocPageNormalImageTaskFinished, Qt::QueuedConnection);
    connect(this, &PageRenderThread::sigDocPageThumbnailTaskFinished, this, &PageRenderThread::onDocPageThumbnailTask, Qt::QueuedConnection);
    connect(this, &PageRenderThread::sigDocOpenTask, this, &PageRenderThread::onDocOpenTask, Qt::QueuedConnection);
}

PageRenderThread::~PageRenderThread()
{
    quitDoc = true;
    wait();
    if (isFinished())
        quitForever = false;
}

bool PageRenderThread::clearImageTasks(DocSheet *sheet, BrowserPage *page, int pixmapId)
{
    if (nullptr == page)
        return true;

    PageRenderThread *instance = PageRenderThread::instance();

    if (nullptr == instance) {
        return false;
    }

    instance->pageNormalImageMutex.lock();

    bool exist = true;

    while (exist) {
        exist = false;
        for (int i = 0; i < instance->pageNormalImageTasks.count(); ++i) {
            if (instance->pageNormalImageTasks[i].page == page && instance->pageNormalImageTasks[i].sheet == sheet && (instance->pageNormalImageTasks[i].pixmapId != pixmapId || -1 == pixmapId)) {
                instance->pageNormalImageTasks.removeAt(i);
                exist = true;
                break;
            }
        }
    }

    instance->pageNormalImageMutex.unlock();

    instance->pageSliceImageMutex.lock();

    exist = true;

    while (exist) {
        exist = false;
        for (int i = 0; i < instance->pageSliceImageTasks.count(); ++i) {
            if (instance->pageSliceImageTasks[i].page == page && instance->pageSliceImageTasks[i].sheet == sheet && (instance->pageSliceImageTasks[i].pixmapId != pixmapId || -1 == pixmapId)) {
                instance->pageSliceImageTasks.removeAt(i);
                exist = true;
                break;
            }
        }
    }

    instance->pageSliceImageMutex.unlock();

    return true;
}

void PageRenderThread::appendTask(DocPageNormalImageTask task)
{
    PageRenderThread *instance = PageRenderThread::instance();

    if (nullptr == instance) {
        return;
    }

    instance->pageNormalImageMutex.lock();

    instance->pageNormalImageTasks.append(task);

    instance->pageNormalImageMutex.unlock();

    if (!instance->isRunning())
        instance->start();
}

void PageRenderThread::appendTask(DocPageSliceImageTask task)
{
    PageRenderThread *instance = PageRenderThread::instance();

    if (nullptr == instance) {
        return;
    }

    instance->pageSliceImageMutex.lock();

    instance->pageSliceImageTasks.append(task);

    instance->pageSliceImageMutex.unlock();

    if (!instance->isRunning())
        instance->start();
}

void PageRenderThread::appendTask(DocPageThumbnailTask task)
{
    PageRenderThread *instance = PageRenderThread::instance();

    if (nullptr == instance) {
        return;
    }

    instance->pageThumbnailMutex.lock();

    instance->pageThumbnailTasks.append(task);

    instance->pageThumbnailMutex.unlock();

    if (!instance->isRunning())
        instance->start();
}

void PageRenderThread::appendTask(DocOpenTask task)
{
    PageRenderThread *instance = PageRenderThread::instance();

    if (nullptr == instance) {
        return;
    }

    instance->openMutex.lock();

    instance->openTasks.append(task);

    instance->openMutex.unlock();

    if (!instance->isRunning())
        instance->start();
}

void PageRenderThread::appendTask(DocCloseTask task)
{
    PageRenderThread *instance = PageRenderThread::instance();

    if (nullptr == instance) {
        return;
    }

    instance->closeMutex.lock();

    instance->closeTasks.append(task);

    instance->closeMutex.unlock();

    if (!instance->isRunning())
        instance->start();
}

void PageRenderThread::run()
{
    quitDoc = false;

    while (!quitDoc) {
        if (!hasNextTask()) {
            msleep(100);
            continue;
        }

        //! 先完成所有的关闭任务再进行打开
        while (execNextDocCloseTask()) {
        }

        while (execNextDocOpenTask()) {
        }

        while (execNextDocPageNormalImageTask()) {
        }

        while (execNextDocPageThumbnailTask()) {
        }

        if (quitDoc)
            break;
    }

    //! 处理关闭所有文档
    while (execNextDocCloseTask()) {
    }
}

bool PageRenderThread::hasNextTask()
{
    QMutexLocker pageNormalImageLocker(&pageNormalImageMutex);
    QMutexLocker pageThumbnailLocker(&pageThumbnailMutex);
    QMutexLocker pageOpenLocker(&openMutex);

    return !pageNormalImageTasks.isEmpty() || !pageThumbnailTasks.isEmpty()
            || !openTasks.isEmpty();
}

bool PageRenderThread::popNextDocPageNormalImageTask(DocPageNormalImageTask &task)
{
    QMutexLocker locker(&pageNormalImageMutex);

    if (pageNormalImageTasks.count() <= 0)
        return false;

    task = pageNormalImageTasks.value(0);

    pageNormalImageTasks.removeAt(0);

    return true;
}

bool PageRenderThread::popNextDocPageThumbnailTask(DocPageThumbnailTask &task)
{
    QMutexLocker locker(&pageThumbnailMutex);

    if (pageThumbnailTasks.count() <= 0)
        return false;

    task = pageThumbnailTasks.value(0);

    pageThumbnailTasks.removeAt(0);

    return true;
}

bool PageRenderThread::popNextDocOpenTask(DocOpenTask &task)
{
    QMutexLocker locker(&openMutex);

    if (openTasks.count() <= 0)
        return false;

    task = openTasks.value(0);

    openTasks.removeAt(0);

    return true;
}

bool PageRenderThread::popNextDocCloseTask(DocCloseTask &task)
{
    QMutexLocker locker(&closeMutex);

    if (closeTasks.count() <= 0)
        return false;

    task = closeTasks.value(0);

    closeTasks.removeAt(0);

    return true;
}

bool PageRenderThread::execNextDocPageNormalImageTask()
{
    if (quitDoc)
        return false;

    DocPageNormalImageTask task;

    if (!popNextDocPageNormalImageTask(task))
        return false;

    if (!DocSheet::existSheet(task.sheet))
        return true;

    QImage image = task.sheet->getImage(task.page->itemIndex(), task.rect.width(), task.rect.height());

    if (!image.isNull())
        emit sigDocPageNormalImageTaskFinished(task, QPixmap::fromImage(image));

    return true;
}

bool PageRenderThread::execNextDocPageThumbnailTask()
{
    if (quitDoc)
        return false;

    DocPageThumbnailTask task;

    if (!popNextDocPageThumbnailTask(task))
        return false;

    if (!DocSheet::existSheet(task.sheet))
        return true;

    QImage image = task.sheet->getImage(task.index, 174, 174);

    if (!image.isNull())
        emit sigDocPageThumbnailTaskFinished(task, QPixmap::fromImage(image));

    return true;
}

bool PageRenderThread::execNextDocOpenTask()
{
    if (quitDoc)
        return false;   //! false 为不用再继续循环调用

    DocOpenTask task;

    if (!popNextDocOpenTask(task))
        return false;   //! false 为不用再继续循环调用

    if (!DocSheet::existSheet(task.sheet))
        return true;

    QString filePath = task.sheet->filePath();

    Document::Error error = Document::NoError;

    Document *document = DocumentFactory::getDocument(task.sheet->fileType(), filePath, task.password, error);

    if (nullptr == document) {
        emit sigDocOpenTask(task, error, nullptr, QList<Page *>());

    } else {
        int pagesNumber = document->pageCount();

        QList<Page *> pages;

        for (int i = 0; i < pagesNumber; ++i) {
            Page *page = document->page(i);

            if (nullptr == page)
                break;

            pages.append(page);
        }

        if (pages.count() == pagesNumber) {
            emit sigDocOpenTask(task, Document::NoError, document, pages);

        } else {
            qDeleteAll(pages);

            pages.clear();

            delete document;

            emit sigDocOpenTask(task, Document::FileDamaged, nullptr, QList<Page *>());
        }
    }

    return true;
}

bool PageRenderThread::execNextDocCloseTask()
{
    DocCloseTask task;

    if (!popNextDocCloseTask(task))
        return false;   //! false 为不用再继续循环调用

    foreach (Page *p, task.pages)
        p->deleteLater();

    delete task.document;

    return true;
}

void PageRenderThread::onDocPageNormalImageTaskFinished(DocPageNormalImageTask task, QPixmap pixmap)
{
    if (DocSheet::existSheet(task.sheet)) {
        task.page->handleRenderFinished(task.pixmapId, pixmap);
    }
}

void PageRenderThread::onDocPageThumbnailTask(DocPageThumbnailTask task, QPixmap pixmap)
{
    if (DocSheet::existSheet(task.sheet)) {
        task.model->handleRenderThumbnail(task.index, pixmap);
    }
}

void PageRenderThread::onDocOpenTask(DocOpenTask task, Document::Error error, Document *document, QList<Page *> pages)
{
    if (DocSheet::existSheet(task.sheet)) {
        task.renderer->handleOpened(error, document, pages);
    }
}

void PageRenderThread::destroyForever()
{
    quitForever = true;

    if (nullptr != pageRenderThread) {
        delete pageRenderThread;
        pageRenderThread = nullptr;
    }
}

PageRenderThread *PageRenderThread::instance()
{
    if (quitForever)
        return nullptr;

    if (nullptr == pageRenderThread) {
        pageRenderThread = new PageRenderThread;
    }

    return pageRenderThread;
}
