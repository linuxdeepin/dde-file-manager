// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDCIICONPREVIEW_H
#define DDCIICONPREVIEW_H

#include "preview_plugin_global.h"
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>

DFMBASE_BEGIN_NAMESPACE
class AbstractBasePreview;
DFMBASE_END_NAMESPACE

namespace plugin_filepreview {
class DDciIconPreviewPlugin : public DFMBASE_NAMESPACE::AbstractFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePreviewFactoryInterface_iid FILE "dtk-dciicon-preview-plugin.json")

public:
    DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &) override;
};
}   // namespace plugin_filepreview

#endif   // DDCIICONPREVIEW_H
