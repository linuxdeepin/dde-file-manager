// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textextractorplugin.h"
#include "textextractor.h"

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE
DFM_LOG_REGISTER_CATEGORY(EXTRACTOR_PLUGIN_NAMESPACE)
EXTRACTOR_PLUGIN_END_NAMESPACE

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE

TextExtractorPlugin::TextExtractorPlugin(QObject *parent)
    : AbstractExtractorPlugin(parent)
{
}

bool TextExtractorPlugin::canExtract(const QString &filePath) const
{
    return TextExtractor::isTextDocument(filePath);
}

std::optional<QByteArray> TextExtractorPlugin::extract(const QString &filePath)
{
    return TextExtractor::extract(filePath);
}

QString TextExtractorPlugin::name() const
{
    return "TextExtractor";
}

int TextExtractorPlugin::priority() const
{
    // Default priority for text extraction
    return 0;
}

EXTRACTOR_PLUGIN_END_NAMESPACE
