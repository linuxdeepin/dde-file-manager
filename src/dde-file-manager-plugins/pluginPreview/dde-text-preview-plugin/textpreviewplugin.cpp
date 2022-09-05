// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textpreviewplugin.h"

DFM_USE_NAMESPACE
DFMFilePreview *TextPreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)

    return new TextPreview();
}
