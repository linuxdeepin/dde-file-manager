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
#include "dfm-framework/event/dispatcher/eventdispatcher.h"
#include "dfm-framework/event/unicast/eventunicast.h"   // TODO(zhangs): DEPRECATED
#include "dfm-framework/event/channel/eventchannel.h"
#include "dfm-framework/event/sequence/eventsequence.h"
#include "dfm-framework/service/pluginservicecontext.h"
#include "dfm-framework/log/codetimecheck.h"

#include <QObject>

DPF_BEGIN_NAMESPACE

/*!
 * \brief The Framework class
 */
class DPF_EXPORT Q_DECL_DEPRECATED_X("include <dfm-framework/dpf.h>") Framework
{
    Q_DISABLE_COPY(Framework)
public:
    static Framework &instance();

    Q_DECL_DEPRECATED PluginServiceContext &serviceContext() const;
    Q_DECL_DEPRECATED EventUnicastManager &eventUnicast() const;

private:
    Framework();
};

DPF_END_NAMESPACE

#define dpfInstance ::dpf::Framework::instance()

#endif   // FRAMEWORK_H
