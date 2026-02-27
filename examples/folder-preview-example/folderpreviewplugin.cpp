// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "folderpreviewplugin.h"
#include "folderpreview.h"

using namespace example_folderprev;
DFMBASE_USE_NAMESPACE

FolderPreviewPlugin::FolderPreviewPlugin(QObject *parent)
    : AbstractFilePreviewPlugin(parent)
{
}

AbstractBasePreview *FolderPreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)
    return new FolderPreview(this);
}
