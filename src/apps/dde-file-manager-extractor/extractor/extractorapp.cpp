// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extractorapp.h"

#include <dfm-base/utils/processprioritymanager.h>

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE

ExtractorApp::ExtractorApp(QObject *parent)
    : QObject(parent)
    , m_pluginLoader(new PluginLoader(this))
    , m_workerPipe(new EXTRACTOR_NAMESPACE::WorkerPipe(this))
{
}

ExtractorApp::~ExtractorApp()
{
}

bool ExtractorApp::initialize(const QString &pluginPath)
{
    fmInfo() << "ExtractorApp: Initializing with plugin path:" << pluginPath;

    // Lower process priority to avoid impacting user experience
    DFMBASE_NAMESPACE::ProcessPriorityManager::lowerAllAvailablePriorities(true);

    // Load plugins
    int pluginCount = m_pluginLoader->loadPlugins(pluginPath);
    if (pluginCount == 0) {
        fmWarning() << "ExtractorApp: No plugins loaded";
        return false;
    }

    fmInfo() << "ExtractorApp: Initialization complete, loaded" << pluginCount << "plugins";
    return true;
}

void ExtractorApp::run()
{
    fmInfo() << "ExtractorApp: Starting main loop";

    if (!m_workerPipe->initialize()) {
        fmCritical() << "ExtractorApp: Failed to initialize worker pipe";
        return;
    }

    // Connect signals
    connect(m_workerPipe.get(), &EXTRACTOR_NAMESPACE::WorkerPipe::batchReceived,
            this, [this](const QVector<QString> &filePaths) {
                processBatch(filePaths);
            });

    connect(m_workerPipe.get(), &EXTRACTOR_NAMESPACE::WorkerPipe::stdinClosed,
            this, []() {
                fmInfo() << "ExtractorApp: Stdin closed, exiting";
                QCoreApplication::quit();
            });

    fmInfo() << "ExtractorApp: Ready to process requests";

    // Enter event loop
    QCoreApplication::exec();

    fmInfo() << "ExtractorApp: Exiting";
}

void ExtractorApp::processBatch(const QVector<QString> &filePaths)
{
    fmInfo() << "ExtractorApp: Processing batch of" << filePaths.size() << "files";

    for (const QString &filePath : filePaths) {
        fmDebug() << "ExtractorApp: Processing file:" << filePath;

        // Send started notification
        m_workerPipe->sendStarted(filePath);

        // Check if file exists
        if (!QFile::exists(filePath)) {
            fmWarning() << "ExtractorApp: File does not exist:" << filePath;
            m_workerPipe->sendFailed(filePath, "File does not exist");
            continue;
        }

        // Find appropriate plugin
        auto plugin = m_pluginLoader->findPlugin(filePath);
        if (!plugin) {
            fmWarning() << "ExtractorApp: No plugin can handle file:" << filePath;
            m_workerPipe->sendFailed(filePath, "No plugin available for this file type");
            continue;
        }

        // Extract content
        auto result = plugin->extract(filePath);
        if (!result.has_value()) {
            fmWarning() << "ExtractorApp: Extraction failed for file:" << filePath;
            m_workerPipe->sendFailed(filePath, "Extraction failed");
            continue;
        }

        // Send extracted data
        fmDebug() << "ExtractorApp: Successfully extracted" << result->size()
                  << "bytes from:" << filePath;
        m_workerPipe->sendData(filePath, result.value());
    }

    // Send batch done notification
    m_workerPipe->sendBatchDone();

    fmInfo() << "ExtractorApp: Batch processing complete";
}

EXTRACTOR_PLUGIN_END_NAMESPACE
