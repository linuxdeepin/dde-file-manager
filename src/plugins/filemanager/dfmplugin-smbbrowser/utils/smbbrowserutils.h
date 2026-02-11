// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBBROWSERUTILS_H
#define SMBBROWSERUTILS_H

#include "dfmplugin_smbbrowser_global.h"
#include "typedefines.h"

#include "dfm-mount/base/dmount_global.h"

#include <QString>
#include <QIcon>
#include <QMutex>

namespace dfmplugin_smbbrowser {

namespace smb_browser_utils {
QString networkScheme();
QUrl netNeighborRootUrl();
QIcon icon();
bool isSmbMounted(const QString &stdSmb);
QString getDeviceIdByStdSmb(const QString &stdSmb);

// service utils
bool isServiceRuning(const QString &service);
bool enableServiceNow(const QString &service);
bool checkAndEnableService(const QString &service);

void initSettingPane();
// bind dconfig
void bindSetting();

// share nodes
QMutex &nodesMutex();
QMap<QUrl, SmbShareNode> &shareNodes();

}
}

#endif   // SMBBROWSERUTILS_H
