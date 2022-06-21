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
#include "framework.h"
#include "backtrace/backtrace.h"
#include "dfm-framework/lifecycle/lifecycle.h"

DPF_BEGIN_NAMESPACE

/*!
 * \brief Get framework instance.
 * \return
 */
Framework &Framework::instance()
{
    static Framework ins;
    return ins;
}

/*!
 * \brief Get plugin service context
 * \return
 */
PluginServiceContext &Framework::serviceContext() const
{
    return PluginServiceContext::instance();
}

EventUnicastManager &Framework::eventUnicast() const
{
    return EventUnicastManager::instance();
}

Framework::Framework()
{
}

DPF_END_NAMESPACE
