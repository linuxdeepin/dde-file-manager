/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef COMPUTERUNICASTRECEIVER_H
#define COMPUTERUNICASTRECEIVER_H

#include "dfmplugin_computer_global.h"

#include <dfm-framework/framework.h>

#include <QObject>

DPCOMPUTER_BEGIN_NAMESPACE

class ComputerUnicastReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ComputerUnicastReceiver)

public:
    static ComputerUnicastReceiver *instance();
    void connectService();

protected:
    void doAddDevice(const QString &name, const QUrl &url);
    void doRemoveDevice(const QUrl &url);

private:
    explicit ComputerUnicastReceiver(QObject *parent = nullptr);
};

DPCOMPUTER_END_NAMESPACE

#endif   // COMPUTERUNICASTRECEIVER_H
