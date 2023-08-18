// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasplugin.h"
#include "canvasmanager.h"
#include "utils/fileutil.h"
#include "canvasdbusinterface.h"

#include <dfm-base/utils/clipboard.h>

#include <QDBusConnection>

using namespace ddplugin_canvas;
DFMBASE_USE_NAMESPACE

void CanvasPlugin::initialize()
{
}

bool CanvasPlugin::start()
{
    // initialize file creator
    DesktopFileCreator::instance();

    //! slow call: GlobalData::onClipboardDataChanged()
    ClipBoard::instance();

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
        qCritical() << "org.deepin.dde.desktop.canvas register object failed" << conn.lastError();
        delete ifs;
    }
}
