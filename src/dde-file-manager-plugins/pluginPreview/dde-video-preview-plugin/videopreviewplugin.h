// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOPREVIEWPLUGIN_H
#define VIDEOPREVIEWPLUGIN_H

#include "dfmfilepreviewplugin.h"
#include "videopreview.h"
#include "libdmr/player_widget.h"

class VideoPreviewPlugin : public DFM_NAMESPACE::DFMFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DFMFilePreviewFactoryInterface_iid FILE "dde-video-preview-plugin.json")

public slots:
    DFM_NAMESPACE::DFMFilePreview *create(const QString &key) override;
};

#endif // VIDEOPREVIEWPLUGIN_H
