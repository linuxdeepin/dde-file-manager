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

#include "computerdatastruct.h"

DPCOMPUTER_BEGIN_NAMESPACE
namespace ContextMenuActionTrs {
QString trOpenInNewWin()
{
    return QObject::tr("Open in new window");
}
QString trOpenInNewTab()
{
    return QObject::tr("Open in new tab");
}
QString trMount()
{
    return QObject::tr("Mount");
}
QString trUnmount()
{
    return QObject::tr("Unmount");
}
QString trRename()
{
    return QObject::tr("Rename");
}
QString trEject()
{
    return QObject::tr("Eject");
}
QString trSafelyRemove()
{
    return QObject::tr("Safely Remove");
}
QString trProperties()
{
    return QObject::tr("Properties");
}
QString trFormat()
{
    return QObject::tr("Format");
}
QString trRemove()
{
    return QObject::tr("Remove");
}
QString trLogoutAndClearSavedPasswd()
{
    return QObject::tr("Clear saved password and unmount");
}
QString trOpen()
{
    return QObject::tr("Open");
}
}   // namespace ContextMenuActionTrs
DPCOMPUTER_END_NAMESPACE
