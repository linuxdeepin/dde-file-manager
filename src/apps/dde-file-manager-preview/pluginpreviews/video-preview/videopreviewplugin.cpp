// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videopreviewplugin.h"

DFMBASE_USE_NAMESPACE
namespace plugin_filepreview {
DFM_LOG_REGISTER_CATEGORY(PREVIEW_NAMESPACE)

AbstractBasePreview *VideoPreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)

    return new VideoPreview();
}
}   // namespace plugin_filepreview
