// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-gui/applet.h>
#include <dfm-gui/appletitem.h>
#include <dfm-gui/containment.h>
#include <dfm-gui/panel.h>

#include "applet_p.h"
#include "containment_p.h"

#include <QJSEngine>

DFMGUI_BEGIN_NAMESPACE

AppletPrivate::AppletPrivate(Applet *q)
    : q_ptr(q)
{
}

AppletPrivate::~AppletPrivate()
{
    // 释放关联的 QQuickItem
    if (rootObject) {
        QObject::disconnect(rootObject, nullptr, q_ptr, nullptr);
        if (QJSEngine::CppOwnership == QJSEngine::objectOwnership(rootObject)) {
            const bool managed = bool(rootObject->parent() == q_ptr);
            if (!managed) {
                qCCritical(logDFMGui) << "Undefined behaviour, unmanaged QQuickItem:";
                q_ptr->dumpAppletTree();
            }
            Q_ASSERT_X(managed, "AppletItem memory management", "Undefined behaviour, unmanaged QQuickItem");

            rootObject->deleteLater();
        }
    }
}

/*!
 * \brief 设置 Applet 关联的 QML 组件 \a item ，仅可设置一次
 */
void AppletPrivate::setRootObject(QObject *item)
{
    Q_ASSERT(nullptr == rootObject && nullptr != item);
    rootObject = item;
    setState(Applet::kReady);

    Q_EMIT q_func()->rootObjectChanged(item);
}

void AppletPrivate::setState(Applet::State s)
{
    if (s == state) {
        state = s;
        Q_EMIT q_ptr->stateChanged(state);
    }
}
/*!
 * \brief \a engine 引擎已加载创建，完成组件创建并更新当前 Applet 的 QML 组件
 */
bool AppletPrivate::createComplete(SharedQmlEngine *engine)
{
    Q_Q(Applet);
    if (engine->completeCreation()) {
        if (QObject *rootObject = engine->rootObject()) {
            switch (flag) {
            case Applet::kApplet:
                Q_FALLTHROUGH();
            case Applet::kContainment:
                if (auto *item = qobject_cast<AppletItem *>(rootObject)) {
                    // 先设置关联，再抛出信号
                    item->setApplet(q);
                    setRootObject(item);
                    return true;
                }

                break;
            case Applet::kPanel:
                if (auto *window = qobject_cast<QQuickWindow *>(rootObject)) {
                    setRootObject(window);
                    return true;
                }
                break;
            default:
                break;
            }

            rootObject->deleteLater();
        }
    }

    setState(Applet::kError);
    return false;
}

/*!
 * \brief 按层级递归打印当前 Applet 树信息
 */
void AppletPrivate::dumpAppletTreeImpl(int level)
{
    QString indent = QString(' ').repeated(level * 4);
    if (metaPtr) {
        qCDebug(logDFMGui) << qPrintable(indent) << metaPtr;
    } else {
        Q_Q(Applet);
        qCDebug(logDFMGui) << qPrintable(indent) << "(Dynamic applet):"
                           << q << q->rootObject() << q->componentUrl();
    }

    if (flag.testFlag(Applet::kContainment)) {
        if (auto *containment = dynamic_cast<ContainmentPrivate *>(this)) {
            qCDebug(logDFMGui) << indent << QStringLiteral("Applet chiledren:");
            for (Applet *child : containment->applets) {
                child->dptr->dumpAppletTreeImpl(level + 1);
            }
        }
    }
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
 * \return 返回当前QML组件加载状态
 */
Applet::State Applet::state() const
{
    return d_func()->state;
}

/*!
 * \brief 创建 Applet 对应的 Qml 组件
 * \return 是否成功调用，若为同步调用，通过 rootObject() 取得对象；若为异步调用，
 *  通过 rootObjectChanged() 判断是否创建成功。
 *
 * \code
 *      // 同步创建使用
 *      if (applet->createRootObject(false)) {
 *          QObject rootObject = applet->rootObject();
 *      }
 *
 *      // 异步创建使用
 *      if (applet->createRootObject(true)) {
 *          connect(applet, &Applet::stateChanged, [](Applet::state s){
 *              if (Applet::kError = s) { ... }
 *          });
 *          connect(applet, &Applet::rootObjectChanged, []()(QObject *obj) {
 *              if (obj) {
 *                  // 抛送至 QML ...
 *              }
 *          });
 *      }
 * \endcode
 */
bool Applet::createRootObject(bool async)
{
    if (kNull != state()) {
        return false;
    }

    Q_D(Applet);
    d->setState(Applet::kLoading);
    if (async) {
        SharedQmlEngine *asyncEngine = new SharedQmlEngine(this);
        QObject::connect(asyncEngine, &SharedQmlEngine::createFinished, [asyncEngine, this](bool success) {
            if (success) {
                d_func()->createComplete(asyncEngine);
            } else {
                d_func()->setState(Applet::kError);
            }

            asyncEngine->deleteLater();
        });

        asyncEngine->create(this, true);
        return true;
    }

    // 同步创建
    SharedQmlEngine tmpEngine;
    tmpEngine.create(this, false);
    return d->createComplete(&tmpEngine);
}

/*!
 * \return Applet 对应的 QML 组件
 */
QObject *Applet::rootObject() const
{
    return d_func()->rootObject;
}

/*!
 * \brief 返回当前 Applet 的容器 Containment , 会递归向上查找
 * \return 容器指针，若父对象没有容器，返回 nullptr
 */
Containment *Applet::containment() const
{
    QObject *parent = this->parent();
    while (parent) {
        if (auto *contain = qobject_cast<Containment *>(parent)) {
            return contain;
        }
        parent = parent->parent();
    }

    return nullptr;
}

/*!
 * \brief 返回当前 Applet 的顶层 Panel , 会递归向上查找
 * \return Panel 指针，若没有被 Panel 管理，返回 nullptr
 */
Panel *Applet::panel() const
{
    QObject *parent = this->parent();
    while (parent) {
        if (auto *panel = qobject_cast<Panel *>(parent)) {
            return panel;
        }
        parent = parent->parent();
    }

    return nullptr;
}

/*!
 * \return 返回 Applet 对应的插件名称，可用于获取信息创建 Applet
 */
QString Applet::plugin() const
{
    Q_D(const Applet);
    return (d->metaPtr) ? d->metaPtr->plugin() : QString();
}

/*!
 * \return 返回 Applet 对应的组件ID，可用于获取信息创建 Applet
 */
QString Applet::id() const
{
    Q_D(const Applet);
    return (d->metaPtr) ? d->metaPtr->id() : QString();
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

/*!
 * \brief 打印当前 Applet 树信息
 */
void Applet::dumpAppletTree()
{
    d_func()->dumpAppletTreeImpl();
}

DFMGUI_END_NAMESPACE
