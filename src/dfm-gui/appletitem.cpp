// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-gui/appletitem.h>
#include <dfm-gui/sharedqmlengine.h>
#include "applet_p.h"
#include "appletitem_p.h"

#include <QQmlContext>

DFMGUI_BEGIN_NAMESPACE

AppletItemPrivate::AppletItemPrivate(AppletItem *q)
    : q_ptr(q)
{
}

/*!
 * \class AppletItem
 * \brief Applet 对应的 Quick 组件，通过 Applet componentUrl 创建
 */
AppletItem::AppletItem(QQuickItem *parent)
    : QQuickItem(parent), dptr(new AppletItemPrivate(this))
{
    connect(this, &QQuickItem::windowChanged, this, &AppletItem::itemWindowChanged);
}

AppletItem::~AppletItem() { }

Applet *AppletItem::applet() const
{
    return d_func()->applet;
}

void AppletItem::setApplet(Applet *applet)
{
    d_func()->applet = applet;
}

/*!
 * \return 返回当前 Item 是否被初始化完成，目前是对 QQuickItem::isComponentComplete
 *  的封装，用于判断是否被 QQmlEngine 完成创建
 */
bool AppletItem::isCreateComplete() const
{
    Q_D(const AppletItem);
    return isComponentComplete() && d->applet;
}

/*!
 * \return 当前 Item 所在的顶层窗体
 */
QQuickWindow *AppletItem::itemWindow() const
{
    return window();
}

/*!
 * \brief 创建 \a applet componentUrl 对应的 AppletItem，若 componentUrl 指向的
 *      QML 组件不基于 AppletItem ，则返回 nullptr . 类型为 Panel 的 Applet 同样
 *      不会创建(基于QQuickWindow)
 * \return 创建的 AppletItem 指针
 */
AppletItem *AppletItem::itemForApplet(Applet *applet)
{
    if (!applet) {
        return nullptr;
    }

    if (!((Applet::kApplet == applet->flags()) || (Applet::kContainment == applet->flags()))) {
        return nullptr;
    }

    if (applet->rootObject()) {
        return qobject_cast<AppletItem *>(applet);
    }

    if (qApp->closingDown()) {
        return nullptr;
    }

    SharedQmlEngine engine;
    if (!engine.create(applet)) {
        return nullptr;
    }
    if (!engine.completeCreation()) {
        return nullptr;
    }

    QObject *rootObject = engine.rootObject();
    if (rootObject) {
        if (auto item = qobject_cast<AppletItem *>(rootObject)) {
            item->setApplet(applet);
            applet->dptr->setRootObject(item);
            return item;
        }

        rootObject->deleteLater();
    }

    return nullptr;
}

DFMGUI_END_NAMESPACE
