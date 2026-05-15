// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrdeduplication.h"
#include "utils/scopeguard.h"

#include <dfm-search/field_names.h>

#include <lucene++/LuceneHeaders.h>

SERVICETEXTINDEX_BEGIN_NAMESPACE

using namespace Lucene;
DFM_SEARCH_USE_NS
using namespace DFMSEARCH::LuceneFieldNames;

namespace OcrDeduplication {

QString lookupByTextChecksum(const QString &checksum, const QString &ocrIndexDir)
{
    if (checksum.isEmpty() || ocrIndexDir.isEmpty()) {
        return {};
    }

    try {
        IndexReaderPtr reader = IndexReader::open(
                FSDirectory::open(ocrIndexDir.toStdWString()), true);

        ScopeGuard readerCloser([&reader]() {
            try {
                if (reader) {
                    reader->close();
                }
            } catch (...) {
            }
        });

        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        TermQueryPtr query = newLucene<TermQuery>(
                newLucene<Term>(OcrText::kCheckSum, checksum.toStdWString()));

        TopDocsPtr topDocs = searcher->search(query, 1);

        if (topDocs->totalHits == 0) {
            return {};
        }

        DocumentPtr doc = searcher->doc(topDocs->scoreDocs[0]->doc);
        const String ocrContents = doc->get(OcrText::kOcrContents);

        return ocrContents.empty() ? QString()
                                   : QString::fromStdWString(ocrContents);
    } catch (...) {
        // Deduplication is a best-effort optimization.
        // On any failure, fall through to normal OCR extraction.
        return {};
    }
}

}   // namespace OcrDeduplication

SERVICETEXTINDEX_END_NAMESPACE
