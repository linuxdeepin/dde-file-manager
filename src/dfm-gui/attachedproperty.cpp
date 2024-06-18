// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "attachedproperty_p.h"

#include <dfm-gui/appletitem.h>

#include <QQmlContext>

DFMGUI_BEGIN_NAMESPACE

/*!
 * \class AppletAttached
 * \brief 用于 Quick 组件 AppletItem 在 QML 中访问对应的附加 Applet 属性
 *      使用 qmlRegisterExtendedType<Applet, AppletAttached>(uri, 1, 0, "Applet") 注册
 */
AppletAttached::AppletAttached(QObject *parent)
    : QObject(parent)
{
}

AppletAttached::~AppletAttached() { }

Applet *AppletAttached::qmlAttachedProperties(QObject *object)
{
    if (auto context = qmlContext(object)) {
        return context->contextProperty("_dfm_applet").value<Applet *>();
    }

    return nullptr;
}

/*!
 * \class ContainmentAttached
 * \brief 用于 Quick 组件 ContainmentItem 在 QML 中访问对应的附加 Containment 属性
 */
ContainmentAttached::ContainmentAttached(QObject *parent)
    : QObject(parent)
{
}

ContainmentAttached::~ContainmentAttached() { }

Containment *ContainmentAttached::qmlAttachedProperties(QObject *object)
{
    auto contain = qobject_cast<Containment *>(AppletAttached::qmlAttachedProperties(object));
    return contain;
}

DFMGUI_END_NAMESPACE
