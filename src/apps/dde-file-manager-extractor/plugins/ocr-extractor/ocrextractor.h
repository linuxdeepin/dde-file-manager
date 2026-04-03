// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OCREXTRACTOR_H
#define OCREXTRACTOR_H

#include "extractor_plugin_global.h"

#include <QByteArray>
#include <QString>

#include <optional>

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE

class OcrExtractor
{
public:
    static constexpr size_t kDefaultMaxImageBytes = 30 * 1024 * 1024;   // 30 MB

    static std::optional<QByteArray> extract(const QString &filePath);
    static bool isSupportedImage(const QString &filePath);
};

EXTRACTOR_PLUGIN_END_NAMESPACE

#endif   // OCREXTRACTOR_H
