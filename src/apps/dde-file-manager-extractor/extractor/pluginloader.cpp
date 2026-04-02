// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginloader.h"

#include <QDir>
#include <QPluginLoader>
#include <QDebug>
#include <algorithm>

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE

PluginLoader::PluginLoader(QObject *parent)
    : QObject(parent)
{
}

PluginLoader::~PluginLoader()
{
}

int PluginLoader::loadPlugins(const QString &pluginPath)
{
    m_plugins.clear();

    if (pluginPath.isEmpty()) {
        fmWarning() << "PluginLoader::loadPlugins: Plugin path is empty";
        return 0;
    }

    QDir dir(pluginPath);
    if (!dir.exists()) {
        fmWarning() << "PluginLoader::loadPlugins: Plugin directory does not exist:" << pluginPath;
        return 0;
    }

    fmInfo() << "PluginLoader: Loading plugins from:" << pluginPath;

    // Find all shared libraries in the plugin directory
    QStringList filters;
    filters << "*.so";

    int loadedCount = 0;
    const QStringList entries = dir.entryList(filters, QDir::Files);

    for (const QString &fileName : entries) {
        QString filePath = dir.absoluteFilePath(fileName);

        fmDebug() << "PluginLoader: Attempting to load:" << filePath;

        QPluginLoader loader(filePath);
        QObject *instance = loader.instance();

        if (!instance) {
            fmWarning() << "PluginLoader: Failed to load plugin:" << filePath
                        << "error:" << loader.errorString();
            continue;
        }

        AbstractExtractorPlugin *plugin = qobject_cast<AbstractExtractorPlugin *>(instance);
        if (!plugin) {
            fmWarning() << "PluginLoader: Plugin does not implement AbstractExtractorPlugin:" << filePath;
            loader.unload();
            continue;
        }

        m_plugins.append(QSharedPointer<AbstractExtractorPlugin>(plugin, [](AbstractExtractorPlugin *) {
            // Don't delete, QPluginLoader manages lifetime
        }));

        fmInfo() << "PluginLoader: Loaded plugin:" << plugin->name()
                 << "priority:" << plugin->priority();
        loadedCount++;
    }

    // Sort plugins by priority (highest first)
    std::sort(m_plugins.begin(), m_plugins.end(),
              [](const QSharedPointer<AbstractExtractorPlugin> &a,
                 const QSharedPointer<AbstractExtractorPlugin> &b) {
                  return a->priority() > b->priority();
              });

    fmInfo() << "PluginLoader: Loaded" << loadedCount << "plugins";
    return loadedCount;
}

QList<QSharedPointer<AbstractExtractorPlugin>> PluginLoader::plugins() const
{
    return m_plugins;
}

QSharedPointer<AbstractExtractorPlugin> PluginLoader::findPlugin(const QString &filePath) const
{
    for (const auto &plugin : m_plugins) {
        if (plugin->canExtract(filePath)) {
            fmDebug() << "PluginLoader: Found plugin" << plugin->name()
                      << "for file:" << filePath;
            return plugin;
        }
    }

    fmDebug() << "PluginLoader: No plugin found for file:" << filePath;
    return nullptr;
}

int PluginLoader::pluginCount() const
{
    return m_plugins.size();
}

EXTRACTOR_PLUGIN_END_NAMESPACE
