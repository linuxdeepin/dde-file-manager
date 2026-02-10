// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEPREVIEWPLUGIN_H
#define IMAGEPREVIEWPLUGIN_H

#include "preview_plugin_global.h"
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>
#include "imagepreview.h"

namespace plugin_filepreview {
class ImagePreviewPlugin : public DFMBASE_NAMESPACE::AbstractFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePreviewFactoryInterface_iid FILE "dde-image-preview-plugin.json")

public slots:
    DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &key) Q_DECL_OVERRIDE;
};
}
#endif   // IMAGEPREVIEWPLUGIN_H
