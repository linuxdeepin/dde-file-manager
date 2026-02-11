// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTPREVIEWPLUGIN_H
#define TEXTPREVIEWPLUGIN_H

#include "preview_plugin_global.h"
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>

#include "textpreview.h"

namespace plugin_filepreview {
class TextPreviewPlugin : public DFMBASE_NAMESPACE::AbstractFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePreviewFactoryInterface_iid FILE "dde-text-preview-plugin.json")

public:
    DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &key) Q_DECL_OVERRIDE;
};
}
#endif   // TEXTPREVIEWPLUGIN_H
