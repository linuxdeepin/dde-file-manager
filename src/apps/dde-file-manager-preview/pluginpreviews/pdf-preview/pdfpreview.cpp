// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
    fmInfo() << "PDF preview: PDFPreview instance created";
}

PDFPreview::~PDFPreview()
{
    fmInfo() << "PDF preview: PDFPreview instance destroyed";
    if (pdfWidget) {
        pdfWidget->deleteLater();
    }
}

bool PDFPreview::setFileUrl(const QUrl &url)
{
    fmInfo() << "PDF preview: setting file URL:" << url;
    
    if (selectFileUrl == url) {
        fmDebug() << "PDF preview: URL unchanged, skipping:" << url;
        return true;
    }

    if (!url.isLocalFile()) {
        fmWarning() << "PDF preview: URL is not a local file:" << url;
        return false;
    }

    const QString filePath = url.toLocalFile();
    if (!QFileInfo::exists(filePath)) {
        fmWarning() << "PDF preview: file does not exist:" << filePath;
        return false;
    }

    if (pdfWidget == nullptr) {
        fmDebug() << "PDF preview: creating new PdfWidget";
        pdfWidget = new PdfWidget();
    }
    
    pdfWidget->setFixedSize(kPdfWidgetWidth, kPdfWidgetHeight);
    pdfWidget->addFileAsync(filePath);
    pageTitle = QFileInfo(filePath).fileName();

    selectFileUrl = url;
    fmInfo() << "PDF preview: file URL set successfully:" << url << "title:" << pageTitle;

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
    fmDebug() << "PDF preview: initializing with main window";
    setMainWidget(window);
}
