// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ROFDPREVIEWPLUGIN_H
#define ROFDPREVIEWPLUGIN_H
#include "preview_plugin_global.h"
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>

#include "rofdpreview.h"

namespace plugin_filepreview {
class RofdPreviewPlugin : public DFMBASE_NAMESPACE::AbstractFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePreviewFactoryInterface_iid FILE "dde-rofd-preview-plugin.json")

public:
    DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &key) Q_DECL_OVERRIDE;
};
}
#endif   // ROFDPREVIEWPLUGIN_H
