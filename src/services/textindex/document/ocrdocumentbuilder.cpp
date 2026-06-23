// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrdocumentbuilder.h"

#include "utils/indexutility.h"

#include <dfm-search/field_names.h>

#include <lucene++/NumericField.h>

#include <QFileInfo>

SERVICETEXTINDEX_BEGIN_NAMESPACE
using namespace Lucene;
DFM_SEARCH_USE_NS
using namespace DFMSEARCH::LuceneFieldNames;

DocumentPtr OcrDocumentBuilder::build(const QString &filePath, const QString &text,
                                       const BuilderOptions &options) const
{
    DocumentPtr doc = newLucene<Document>();

    doc->add(newLucene<Field>(OcrText::kPath, filePath.toStdWString(),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    const QStringList ancestorPaths = PathCalculator::extractAncestorPaths(filePath);
    for (const QString &ancestorPath : ancestorPaths) {
        doc->add(newLucene<Field>(OcrText::kAncestorPaths, ancestorPath.toStdWString(),
                                  Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
    }

    const QFileInfo fileInfo(filePath);

    // Add modify time as NumericField for efficient range queries
    const qint64 modifyTimeSecs = fileInfo.lastModified().toSecsSinceEpoch();
    NumericFieldPtr modifyTimeField = newLucene<NumericField>(OcrText::kModifyTime, Field::STORE_YES, true);
    modifyTimeField->setLongValue(modifyTimeSecs);
    doc->add(modifyTimeField);

    // Add birth time as NumericField for efficient range queries
    const qint64 birthTimeSecs = fileInfo.birthTime().toSecsSinceEpoch();
    NumericFieldPtr birthTimeField = newLucene<NumericField>(OcrText::kBirthTime, Field::STORE_YES, true);
    birthTimeField->setLongValue(birthTimeSecs);
    doc->add(birthTimeField);

    // Add file size as NumericField for efficient range queries
    const qint64 fileSize = fileInfo.size();
    NumericFieldPtr fileSizeField = newLucene<NumericField>(OcrText::kFileSize, Field::STORE_YES, true);
    fileSizeField->setLongValue(fileSize);
    doc->add(fileSizeField);

    doc->add(newLucene<Field>(OcrText::kFilename, fileInfo.fileName().toStdWString(),
                              Field::STORE_YES, Field::INDEX_ANALYZED));

    const QString hiddenTag = DFMSEARCH::Global::isHiddenPathOrInHiddenDir(fileInfo.absoluteFilePath())
            ? QStringLiteral("Y")
            : QStringLiteral("N");
    doc->add(newLucene<Field>(OcrText::kIsHidden, hiddenTag.toStdWString(),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // Add MD5 checksum for deduplication (exact match, not tokenized)
    if (!options.checksum.isEmpty()) {
        doc->add(newLucene<Field>(OcrText::kCheckSum, options.checksum.toStdWString(),
                                  Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    }

    // Add file extension for filtering (exact match, not tokenized)
    const QString fileExt = fileInfo.suffix().toLower();
    if (!fileExt.isEmpty()) {
        doc->add(newLucene<Field>(OcrText::kFileExt, fileExt.toStdWString(),
                                  Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    }

    if (!text.trimmed().isEmpty()) {
        doc->add(newLucene<Field>(OcrText::kOcrContents, text.trimmed().toStdWString(),
                                  Field::STORE_YES, Field::INDEX_ANALYZED));
    }

    return doc;
}

SERVICETEXTINDEX_END_NAMESPACE
