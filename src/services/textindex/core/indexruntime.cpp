// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "indexruntime.h"
#include "env/envdetector.h"

SERVICETEXTINDEX_BEGIN_NAMESPACE

IndexRuntime::IndexRuntime(IndexProfile profile, QObject *parent)
    : QObject(parent),
      m_profile(std::move(profile)),
      m_stateStore(m_profile),
      m_context(m_profile, &m_stateStore, selectExtractor(), selectDocumentBuilder()),
      m_fsEventController(new FSEventController(m_profile, this))
{
    if (m_profile.type() == IndexProfile::Type::Filename)
        m_backlogTracker = std::make_unique<FilenameBacklogTracker>(&m_stateStore);
    m_taskManager = new TaskManager(&m_context, m_backlogTracker.get(), this);
    EnvDetector::instance().setDataPath(m_profile.indexDirectory());
    EnvDetector::instance().start();
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
    // 无内容提取需求的 profile（filename 只读文件元数据）不装配 extractor：
    // 返回 nullptr → createFileDocument 跳过 ProcessExtractor::extract 调用，
    // 避免全盘遍历时逐文件读取内容造成的严重 IO 浪费。
    if (!m_profile.requiresContentExtraction())
        return nullptr;
    return &m_processExtractor;
}

const IndexDocumentBuilder *IndexRuntime::selectDocumentBuilder() const
{
    switch (m_profile.type()) {
    case IndexProfile::Type::Ocr:
        return &m_ocrDocumentBuilder;
    case IndexProfile::Type::Filename:
        return &m_fileNameDocumentBuilder;
    case IndexProfile::Type::Content:
    default:
        return &m_contentDocumentBuilder;
    }
}

SERVICETEXTINDEX_END_NAMESPACE
