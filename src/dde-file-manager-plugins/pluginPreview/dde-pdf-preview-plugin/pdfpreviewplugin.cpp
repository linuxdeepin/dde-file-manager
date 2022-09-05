// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pdfpreviewplugin.h"

DFM_USE_NAMESPACE
DFMFilePreview *PDFPreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)

    return new PDFPreview();
}
