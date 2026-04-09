// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "indexruntime.h"

SERVICETEXTINDEX_BEGIN_NAMESPACE

IndexRuntime::IndexRuntime(IndexProfile profile, QObject *parent)
    : QObject(parent),
      m_profile(std::move(profile)),
      m_stateStore(m_profile),
      m_context(m_profile, &m_stateStore, selectExtractor(), selectDocumentBuilder()),
      m_taskManager(new TaskManager(&m_context, this)),
      m_fsEventController(new FSEventController(m_profile, this))
{
}

const IndexProfile &IndexRuntime::profile() const
{
    return m_profile;
}

const IndexStateStore &IndexRuntime::stateStore() const
{
    return m_stateStore;
}

const IndexContext &IndexRuntime::context() const
{
    return m_context;
}

TaskManager *IndexRuntime::taskManager() const
{
    return m_taskManager;
}

FSEventController *IndexRuntime::fsEventController() const
{
    return m_fsEventController;
}

const IndexExtractor *IndexRuntime::selectExtractor() const
{
    switch (m_profile.type()) {
    case IndexProfile::Type::Ocr:
        return &m_processExtractor;
    case IndexProfile::Type::Content:
    default:
        return &m_legacyExtractor;
    }
}

const IndexDocumentBuilder *IndexRuntime::selectDocumentBuilder() const
{
    switch (m_profile.type()) {
    case IndexProfile::Type::Ocr:
        return &m_ocrDocumentBuilder;
    case IndexProfile::Type::Content:
    default:
        return &m_contentDocumentBuilder;
    }
}

SERVICETEXTINDEX_END_NAMESPACE
