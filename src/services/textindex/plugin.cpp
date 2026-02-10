// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textindexdbus.h"
#include "utils/processprioritymanager.h"

static TextIndexDBus *textIndexDBus = nullptr;

// DEBUG:
// 1. budild a debug so file and copy to isntall path
// 2. systemctl --user stop deepin-service-plugin@org.deepin.Filemanager.TextIndex.service
// 3. launch app: /usr/bin/deepin-service-manager -n org.deepin.Filemanager.TextIndex

extern "C" int DSMRegister(const char *name, void *data)
{
    (void)data;
    textIndexDBus = new TextIndexDBus(name);
    service_textindex::ProcessPriorityManager::lowerAllAvailablePriorities(true);

    return 0;
}

extern "C" int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    textIndexDBus->cleanup();
    textIndexDBus->deleteLater();
    textIndexDBus = nullptr;
    return 0;
}
