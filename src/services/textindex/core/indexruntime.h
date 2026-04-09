// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXRUNTIME_H
#define INDEXRUNTIME_H

#include "core/indexcontext.h"
#include "document/contentdocumentbuilder.h"
#include "document/ocrdocumentbuilder.h"
#include "fsmonitor/fseventcontroller.h"
#include "extractor/legacydocutilsextractor.h"
#include "extractor/processextractor.h"
#include "profile/indexprofile.h"
#include "state/indexstatestore.h"
#include "task/taskmanager.h"

#include <QObject>

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
    LegacyDocUtilsExtractor m_legacyExtractor;
    ProcessExtractor m_processExtractor;
    ContentDocumentBuilder m_contentDocumentBuilder;
    OcrDocumentBuilder m_ocrDocumentBuilder;
    IndexContext m_context;
    TaskManager *m_taskManager { nullptr };
    FSEventController *m_fsEventController { nullptr };
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INDEXRUNTIME_H
