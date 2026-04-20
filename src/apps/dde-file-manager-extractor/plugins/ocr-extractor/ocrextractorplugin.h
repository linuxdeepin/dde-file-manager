// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OCREXTRACTORPLUGIN_H
#define OCREXTRACTORPLUGIN_H

#include "abstractextractorplugin.h"
#include "extractor_plugin_global.h"

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE

class OcrExtractorPlugin : public AbstractExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractExtractorPluginInterface_iid FILE "ocrextractor.json")
    Q_INTERFACES(EXTRACTOR_PLUGIN_NAMESPACE::AbstractExtractorPlugin)

public:
    explicit OcrExtractorPlugin(QObject *parent = nullptr);
    ~OcrExtractorPlugin() override = default;

    bool canExtract(const QString &filePath) const override;
    std::optional<QByteArray> extract(const QString &filePath) override;
    QString name() const override;
    int priority() const override;
};

EXTRACTOR_PLUGIN_END_NAMESPACE

#endif   // OCREXTRACTORPLUGIN_H
