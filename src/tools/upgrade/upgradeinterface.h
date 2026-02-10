// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPGRADEINTERFACE_H
#define UPGRADEINTERFACE_H

#include <QMap>

extern "C" {
int dfm_tools_upgrade_doUpgrade(const QMap<QString, QString> &args);
int dfm_tools_upgrade_doRestart(const QMap<QString, QString> &args);
}

#endif   // UPGRADEINTERFACE_H
