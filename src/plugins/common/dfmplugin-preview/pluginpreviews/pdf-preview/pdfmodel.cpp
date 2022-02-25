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

PREVIEW_USE_NAMESPACE
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
        error = Document::NoError;
        return new PDFDocument(document);
    } else if (document->status() == DPdfDoc::PASSWORD_ERROR) {
        if (password.isEmpty())
            error = Document::NeedPassword;
        else
            error = Document::WrongPassword;
    } else
        error = Document::FileError;

    delete document;

    return nullptr;
}
