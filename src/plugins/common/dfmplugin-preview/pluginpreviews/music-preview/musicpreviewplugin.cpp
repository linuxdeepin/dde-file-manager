// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "musicpreviewplugin.h"

DFMBASE_USE_NAMESPACE
using namespace plugin_filepreview;
AbstractBasePreview *MusicPreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)

    return new MusicPreview();
}
