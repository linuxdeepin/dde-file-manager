/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "operationsstackmanagerdbus.h"

#include "dfm-base/utils/universalutils.h"
#include "dfm-base/dbusservice/global_server_defines.h"

#include <dfm-framework/framework.h>

OperationsStackManagerDbus::OperationsStackManagerDbus(QObject *parent)
    : QObject(parent)
{
}

OperationsStackManagerDbus::~OperationsStackManagerDbus()
{
}

void OperationsStackManagerDbus::SaveOperations(const QVariantMap &values)
{
    QMutexLocker lk(&lock);
    fileOperations.push(values);
}

void OperationsStackManagerDbus::CleanOperations()
{
    QMutexLocker lk(&lock);
    fileOperations.clear();
}

QVariantMap OperationsStackManagerDbus::RevocationOperations()
{
    QMutexLocker lk(&lock);
    if (fileOperations.count() > 0)
        return fileOperations.pop();

    return QVariantMap();
}
