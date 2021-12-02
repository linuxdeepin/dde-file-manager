/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "plugin.h"

DPF_USE_NAMESPACE

/*!
 * \class DiskControlWidget
 *
 * \brief DiskControlWidget is Plug-in interface class, used to implement the plug-in, use the following way:
 * \code
 * class Core : public Plugin
 * {
 *     Q_OBJECT
 *     Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE FILE "core.json")
 * public:
 *     virtual void initialized() override;
 *     virtual bool start() override;
 *     virtual PluginMetaObject::ShutDownFlag stop() override;
 * };
 * \endcode
 * Q_PLUGIN_METADATA see Qt macro definitions
 * PLUGIN_INTERFACE
 */

Plugin::Plugin()
{
}

Plugin::~Plugin()
{
}

/*!
 * \brief Plugin::initialize This function is multi-threaded execution
 * and can be used internally for some thread-safe functions, class operations
 */
void Plugin::initialize()
{
}

/*!
 * \brief Plugin::stop
 * \return Release method
 * Sync (synchronous) and Async (asynchronous) are currently supported
 * If Async is used, then the builder of the plugin should send the signal
 * \code
 * emit asyncStopFinished
 * \endcode
 * Otherwise it will lead to memory leaks or the inability to uninstall the plugin.
 */
Plugin::ShutdownFlag Plugin::stop()
{
    return ShutdownFlag::kSync;
}
