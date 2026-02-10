// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pdfmodel.h"

#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <QThread>

#include <functional>
#include <pdfmodel.h>
#include <dpdfannot.h>
#include <dpdfpage.h>
#include <dpdfdoc.h>

using namespace plugin_filepreview;
#define LOCK_DOCUMENT QMutexLocker docMutexLocker(docMutex);

PDFPage::PDFPage(QMutex *mutex, DPdfPage *page)
    : docMutex(mutex), pdfPage(page)
{
}

PDFPage::~PDFPage()
{
}

QSizeF PDFPage::sizeF() const
{
    return pdfPage->sizeF();
}

QImage PDFPage::render(int width, int height, const QRect &slice) const
{
    LOCK_DOCUMENT

    QRect ratioRect = slice.isValid() ? QRect(slice.x(), slice.y(), slice.width(), slice.height()) : QRect();

    return pdfPage->image(width, height, ratioRect);
}

PDFDocument::PDFDocument(DPdfDoc *document)
    : pdfDocument(document)
{
    docMutex = new QMutex;

    QScreen *srn = QApplication::screens().value(0);
    if (nullptr != srn) {
        docXRes = srn->physicalDotsPerInchX();
        docYRes = srn->physicalDotsPerInchY();
    }
}

PDFDocument::~PDFDocument()
{
    //! 需要确保pages先被析构完成

    docMutex->lock();

    delete pdfDocument;

    pdfDocument = nullptr;

    docMutex->unlock();

    delete docMutex;
}

int PDFDocument::pageCount() const
{
    return pdfDocument->pageCount();
}

Page *PDFDocument::page(int index) const
{
    if (DPdfPage *page = pdfDocument->page(index, docXRes, docYRes)) {

        if (page->isValid())
            return new PDFPage(docMutex, page);
    }

    return nullptr;
}

PDFDocument *PDFDocument::loadDocument(const QString &filePath, const QString &password, Document::Error &error)
{
    DPdfDoc *document = new DPdfDoc(filePath, password);

    if (document->status() == DPdfDoc::SUCCESS) {
        error = Document::kNoError;
        return new PDFDocument(document);
    } else if (document->status() == DPdfDoc::PASSWORD_ERROR) {
        if (password.isEmpty())
            error = Document::kNeedPassword;
        else
            error = Document::kWrongPassword;
    } else
        error = Document::kFileError;

    delete document;

    return nullptr;
}
