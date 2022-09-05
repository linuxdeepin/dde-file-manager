// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MUSICPREVIEWPLUGIN_H
#define MUSICPREVIEWPLUGIN_H

#include "dfmfilepreviewplugin.h"
#include "musicpreview.h"

class MusicPreviewPlugin : public DFM_NAMESPACE::DFMFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DFMFilePreviewFactoryInterface_iid FILE "dde-music-preview-plugin.json")

public:
    DFM_NAMESPACE::DFMFilePreview *create(const QString &key) Q_DECL_OVERRIDE;
};

#endif // MUSICPREVIEWPLUGIN_H
