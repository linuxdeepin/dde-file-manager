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

    return Document::NoError == docError;
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
