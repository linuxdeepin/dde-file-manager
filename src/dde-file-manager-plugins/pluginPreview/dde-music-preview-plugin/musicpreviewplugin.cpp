// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "musicpreviewplugin.h"

DFM_USE_NAMESPACE

DFMFilePreview *MusicPreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)

    return new MusicPreview();
}
