// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewhelper.h"

#include "dfm-base/base/configs/dconfig/dconfigmanager.h"

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;

PreviewHelper *PreviewHelper::instance()
{
    static PreviewHelper instance;
    return &instance;
}

bool PreviewHelper::isPreviewEnabled()
{
    const auto &&ret = DConfigManager::instance()->value("org.deepin.dde.file-manager.preview", "previewEnable");
    return ret.isValid() ? ret.toBool() : true;
}

PreviewHelper::PreviewHelper(QObject *parent)
    :QObject (parent)
{
}



