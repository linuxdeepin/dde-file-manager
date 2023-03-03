// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filepreview.h"
#include "events/fileprevieweventreceiver.h"

#include "dfm-base/base/configs/dconfig/dconfigmanager.h"
#include "dfm-base/utils/windowutils.h"

#include <QSurfaceFormat>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;

void FilePreview::initialize()
{
    FilePreviewEventReceiver::instance()->connectService();

    // Under the wayland protocol, set the rendering mode to OpenGLES
    if (WindowUtils::isWayLand()) {
#ifndef __x86_64__
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setDefaultFormat(format);
#endif
    }
}

bool FilePreview::start()
{
    QString err;
    auto ret = DConfigManager::instance()->addConfig("org.deepin.dde.file-manager.preview", &err);
    if (!ret)
        qWarning() << "create dconfig failed: " << err;
    return true;
}
