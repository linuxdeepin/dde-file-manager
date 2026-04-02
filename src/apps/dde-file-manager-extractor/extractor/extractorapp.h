// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTRACTORAPP_H
#define EXTRACTORAPP_H

#include "pluginloader.h"
#include "workerpipe.h"

#include <QObject>
#include <QScopedPointer>
#include <QSharedPointer>

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE

/**
 * @brief ExtractorApp is the main application class for the extractor subprocess.
 *
 * It manages the plugin loader and handles extraction requests from stdin.
 */
class ExtractorApp : public QObject
{
    Q_OBJECT

public:
    explicit ExtractorApp(QObject *parent = nullptr);
    ~ExtractorApp() override;

    /**
     * @brief Initialize the extractor application.
     *
     * @param pluginPath Path to the plugin directory
     * @return true if initialization succeeded
     */
    bool initialize(const QString &pluginPath);

    /**
     * @brief Run the main event loop.
     *
     * Processes extraction requests until stdin is closed.
     */
    void run();

    /**
     * @brief Process a batch of files for extraction.
     *
     * @param filePaths List of files to extract
     */
    void processBatch(const QVector<QString> &filePaths);

private:
    QSharedPointer<PluginLoader> m_pluginLoader;
    QScopedPointer<EXTRACTOR_NAMESPACE::WorkerPipe> m_workerPipe;
};

EXTRACTOR_PLUGIN_END_NAMESPACE

#endif   // EXTRACTORAPP_H
