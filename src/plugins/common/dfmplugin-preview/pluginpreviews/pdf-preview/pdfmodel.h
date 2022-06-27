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
