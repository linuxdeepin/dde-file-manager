// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computerdatastruct.h"

namespace dfmplugin_computer {
namespace ContextMenuAction {

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
QString trLogoutAndClearSavedPasswd()
{
    return QObject::tr("Clear saved password and unmount");
}
QString trOpen()
{
    return QObject::tr("Open");
}
QString trErase()
{
    return QObject::tr("Erase");
}

}   // namespace ContextMenuActionTrs
}
