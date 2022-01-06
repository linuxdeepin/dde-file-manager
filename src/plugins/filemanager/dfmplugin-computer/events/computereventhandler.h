/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef COMPUTEREVENTHANDLER_H
#define COMPUTEREVENTHANDLER_H

#include "dfmplugin_computer_global.h"
#include <dfm-framework/event/pubsub/eventhandler.h>
#include <dfm-framework/event/pubsub/eventcallproxy.h>

DPCOMPUTER_BEGIN_NAMESPACE

class ComputerEventHandler final : public dpf::EventHandler, dpf::AutoEventHandlerRegister<ComputerEventHandler>
{
    Q_OBJECT

public:
    explicit ComputerEventHandler(QObject *parent = nullptr);

    static EventHandler::Type type();
    static QStringList topics();

    // EventHandler interface
public:
    void eventProcess(const dpf::Event &);
};
DPCOMPUTER_END_NAMESPACE
#endif   // COMPUTEREVENTHANDLER_H
