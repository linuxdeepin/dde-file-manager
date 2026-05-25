// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "contentdeduplication.h"
#include "utils/scopeguard.h"

#include <dfm-search/field_names.h>

#include <lucene++/LuceneHeaders.h>

SERVICETEXTINDEX_BEGIN_NAMESPACE

using namespace Lucene;
DFM_SEARCH_USE_NS
using namespace DFMSEARCH::LuceneFieldNames;

namespace ContentDeduplication {

QString lookupByTextChecksum(const QString &checksum, const QString &contentIndexDir)
{
    if (checksum.isEmpty() || contentIndexDir.isEmpty()) {
        return {};
    }

    try {
        IndexReaderPtr reader = IndexReader::open(
                FSDirectory::open(contentIndexDir.toStdWString()), true);

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
                newLucene<Term>(Content::kCheckSum, checksum.toStdWString()));

        TopDocsPtr topDocs = searcher->search(query, 1);

        if (topDocs->totalHits == 0) {
            return {};
        }

        DocumentPtr doc = searcher->doc(topDocs->scoreDocs[0]->doc);
        const String contents = doc->get(Content::kContents);

        return contents.empty() ? QString()
                                : QString::fromStdWString(contents);
    } catch (...) {
        // Deduplication is a best-effort optimization.
        // On any failure, fall through to normal content extraction.
        return {};
    }
}

}   // namespace ContentDeduplication

SERVICETEXTINDEX_END_NAMESPACE