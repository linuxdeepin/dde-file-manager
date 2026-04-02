// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEXTRACTORPLUGIN_H
#define TEXTEXTRACTORPLUGIN_H

#include "extractor_plugin_global.h"
#include "abstractextractorplugin.h"

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE

/**
 * @brief TextExtractorPlugin provides text content extraction functionality.
 */
class TextExtractorPlugin : public AbstractExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractExtractorPluginInterface_iid FILE "textextractor.json")
    Q_INTERFACES(EXTRACTOR_PLUGIN_NAMESPACE::AbstractExtractorPlugin)

public:
    explicit TextExtractorPlugin(QObject *parent = nullptr);
    ~TextExtractorPlugin() override = default;

    bool canExtract(const QString &filePath) const override;
    std::optional<QByteArray> extract(const QString &filePath) override;
    QString name() const override;
    int priority() const override;
};

EXTRACTOR_PLUGIN_END_NAMESPACE

#endif   // TEXTEXTRACTORPLUGIN_H
