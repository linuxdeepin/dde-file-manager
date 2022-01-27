/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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
#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "dfm-framework/dfm_framework_global.h"
#include "dfm-framework/lifecycle/lifecycle.h"
#include "dfm-framework/listener/listener.h"
#include "dfm-framework/event/pubsub/eventcallproxy.h"
#include "dfm-framework/event/dispatcher/eventdispatcher.h"
#include "dfm-framework/event/unicast/eventunicast.h"
#include "dfm-framework/service/pluginservicecontext.h"

#include <QObject>

DPF_BEGIN_NAMESPACE

class FrameworkPrivate;

/*!
 * \brief The Framework class
 */
class DPF_EXPORT Framework
{
    Q_DISABLE_COPY(Framework)
public:
    static Framework &instance();

    bool initialize();
    bool start();
    const LifeCycle &lifeCycle() const;
    [[gnu::hot]] PluginServiceContext &serviceContext() const;
    EventCallProxy &eventProxy() const;
    [[gnu::hot]] EventDispatcherManager &eventDispatcher() const;
    [[gnu::hot]] EventUnicastManager &eventUnicast() const;
    const Listener &listener() const;

private:
    Framework();

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d), Framework)
    QScopedPointer<FrameworkPrivate> d;
};

DPF_END_NAMESPACE

#define dpfInstance ::dpf::Framework::instance()

#endif   // FRAMEWORK_H
