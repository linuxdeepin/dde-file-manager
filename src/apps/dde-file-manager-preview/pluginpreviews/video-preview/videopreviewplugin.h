// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOPREVIEWPLUGIN_H
#define VIDEOPREVIEWPLUGIN_H

#include "preview_plugin_global.h"
#include "videopreview.h"
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>

#include <libdmr/player_widget.h>

namespace plugin_filepreview {
class VideoPreviewPlugin : public DFMBASE_NAMESPACE::AbstractFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePreviewFactoryInterface_iid FILE "dde-video-preview-plugin.json")

public slots:
    DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &key) override;
};
}
#endif   // VIDEOPREVIEWPLUGIN_H
