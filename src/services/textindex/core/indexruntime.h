// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXRUNTIME_H
#define INDEXRUNTIME_H

#include "core/indexcontext.h"
#include "document/contentdocumentbuilder.h"
#include "document/ocrdocumentbuilder.h"
#include "document/filenamedocumentbuilder.h"
#include "fsmonitor/fseventcontroller.h"
#include "extractor/processextractor.h"
#include "profile/indexprofile.h"
#include "state/indexstatestore.h"
#include "task/taskmanager.h"
#include "task/backlogtracker.h"

#include <QObject>
#include <memory>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class IndexRuntime : public QObject
{
    Q_OBJECT

public:
    explicit IndexRuntime(IndexProfile profile, QObject *parent = nullptr);

    const IndexProfile &profile() const;
    const IndexStateStore &stateStore() const;
    const IndexContext &context() const;

    TaskManager *taskManager() const;
    FSEventController *fsEventController() const;

private:
    const IndexExtractor *selectExtractor() const;
    const IndexDocumentBuilder *selectDocumentBuilder() const;

    IndexProfile m_profile;
    IndexStateStore m_stateStore;
    ProcessExtractor m_processExtractor;
    ContentDocumentBuilder m_contentDocumentBuilder;
    OcrDocumentBuilder m_ocrDocumentBuilder;
    FileNameDocumentBuilder m_fileNameDocumentBuilder;
    IndexContext m_context;
    std::unique_ptr<BacklogTracker> m_backlogTracker;
    TaskManager *m_taskManager { nullptr };
    FSEventController *m_fsEventController { nullptr };
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INDEXRUNTIME_H
