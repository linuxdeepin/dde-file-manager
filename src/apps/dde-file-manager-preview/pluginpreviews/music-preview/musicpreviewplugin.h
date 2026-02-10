// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MUSICPREVIEWPLUGIN_H
#define MUSICPREVIEWPLUGIN_H

#include "preview_plugin_global.h"
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>
#include "musicpreview.h"

namespace plugin_filepreview {
class MusicPreviewPlugin : public DFMBASE_NAMESPACE::AbstractFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePreviewFactoryInterface_iid FILE "dde-music-preview-plugin.json")

public:
    DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &key) Q_DECL_OVERRIDE;
};
}
#endif   // MUSICPREVIEWPLUGIN_H
