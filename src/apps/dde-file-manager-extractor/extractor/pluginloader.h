// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include "extractor_plugin_global.h"
#include "abstractextractorplugin.h"

#include <QObject>
#include <QList>
#include <QSharedPointer>

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE

/**
 * @brief PluginLoader manages loading and querying extractor plugins.
 *
 * This class loads plugins from a specified directory and provides
 * methods to find the appropriate plugin for a given file.
 */
class PluginLoader : public QObject
{
    Q_OBJECT

public:
    explicit PluginLoader(QObject *parent = nullptr);
    ~PluginLoader() override;

    /**
     * @brief Load plugins from the specified directory.
     *
     * @param pluginPath Directory containing plugin libraries
     * @return Number of plugins loaded
     */
    int loadPlugins(const QString &pluginPath);

    /**
     * @brief Get all loaded plugins.
     *
     * @return List of loaded plugins, sorted by priority (highest first)
     */
    QList<QSharedPointer<AbstractExtractorPlugin>> plugins() const;

    /**
     * @brief Find a plugin that can extract from the given file.
     *
     * Iterates through plugins by priority until one returns true
     * for canExtract().
     *
     * @param filePath Path to the file
     * @return Plugin that can extract the file, or nullptr if none found
     */
    QSharedPointer<AbstractExtractorPlugin> findPlugin(const QString &filePath) const;

    /**
     * @brief Get the number of loaded plugins.
     */
    int pluginCount() const;

private:
    QList<QSharedPointer<AbstractExtractorPlugin>> m_plugins;
};

EXTRACTOR_PLUGIN_END_NAMESPACE

#endif   // PLUGINLOADER_H
