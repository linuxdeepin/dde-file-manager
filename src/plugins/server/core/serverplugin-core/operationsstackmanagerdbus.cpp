// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "operationsstackmanagerdbus.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-framework/dpf.h>
static constexpr uint8_t kMaxStep { 100 };
OperationsStackManagerDbus::OperationsStackManagerDbus(QObject *parent)
    : QObject(parent)
{
}

void OperationsStackManagerDbus::SaveOperations(const QVariantMap &values)
{
    while (fileOperations.size() >= kMaxStep)
        fileOperations.pop_front();

    bool back = true;
    auto op = values;
    if (op.contains("stackBack")) {
        back = values.value("stackBack", true).toBool();
        op.remove("stackBack");
    }
    if (back) {
        fileOperations.push(op);
    } else {
        fileOperations.push_front(op);
    }
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

void OperationsStackManagerDbus::SaveRedoOperations(const QVariantMap &values)
{
    while (redoFileOperations.size() >= kMaxStep)
        redoFileOperations.pop_front();

    bool back = true;
    auto op = values;
    if (op.contains("stackBack")) {
        back = values.value("stackBack", true).toBool();
        op.remove("stackBack");
    }
    if (back) {
        redoFileOperations.push(op);
    } else {
        redoFileOperations.push_front(op);
    }
}

QVariantMap OperationsStackManagerDbus::RevocationRedoOperations()
{
    if (redoFileOperations.count() > 0)
        return redoFileOperations.pop();

    return QVariantMap();
}

void OperationsStackManagerDbus::CleanOperationsByUrl(const QStringList &urls)
{

}
