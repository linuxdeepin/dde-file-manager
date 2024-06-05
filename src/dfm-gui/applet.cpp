// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-gui/applet.h>
#include <dfm-gui/containment.h>
#include "applet_p.h"

#include <QJSEngine>

DFMGUI_BEGIN_NAMESPACE

AppletPrivate::AppletPrivate(Applet *q)
    : q_ptr(q)
{
}

AppletPrivate::~AppletPrivate()
{
    // 释放关联的 QQuickItem
    if (rootObject && QJSEngine::CppOwnership == QJSEngine::objectOwnership(rootObject)) {
        rootObject->deleteLater();
    }
}

/*!
 * \brief 设置 Applet 关联的 QML 组件 \a item ，仅可设置一次
 */
void AppletPrivate::setRootObject(QObject *item)
{
    Q_ASSERT(nullptr == rootObject);
    rootObject = item;

    Q_EMIT q_func()->rootObjectChanged(item);
}

/*!
 * \class Applet
 * \brief 管理插件 QML 组件的最小单元，包含基础的组件信息.
 * \details 使用此类在 C++ 代码中管理小组件，插件可派生 Applet 并注册对应的构造函数(SchemeFactory)。
 *      设计参考自 plasma-framework 和 dde-shell
 *
 * \sa AppletItem
 */
Applet::Applet(QObject *parent)
    : QObject(parent), dptr(new AppletPrivate(this))
{
}

Applet::Applet(AppletPrivate &dd, QObject *parent)
    : QObject(parent), dptr(&dd)
{
}

Applet::~Applet() { }

/*!
 * \return 当前操作的文件路径
 */
QUrl Applet::currentUrl() const
{
    return d_func()->currentUrl;
}

/*!
 * \brief 设置当前操作的文件路径 \a url
 */
void Applet::setCurrentUrl(const QUrl &url)
{
    Q_D(Applet);
    if (d->currentUrl != url) {
        d->currentUrl = url;
        Q_EMIT currentUrlChanged(url);
    }
}

/*!
 * \return Applet 标识，按派生类型分为
 *  Applet 普通组件
 *  Containment 容器组件
 *  Panel 面板，用于顶层窗体
 */
Applet::Flags Applet::flags() const
{
    return d_func()->flag;
}

/*!
 * \return Applet 对应的 QML 组件，
 */
QObject *Applet::rootObject() const
{
    return d_func()->rootObject;
}

/*!
 * \brief 返回当前 Applet 的容器 Containment , 若自身是容器则会返回当前对象
 * \return 容器指针
 */
Containment *Applet::containment() const
{
    Containment *contain = qobject_cast<Containment *>(const_cast<Applet *>(this));
    if (contain) {
        return contain;
    }
    contain = nullptr;

    QObject *parent = this->parent();

    while (parent) {
        Containment *tmp = qobject_cast<Containment *>(parent);
        if (tmp) {
            contain = tmp;
            break;
        }
        parent = parent->parent();
    }

    return contain;
}

/*!
 * \return 返回 Applet 对应的插件名称，可用于获取信息创建 Applet
 */
QString Applet::plugin() const
{
    return d_func()->metaPtr->plugin();
}

/*!
 * \return 返回 Applet 对应的组件ID，可用于获取信息创建 Applet
 */
QString Applet::id() const
{
    return d_func()->metaPtr->id();
}

/**
 * @return 当前 Applet 对应的 QML 组件文件路径
 */
QUrl Applet::componentUrl() const
{
    return d_func()->componentUrl;
}

/*!
 * \brief 设置当前 Applet 的 QML 组件文件为 \a url
 */
void Applet::setComponentUrl(const QUrl &url)
{
    Q_D(Applet);
    if (d->componentUrl != url) {
        d->componentUrl = url;
        Q_EMIT componentUrlChanged(url);
    }
}

DFMGUI_END_NAMESPACE
