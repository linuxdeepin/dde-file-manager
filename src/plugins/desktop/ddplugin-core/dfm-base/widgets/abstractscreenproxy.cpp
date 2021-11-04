/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#include "abstractscreenproxy.h"

DFMBASE_BEGIN_NAMESPACE
/*!
 * \brief The AbstractScreenProxy class
 * 屏幕代理创建类，可实现但不限于平台xcb与wayland
 * 此类为扩展类型，接口最终会在ScreenService中集成并使用
 */

/*!
 * \brief AbstractScreenProxy::AbstractScreenProxy
 * \param parent
 */
AbstractScreenProxy::AbstractScreenProxy(QObject *parent) : QObject(parent)
{

}

/*!
 * \brief AbstractScreenProxy::allScreen 获取所有的屏幕对象接口
 *  实现但不限于平台xcb与wayland重载实现的屏幕获取
 * \return
 */
QList<dfmbase::AbstractScreen *> AbstractScreenProxy::allScreen()
{
    return {};
}
DFMBASE_END_NAMESPACE
