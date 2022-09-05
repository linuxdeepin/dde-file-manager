// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videopreviewplugin.h"

DFM_USE_NAMESPACE

DFMFilePreview *VideoPreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)

    return new VideoPreview();
}
