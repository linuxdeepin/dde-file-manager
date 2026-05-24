// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lowercasengramanalyzer.h"

#include <lucene++/LowerCaseFilter.h>
#include <lucene++/LuceneObject.h>
#include <lucene++/NGramTokenizer.h>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace {

class LowerCaseNGramTokenStreams : public Lucene::LuceneObject
{
public:
    LowerCaseNGramTokenStreams(const Lucene::ReaderPtr &reader, int32_t minGram, int32_t maxGram)
        : source(Lucene::newLucene<Lucene::NGramTokenizer>(reader, minGram, maxGram)),
          result(Lucene::newLucene<Lucene::LowerCaseFilter>(source))
    {
    }

    Lucene::NGramTokenizerPtr source;
    Lucene::TokenStreamPtr result;
};

}   // namespace

LowerCaseNGramAnalyzer::LowerCaseNGramAnalyzer(int32_t minGram, int32_t maxGram)
    : m_minGram(minGram),
      m_maxGram(maxGram)
{
}

LowerCaseNGramAnalyzer::~LowerCaseNGramAnalyzer() = default;

Lucene::TokenStreamPtr LowerCaseNGramAnalyzer::tokenStream(const Lucene::String &fieldName,
                                                           const Lucene::ReaderPtr &reader)
{
    (void) fieldName;
    return Lucene::newLucene<Lucene::LowerCaseFilter>(
            Lucene::newLucene<Lucene::NGramTokenizer>(reader, m_minGram, m_maxGram));
}

Lucene::TokenStreamPtr LowerCaseNGramAnalyzer::reusableTokenStream(const Lucene::String &fieldName,
                                                                   const Lucene::ReaderPtr &reader)
{
    (void) fieldName;

    boost::shared_ptr<LowerCaseNGramTokenStreams> streams =
            boost::dynamic_pointer_cast<LowerCaseNGramTokenStreams>(getPreviousTokenStream());

    if (!streams) {
        streams = Lucene::newLucene<LowerCaseNGramTokenStreams>(reader, m_minGram, m_maxGram);
        setPreviousTokenStream(streams);
    } else {
        streams->source->reset(reader);
    }

    return streams->result;
}

SERVICETEXTINDEX_END_NAMESPACE
