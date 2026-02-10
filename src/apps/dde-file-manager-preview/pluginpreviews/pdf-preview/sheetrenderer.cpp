// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sheetrenderer.h"
#include "pagerenderthread.h"

#include <QEventLoop>
#include <QDebug>

using namespace plugin_filepreview;
SheetRenderer::SheetRenderer(DocSheet *parent)
    : QObject(parent), docSheet(parent)
{
}

SheetRenderer::~SheetRenderer()
{
    DocCloseTask task;

    task.document = documentObj;

    task.pages = pageList;

    PageRenderThread::appendTask(task);
}

bool SheetRenderer::openFileExec(const QString &password)
{
    QEventLoop loop;

    connect(this, &SheetRenderer::sigOpened, &loop, &QEventLoop::quit);

    openFileAsync(password);

    loop.exec();

    return Document::kNoError == docError;
}

void SheetRenderer::openFileAsync(const QString &password)
{
    DocOpenTask task;

    task.sheet = docSheet;

    task.password = password;

    task.renderer = this;

    PageRenderThread::appendTask(task);
}

bool SheetRenderer::opened()
{
    return documentObj != nullptr;
}

int SheetRenderer::getPageCount()
{
    return pageList.count();
}

QImage SheetRenderer::getImage(int index, int width, int height, const QRect &slice)
{
    if (pageList.count() <= index)
        return QImage();

    QImage image = pageList.value(index)->render(width, height, slice);

    return image;
}

QSizeF SheetRenderer::getPageSize(int index) const
{
    if (pageList.count() <= index)
        return QSizeF();

    return pageList.value(index)->sizeF();
}

void SheetRenderer::handleOpened(Document::Error error, Document *document, QList<Page *> pages)
{
    docError = error;

    documentObj = document;

    pageList = pages;

    emit sigOpened(error);
}
