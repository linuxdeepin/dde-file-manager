// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model.h"
#include "pdfmodel.h"
#include "dpdfannot.h"
#include "dpdfpage.h"
#include "dpdfdoc.h"

#include <QProcess>
#include <QFile>

using namespace plugin_filepreview;
Document *DocumentFactory::getDocument(const int &fileType, const QString &filePath,
                                       const QString &password, Document::Error &error)
{
    Document *document = nullptr;

    if (kPDF == fileType) {
        document = PDFDocument::loadDocument(filePath, password, error);
    }

    return document;
}
