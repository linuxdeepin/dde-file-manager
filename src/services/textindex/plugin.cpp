// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textindexdbus.h"
#include "ocrindexdbus.h"

#include <dfm-base/utils/processprioritymanager.h>
#include <QDBusConnection>

SERVICETEXTINDEX_USE_NAMESPACE

static TextIndexDBus *textIndexDBus = nullptr;
static OcrIndexDBus *ocrIndexDBus = nullptr;

// DEBUG:
// 1. budild a debug so file and copy to isntall path
// 2. systemctl --user stop deepin-service-plugin@org.deepin.Filemanager.TextIndex.service
// 3. launch app: /usr/bin/deepin-service-manager -n org.deepin.Filemanager.TextIndex

extern "C" int DSMRegister(const char *name, void *data)
{
    Q_UNUSED(name)
    (void)data;
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService(service_textindex::Defines::kTextIndexDBusService)
        && bus.lastError().type() != QDBusError::NoError) {
        fmWarning() << "TextIndex plugin: failed to register text index DBus service:" << bus.lastError().message();
    }

    if (!bus.registerService(service_textindex::Defines::kOcrIndexDBusService)
        && bus.lastError().type() != QDBusError::NoError) {
        fmWarning() << "TextIndex plugin: failed to register OCR index DBus service:" << bus.lastError().message();
    }

    textIndexDBus = new TextIndexDBus();
    ocrIndexDBus = new OcrIndexDBus();
    dfmbase::ProcessPriorityManager::lowerAllAvailablePriorities(true);

    return 0;
}

extern "C" int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    if (ocrIndexDBus) {
        ocrIndexDBus->cleanup();
        ocrIndexDBus->deleteLater();
        ocrIndexDBus = nullptr;
    }

    if (textIndexDBus) {
        textIndexDBus->cleanup();
        textIndexDBus->deleteLater();
        textIndexDBus = nullptr;
    }

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterService(service_textindex::Defines::kOcrIndexDBusService);
    bus.unregisterService(service_textindex::Defines::kTextIndexDBusService);
    return 0;
}
