// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrextractorplugin.h"
#include "ocrextractor.h"

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE
DFM_LOG_REGISTER_CATEGORY(EXTRACTOR_PLUGIN_NAMESPACE)
EXTRACTOR_PLUGIN_END_NAMESPACE

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE

OcrExtractorPlugin::OcrExtractorPlugin(QObject *parent)
    : AbstractExtractorPlugin(parent)
{
}

bool OcrExtractorPlugin::canExtract(const QString &filePath) const
{
    return OcrExtractor::isSupportedImage(filePath);
}

std::optional<QByteArray> OcrExtractorPlugin::extract(const QString &filePath)
{
    return OcrExtractor::extract(filePath);
}

QString OcrExtractorPlugin::name() const
{
    return "OcrExtractor";
}

int OcrExtractorPlugin::priority() const
{
    return 0;
}

EXTRACTOR_PLUGIN_END_NAMESPACE
