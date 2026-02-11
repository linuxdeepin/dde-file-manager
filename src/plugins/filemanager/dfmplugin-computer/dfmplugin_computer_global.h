// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_COMPUTER_GLOBAL_H
#define DFMPLUGIN_COMPUTER_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DPCOMPUTER_NAMESPACE dfmplugin_computer

#define DPCOMPUTER_BEGIN_NAMESPACE namespace DPCOMPUTER_NAMESPACE {
#define DPCOMPUTER_END_NAMESPACE }
#define DPCOMPUTER_USE_NAMESPACE using namespace DPCOMPUTER_NAMESPACE;

#include <QIcon>
#include <QUrl>

DPCOMPUTER_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DPCOMPUTER_NAMESPACE)

struct DeviceInfo
{
    QIcon icon;
    QUrl deviceUrl;
    QUrl mountPoint;
    QString deviceName;
    QString deviceType;
    QString fileSystem;
    qint64 totalCapacity;
    qint64 availableSpace;
    QString deviceDesc;
};

DPCOMPUTER_END_NAMESPACE
#endif   // DFMPLUGIN_COMPUTER_GLOBAL_H
