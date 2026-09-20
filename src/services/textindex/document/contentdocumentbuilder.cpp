// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "contentdocumentbuilder.h"

#include "utils/indexutility.h"

#include <dfm-search/field_names.h>

#include <lucene++/NumericField.h>

#include <QDateTime>
#include <QFileInfo>

SERVICETEXTINDEX_BEGIN_NAMESPACE
using namespace Lucene;
DFM_SEARCH_USE_NS
using namespace DFMSEARCH::LuceneFieldNames;

DocumentPtr ContentDocumentBuilder::build(const QString &filePath, const QString &text,
                                          const BuilderOptions &options) const
{
    DocumentPtr doc = newLucene<Document>();

    doc->add(newLucene<Field>(Content::kPath, filePath.toStdWString(),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    const QStringList ancestorPaths = PathCalculator::extractAncestorPaths(filePath);
    for (const QString &ancestorPath : ancestorPaths) {
        doc->add(newLucene<Field>(Content::kAncestorPaths, ancestorPath.toStdWString(),
                                  Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
    }

    const QFileInfo fileInfo(filePath);
    const qint64 modifyTimeSecs = fileInfo.lastModified().toSecsSinceEpoch();
    NumericFieldPtr modifyTimeField = newLucene<NumericField>(Content::kModifyTime, Field::STORE_YES, true);
    modifyTimeField->setLongValue(modifyTimeSecs);
    doc->add(modifyTimeField);

    // Add birth time as NumericField for efficient range queries
    const qint64 birthTimeSecs = fileInfo.birthTime().toSecsSinceEpoch();
    NumericFieldPtr birthTimeField = newLucene<NumericField>(Content::kBirthTime, Field::STORE_YES, true);
    birthTimeField->setLongValue(birthTimeSecs);
    doc->add(birthTimeField);

    // Add file size as NumericField for efficient range queries
    const qint64 fileSize = fileInfo.size();
    NumericFieldPtr fileSizeField = newLucene<NumericField>(Content::kFileSize, Field::STORE_YES, true);
    fileSizeField->setLongValue(fileSize);
    doc->add(fileSizeField);

    doc->add(newLucene<Field>(Content::kFilename, fileInfo.fileName().toStdWString(),
                              Field::STORE_YES, Field::INDEX_ANALYZED));

    const QString hiddenTag = DFMSEARCH::Global::isHiddenPathOrInHiddenDir(fileInfo.absoluteFilePath())
            ? QStringLiteral("Y")
            : QStringLiteral("N");
    doc->add(newLucene<Field>(Content::kIsHidden, hiddenTag.toStdWString(),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // Add MD5 checksum for deduplication (exact match, not tokenized)
    if (!options.checksum.isEmpty()) {
        doc->add(newLucene<Field>(Content::kCheckSum, options.checksum.toStdWString(),
                                  Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    }

    // Add file extension for filtering (exact match, not tokenized)
    const QString fileExt = fileInfo.suffix().toLower();
    if (!fileExt.isEmpty()) {
        doc->add(newLucene<Field>(Content::kFileExt, fileExt.toStdWString(),
                                  Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    }

    doc->add(newLucene<Field>(Content::kContents, text.trimmed().toStdWString(),
                              Field::STORE_YES, Field::INDEX_ANALYZED));

    return doc;
}

std::list<PathDerivedFieldSpec> ContentDocumentBuilder::pathDerivedFields() const
{
    // 与 build() 中对应字段的存储/索引属性保持一致；纯元数据重算，无内容提取
    return {
        { Content::kFilename, [](const QString &newPath) -> FieldPtr {
             return newLucene<Field>(Content::kFilename, QFileInfo(newPath).fileName().toStdWString(),
                                     Field::STORE_YES, Field::INDEX_ANALYZED);
         } },
        { Content::kIsHidden, [](const QString &newPath) -> FieldPtr {
             const QString hiddenTag = DFMSEARCH::Global::isHiddenPathOrInHiddenDir(QFileInfo(newPath).absoluteFilePath())
                     ? QStringLiteral("Y")
                     : QStringLiteral("N");
             return newLucene<Field>(Content::kIsHidden, hiddenTag.toStdWString(),
                                     Field::STORE_YES, Field::INDEX_NOT_ANALYZED);
         } },
        { Content::kFileExt, [](const QString &newPath) -> FieldPtr {
             const QString fileExt = QFileInfo(newPath).suffix().toLower();
             if (fileExt.isEmpty())
                 return nullptr;   // 与 build() 一致：无扩展名则不加该字段
             return newLucene<Field>(Content::kFileExt, fileExt.toStdWString(),
                                     Field::STORE_YES, Field::INDEX_NOT_ANALYZED);
         } },
    };
}

SERVICETEXTINDEX_END_NAMESPACE
