// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MARKDOWNPREVIEWPLUGIN_H
#define MARKDOWNPREVIEWPLUGIN_H

#include "preview_plugin_global.h"
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>

#include "markdownpreview.h"

namespace plugin_filepreview {
class MarkdownPreviewPlugin : public DFMBASE_NAMESPACE::AbstractFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePreviewFactoryInterface_iid FILE "dde-markdown-preview-plugin.json")

public:
    DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &key) Q_DECL_OVERRIDE;
};
}
#endif   // MARKDOWNPREVIEWPLUGIN_H
