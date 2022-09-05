// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOP_ACCESSIBLE_OBJECT_LIST_H
#define DESKTOP_ACCESSIBLE_OBJECT_LIST_H

#include "acframefunctions.h"

// 添加accessible

SET_FORM_ACCESSIBLE(QWidget,m_w->objectName())

QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;
    USE_ACCESSIBLE(classname, QWidget);

    return interface;
}

#endif // DESKTOP_ACCESSIBLE_OBJECT_LIST_H
