// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "operationsstackmanagerdbus.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-framework/dpf.h>

static constexpr uint8_t kMaxStep { 2 };

OperationsStackManagerDbus::OperationsStackManagerDbus(QObject *parent)
    : QObject(parent)
{
}

void OperationsStackManagerDbus::SaveOperations(const QVariantMap &values)
{
    while (fileOperations.size() >= kMaxStep)
        fileOperations.pop_front();

    fileOperations.push(values);
}

void OperationsStackManagerDbus::CleanOperations()
{
    fileOperations.clear();
}

QVariantMap OperationsStackManagerDbus::RevocationOperations()
{
    if (fileOperations.count() > 0)
        return fileOperations.pop();

    return QVariantMap();
}
