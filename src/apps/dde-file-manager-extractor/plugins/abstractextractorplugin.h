// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTEXTRACTORPLUGIN_H
#define ABSTRACTEXTRACTORPLUGIN_H

#include "extractor_plugin_global.h"

#include <QObject>
#include <QByteArray>
#include <QString>
#include <optional>

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE

/**
 * @brief Abstract base class for extractor plugins.
 *
 * Plugins must implement canExtract() to check if they can handle a file,
 * and extract() to perform the actual extraction.
 */
class AbstractExtractorPlugin : public QObject
{
    Q_OBJECT

public:
    explicit AbstractExtractorPlugin(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    virtual ~AbstractExtractorPlugin() = default;

    /**
     * @brief Check if this plugin can extract content from the given file.
     *
     * This method should perform runtime detection based on file content,
     * not just mimetype. This allows plugins to handle files based on
     * actual content rather than file extension.
     *
     * @param filePath Path to the file to check
     * @return true if this plugin can extract from the file
     */
    virtual bool canExtract(const QString &filePath) const = 0;

    /**
     * @brief Extract content from the given file.
     *
     * @param filePath Path to the file to extract from
     * @return Extracted content as QByteArray, or nullopt on failure
     */
    virtual std::optional<QByteArray> extract(const QString &filePath) = 0;

    /**
     * @brief Get the name of this plugin.
     *
     * @return Plugin name for logging and debugging
     */
    virtual QString name() const = 0;

    /**
     * @brief Get the priority of this plugin.
     *
     * Higher priority plugins are checked first.
     * Default priority is 0.
     *
     * @return Priority value (higher = more priority)
     */
    virtual int priority() const { return 0; }
};

EXTRACTOR_PLUGIN_END_NAMESPACE

#define AbstractExtractorPluginInterface_iid "org.deepin.dde.filemanager.AbstractExtractorPlugin/1.0"
Q_DECLARE_INTERFACE(EXTRACTOR_PLUGIN_NAMESPACE::AbstractExtractorPlugin, AbstractExtractorPluginInterface_iid)

#endif   // ABSTRACTEXTRACTORPLUGIN_H
