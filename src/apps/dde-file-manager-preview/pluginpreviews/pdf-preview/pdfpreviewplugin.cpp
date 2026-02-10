// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pdfpreviewplugin.h"

DFMBASE_USE_NAMESPACE
namespace plugin_filepreview {
DFM_LOG_REGISTER_CATEGORY(PREVIEW_NAMESPACE)
AbstractBasePreview *PDFPreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)

    return new PDFPreview();
}
}   // namespace plugin_filepreview
