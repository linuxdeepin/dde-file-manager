/**
 * Copyright (C) 2020 UOS Technology Co., Ltd.
 *
 * to mark the desktop UI
 **/

#ifndef DDE_FILE_MANAGER_ACCESSIBLE_LIST_H
#define DDE_FILE_MANAGER_ACCESSIBLE_LIST_H

#include "acframefunctions.h"

#include <QWidget>


// 添加accessible
SET_FORM_ACCESSIBLE(QWidget,m_w->accessibleName())

QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;

    USE_ACCESSIBLE(classname, QWidget);

    return interface;
}

#endif // DDE_FILE_MANAGER_ACCESSIBLE_LIST_H
