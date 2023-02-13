// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filepreview.h"
#include "events/fileprevieweventreceiver.h"

#include "dfm-base/base/configs/dconfig/dconfigmanager.h"

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;

void FilePreview::initialize()
{
    FilePreviewEventReceiver::instance()->connectService();
}

bool FilePreview::start()
{
    QString err;
    auto ret = DConfigManager::instance()->addConfig("org.deepin.dde.file-manager.preview", &err);
    if (!ret)
        qWarning() << "create dconfig failed: " << err;
    return true;
}
