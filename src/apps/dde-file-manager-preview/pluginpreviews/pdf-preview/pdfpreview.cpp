// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/utils/fileutils.h>

#include "pdfpreview.h"
#include "pdfwidget.h"
#include "global.h"

#include <QMimeType>
#include <QUrl>
#include <QFileInfo>

using namespace plugin_filepreview;
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
    pdfWidget->setFixedSize(kPdfWidgetWidth, kPdfWidgetHeight);
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
    return false;
}

void PDFPreview::initialize(QWidget *window, QWidget *statusBar)
{
    Q_UNUSED(statusBar);
    setMainWidget(window);
}
