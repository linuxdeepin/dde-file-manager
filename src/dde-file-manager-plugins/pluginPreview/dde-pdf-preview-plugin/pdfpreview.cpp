// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pdfpreview.h"

#include <QMimeType>
#include <QMimeDatabase>
#include <QUrl>
#include <QFileInfo>

#include "pdfwidget.h"

DFM_USE_NAMESPACE

PDFPreview::PDFPreview(QObject *parent)
    : DFMFilePreview(parent)
{

}

PDFPreview::~PDFPreview()
{
    if (m_pdfWidget) {
        m_pdfWidget->setNeedRelease(true);
        if (m_pdfWidget->getCanRelease())
            m_pdfWidget->deleteLater();
    }

}

bool PDFPreview::setFileUrl(const DUrl &url)
{
    if (m_url == url)
        return true;

    if (!url.isLocalFile())
        return false;

    if (m_pdfWidget)
        return false;

    m_pdfWidget = new PdfWidget(url.toLocalFile());
    m_pdfWidget->setFixedSize(800, 500);

    m_title = QFileInfo(url.toLocalFile()).fileName();

    Q_EMIT titleChanged();

    return true;
}

DUrl PDFPreview::fileUrl() const
{
    return m_url;
}

QWidget *PDFPreview::contentWidget() const
{
    return m_pdfWidget;
}

QString PDFPreview::title() const
{
    return m_title;
}

bool PDFPreview::showStatusBarSeparator() const
{
    return true;
}
