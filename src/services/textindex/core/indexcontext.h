// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXCONTEXT_H
#define INDEXCONTEXT_H

#include "document/indexdocumentbuilder.h"
#include "extractor/indexextractor.h"
#include "profile/indexprofile.h"
#include "state/indexstatestore.h"

SERVICETEXTINDEX_BEGIN_NAMESPACE

class IndexContext
{
public:
    IndexContext(IndexProfile profile,
                 const IndexStateStore *stateStore,
                 const IndexExtractor *extractor,
                 const IndexDocumentBuilder *documentBuilder)
        : m_profile(std::move(profile)),
          m_stateStore(stateStore),
          m_extractor(extractor),
          m_documentBuilder(documentBuilder)
    {
    }

    const IndexProfile &profile() const
    {
        return m_profile;
    }

    const IndexStateStore *stateStore() const
    {
        return m_stateStore;
    }

    const IndexExtractor *extractor() const
    {
        return m_extractor;
    }

    const IndexDocumentBuilder *documentBuilder() const
    {
        return m_documentBuilder;
    }

private:
    IndexProfile m_profile;
    const IndexStateStore *m_stateStore { nullptr };
    const IndexExtractor *m_extractor { nullptr };
    const IndexDocumentBuilder *m_documentBuilder { nullptr };
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INDEXCONTEXT_H
