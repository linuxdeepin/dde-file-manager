// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-gui/containment.h>
#include <dfm-gui/appletmanager.h>
#include "containment_p.h"

#include <QQuickItem>
#include <QQuickWindow>

DFMGUI_BEGIN_NAMESPACE

ContainmentPrivate::ContainmentPrivate(Containment *q)
    : AppletPrivate(q)
{
    flag = Applet::kContainment;
}

void ContainmentPrivate::setRootObject(QObject *item)
{
    for (auto *applet : applets) {
        // 管理部分在父组件完成创建前初始化的子组件
        appletRootObjectCreated(applet->rootObject());
    }

    AppletPrivate::setRootObject(item);
}

/*!
 * \brief 设置子 Applet QML控件 \a appletObject 的父控件为当前 Containment 的QML控件
 */
void ContainmentPrivate::appletRootObjectCreated(QObject *appletObject)
{
    if (!appletObject || !rootObject) {
        return;
    }

    if (rootObject != appletObject->parent()) {
        appletObject->setParent(rootObject);
        Q_EMIT q_func()->appletRootObjectChanged(appletObject);
    }
}

/*!
 * \class Containment
 * \brief 容器，管理多个 Applet
 * \section QML 组件管理
 *  Applet 创建的 QML 组件也由其管理，当子 Applet 的 AppletItem 组件创建时，AppletItem
 *  组件父节点将自动设置为容器 QML 组件。
 *  当移除子 Applet 时，AppletItem 组件将设置父类为子 Applet ，在销毁 Applet 时，若
 *  AppletItem 组件关联在 Applet 上，会自动析构对应的 AppletItem.
 */
Containment::Containment(QObject *parent)
    : Applet(*new ContainmentPrivate(this), parent)
{
}

Containment::Containment(ContainmentPrivate &dd, QObject *parent)
    : Applet(dd, parent)
{
}

Containment::~Containment() { }

/*!
 * \brief 根据 QQuick 组件元信息 \a metaPtr 创建并追加对应的子 Applet
 * \return 创建的子 Applet , 若创建失败则返回 nullptr
 */
Applet *Containment::createApplet(const dpf::PluginQuickMetaPtr &metaPtr)
{
    Applet *applet = AppletManager::instance()->createAppletFromInfo(metaPtr);
    if (applet) {
        appendApplet(applet);
    }

    return applet;
}

/*!
 * \brief 在当前容器中添加 \a applet , 不会自动创建对应的 QQuickItem
 */
void Containment::appendApplet(Applet *applet)
{
    Q_D(Containment);
    if (applet && !d->applets.contains(applet)) {
        if (Containment *oldContain = applet->containment()) {
            oldContain->removeApplet(applet);
        }

        // 已创建的 QQuickItem 发出信号
        d->appletRootObjectCreated(applet->rootObject());
        // 后续创建的绑定信号
        connect(applet, &Applet::rootObjectChanged, this, [this](QObject *object) {
            d_func()->appletRootObjectCreated(object);
        });

        applet->setParent(this);
        d->applets.append(applet);

        Q_EMIT appletAdded(applet);
        Q_EMIT appletsChanged();
    }
}

/*!
 * \brief 在当前容器移除 \a applet ，对应的 QQuickItem 父对象被设置为 \a applet ，
 *      此函数不会销毁 \a applet ，注意生命周期管理
 */
void Containment::removeApplet(Applet *applet)
{
    Q_D(Containment);
    if (d->applets.contains(applet)) {
        disconnect(applet, nullptr, this, nullptr);
        applet->setParent(nullptr);

        if (applet->rootObject()) {
            switch (applet->flags()) {
            case Applet::kApplet:
                Q_FALLTHROUGH();
            case Applet::kContainment:
                if (auto item = qobject_cast<QQuickItem *>(applet->rootObject())) {
                    item->setVisible(false);
                }
                break;
            case Applet::kPanel:
                if (auto window = qobject_cast<QQuickWindow *>(applet->rootObject())) {
                    window->setVisible(false);
                }
                break;
            default:
                break;
            }

            // 调整父节点为 Applet
            applet->rootObject()->setParent(applet);
        }

        d->applets.removeOne(applet);

        Q_EMIT appletRemoved(applet);
        Q_EMIT appletsChanged();
    }
}

/*!
 * \return 返回子 Applet 列表
 */
QList<Applet *> Containment::applets() const
{
    return d_func()->applets;
}

/*!
 * \return 返回存在属性 \a property 值为 \a var 的 Applet
 */
Applet *Containment::appletForProperty(const QString &property, const QVariant &var)
{
    for (Applet *child : applets()) {
        if (child && child->property(property.toStdString().c_str()) == var) {
            return child;
        }
    }

    return nullptr;
}

/*!
 * \return 返回存在属性 \a property 值为 \a var 的 Applet 对应的 Qml 组件
 */
QObject *Containment::objectForProperty(const QString &property, const QVariant &var)
{
    for (Applet *child : applets()) {
        if (QObject *item = child->rootObject()) {
            if (item->property(property.toStdString().c_str()) == var) {
                return item;
            }
        }
    }

    return nullptr;
}

DFMGUI_END_NAMESPACE
