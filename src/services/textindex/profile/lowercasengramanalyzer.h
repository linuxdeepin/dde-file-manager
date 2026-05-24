// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOWERCASENGRAMANALYZER_H
#define LOWERCASENGRAMANALYZER_H

#include "service_textindex_global.h"

#include <lucene++/LuceneHeaders.h>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class LowerCaseNGramAnalyzer : public Lucene::Analyzer
{
public:
    LowerCaseNGramAnalyzer(int32_t minGram, int32_t maxGram);
    ~LowerCaseNGramAnalyzer() override;

    Lucene::TokenStreamPtr tokenStream(const Lucene::String &fieldName,
                                       const Lucene::ReaderPtr &reader) override;
    Lucene::TokenStreamPtr reusableTokenStream(const Lucene::String &fieldName,
                                               const Lucene::ReaderPtr &reader) override;

private:
    int32_t m_minGram;
    int32_t m_maxGram;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // LOWERCASENGRAMANALYZER_H
