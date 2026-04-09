// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "contentdocumentbuilder.h"

#include "utils/indexutility.h"

#include <dfm-search/field_names.h>

#include <QDateTime>
#include <QFileInfo>

SERVICETEXTINDEX_BEGIN_NAMESPACE
using namespace Lucene;
DFM_SEARCH_USE_NS
using namespace DFMSEARCH::LuceneFieldNames;

DocumentPtr ContentDocumentBuilder::build(const QString &filePath, const QString &text) const
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
    const QString modifyEpoch = QString::number(fileInfo.lastModified().toSecsSinceEpoch());
    doc->add(newLucene<Field>(L"modified", modifyEpoch.toStdWString(),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    doc->add(newLucene<Field>(Content::kFilename, fileInfo.fileName().toStdWString(),
                              Field::STORE_YES, Field::INDEX_ANALYZED));

    const QString hiddenTag = DFMSEARCH::Global::isHiddenPathOrInHiddenDir(fileInfo.absoluteFilePath())
            ? QStringLiteral("Y")
            : QStringLiteral("N");
    doc->add(newLucene<Field>(Content::kIsHidden, hiddenTag.toStdWString(),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    if (!text.trimmed().isEmpty()) {
        doc->add(newLucene<Field>(Content::kContents, text.trimmed().toStdWString(),
                                  Field::STORE_YES, Field::INDEX_ANALYZED));
    }

    return doc;
}

SERVICETEXTINDEX_END_NAMESPACE
