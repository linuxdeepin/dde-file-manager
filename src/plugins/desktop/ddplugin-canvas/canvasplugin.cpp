// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasplugin.h"
#include "canvasmanager.h"
#include "utils/fileutil.h"
#include "canvasdbusinterface.h"
#include "dragmonitor.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QDBusConnection>

namespace ddplugin_canvas {
DFM_LOG_REGISTER_CATEGORY(DDP_CANVAS_NAMESPACE)

DFMBASE_USE_NAMESPACE

void CanvasPlugin::initialize()
{
    QString err;
    DConfigManager::instance()->addConfig("org.deepin.dde.file-manager.desktop.sys-watermask", &err);
    fmInfo() << "register desktop dconfig:" << err;
}

bool CanvasPlugin::start()
{
    // initialize file creator
    DesktopFileCreator::instance();

    proxy = new CanvasManager();
    proxy->init();

    registerDBus();
    return true;
}

void CanvasPlugin::stop()
{
    delete proxy;
    proxy = nullptr;
}

void CanvasPlugin::registerDBus()
{
    Q_ASSERT(proxy);
    auto ifs = new CanvasDBusInterface(proxy);

    QDBusConnection conn = QDBusConnection::sessionBus();
    auto registerOptions = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    if (!conn.registerObject("/org/deepin/dde/desktop/canvas", "org.deepin.dde.desktop.canvas", ifs, registerOptions)) {
        fmCritical() << "org.deepin.dde.desktop.canvas register object failed" << conn.lastError();
        delete ifs;
    } else {
        auto mo = new dfm_drag::DragMoniter;
        mo->connect(mo, &dfm_drag::DragMoniter::dragEnter, ifs, &CanvasDBusInterface::DragEnter);
        mo->start();
    }
}
}   // namespace ddplugin_canvas
