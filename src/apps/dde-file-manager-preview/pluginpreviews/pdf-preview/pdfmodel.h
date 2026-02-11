// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PDFMODEL_H
#define PDFMODEL_H

#include "preview_plugin_global.h"

#include "model.h"

#include <QMutex>

class DPdfDoc;
class DPdfPage;
class DPdfAnnot;

namespace plugin_filepreview {

class PDFPage : public Page
{
    friend class PDFDocument;

public:
    ~PDFPage() override;

    /**
     * @brief 获取大小 会根据当前设备的像素密度保持每一页在不同设备上大小相同.
     * @return
     */
    QSizeF sizeF() const override;

    QImage render(int width, int height, const QRect &slice = QRect()) const override;

private:
    explicit PDFPage(QMutex *mutex, DPdfPage *page);

    QMutex *docMutex { nullptr };

    DPdfPage *pdfPage { nullptr };
};

class PDFDocument : public Document
{
public:
    explicit PDFDocument(DPdfDoc *document);

    virtual ~PDFDocument() override;

    int pageCount() const override;

    Page *page(int index) const override;

    static PDFDocument *loadDocument(const QString &filePath, const QString &password, Document::Error &error);

private:
    DPdfDoc *pdfDocument { nullptr };

    QMutex *docMutex { nullptr };

    qreal docXRes = 72;

    qreal docYRes = 72;
};
}
#endif   // PDFMODEL_H
