// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "markdownpreviewplugin.h"

#include <dfm-base/interfaces/abstractbasepreview.h>

DFMBASE_USE_NAMESPACE
namespace plugin_filepreview {

DFM_LOG_REGISTER_CATEGORY(PREVIEW_NAMESPACE)

AbstractBasePreview *MarkdownPreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)

    return new MarkdownPreview();
}
}   //  namespace plugin_filepreview
