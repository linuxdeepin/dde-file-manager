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

#include "pdfpreview.h"
#include "pdfwidget.h"
#include "global.h"

#include <QMimeType>
#include <QMimeDatabase>
#include <QUrl>
#include <QFileInfo>

PREVIEW_USE_NAMESPACE
PDFPreview::PDFPreview(QObject *parent)
    : AbstractBasePreview(parent)
{
}

PDFPreview::~PDFPreview()
{
    if (pdfWidget) {
        pdfWidget->deleteLater();
    }
}

bool PDFPreview::setFileUrl(const QUrl &url)
{
    if (selectFileUrl == url)
        return true;

    if (!url.isLocalFile())
        return false;

    if (pdfWidget == nullptr)
        pdfWidget = new PdfWidget();
    pdfWidget->setFixedSize(800, 500);
    pdfWidget->addFileAsync(url.toLocalFile());
    pageTitle = QFileInfo(url.toLocalFile()).fileName();

    Q_EMIT titleChanged();

    return true;
}

QUrl PDFPreview::fileUrl() const
{
    return selectFileUrl;
}

QWidget *PDFPreview::contentWidget() const
{
    return pdfWidget;
}

QString PDFPreview::title() const
{
    return pageTitle;
}

bool PDFPreview::showStatusBarSeparator() const
{
    return true;
}

void PDFPreview::initialize(QWidget *window, QWidget *statusBar)
{
    Q_UNUSED(statusBar);
    setMainWidget(window);
}
