// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-gui/windowmanager.h>
#include <dfm-gui/appletitem.h>
#include <dfm-gui/containment.h>
#include <dfm-gui/containmentitem.h>
#include <dfm-gui/panel.h>
#include <dfm-gui/appletmanager.h>
#include <dfm-gui/quickutils.h>
#include "applet_p.h"
#include "attachedproperty_p.h"
#include "windowmanager_p.h"

#include <qqml.h>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <qqmlpropertymap.h>

#include <QDebug>

DFMGUI_BEGIN_NAMESPACE

WindowManagerPrivate::WindowManagerPrivate(WindowManager *q)
    : q_ptr(q), globalEngine(new SharedQmlEngine)
{
}

/*!
 * \brief 异步加载 \a applet 对应的 QML 组件
 */
void WindowManagerPrivate::asyncLoadQuickItem(Applet *applet)
{
    Q_Q(WindowManager);
    SharedQmlEngine *tmpEngine = new SharedQmlEngine(q);
    QObject::connect(tmpEngine, &SharedQmlEngine::createFinished, [=](bool success) {
        if (success) {
            tmpEngine->completeCreation();
            QObject *rootObject = tmpEngine->rootObject();
            if (AppletItem *item = qobject_cast<AppletItem *>(rootObject)) {
                item->setApplet(applet);
                applet->dptr->setRootObject(item);

                if (auto containment = applet->containment()) {
                    rootObject->setParent(containment->rootObject());
                }
            } else {
                rootObject->deleteLater();
            }
        }

        tmpEngine->deleteLater();
    });

    tmpEngine->create(applet, true);

    if (auto containment = qobject_cast<Containment *>(applet)) {
        for (Applet *child : containment->applets()) {
            asyncLoadQuickItem(child);
        }
    }
}

/*!
 * \brief 应用退出时调用，手动释放全局的 QQmlEngine ，
 */
void WindowManagerPrivate::aboutToQuit()
{
    // Note: QQmlApplicationEnginePrivate::cleanUp() 实例化的根对象必须在引擎之前销毁
    for (const WindowHandlePtr &handle : std::as_const(windows)) {
        handle->window()->disconnect(globalEngine.get());

        delete handle->window();
        delete handle->panel();
    }
    windows.clear();

    // 手动释放
    globalEngine.reset();
}

/*!
 * \class WindowManager
 * \brief 窗口管理类，创建 QML 窗体
 * \details 通过已加载的组件信息创建对应的窗体
 *
 * \code
 *      QQuickWindow *window = WindowManager::instance()->createWindow("dfm-core", "MainWindow");
 *      window->show();
 * \endcode
 */
WindowManager::WindowManager()
    : dptr(new WindowManagerPrivate(this))
{
    // 注册，使用 @uri ... 标记，以在 QtCreator 中方便访问
    // @uri org.dfm.base
    const char *uri { "org.dfm.base" };
    qmlRegisterModule(uri, 1, 0);
    qmlRegisterUncreatableType<Applet>(uri, 1, 0, "Applet", "Applet attached");
    qmlRegisterExtendedType<Applet, AppletAttached>(uri, 1, 0, "Applet");
    qmlRegisterUncreatableType<Containment>(uri, 1, 0, "Containment", "Containment attached");
    qmlRegisterExtendedType<Containment, ContainmentAttached>(uri, 1, 0, "Containment");
    qmlRegisterType<AppletItem>(uri, 1, 0, "AppletItem");
    qmlRegisterType<ContainmentItem>(uri, 1, 0, "ContainmentItem");

    // 工具类，生命周期由 WindowManager 管理
    QuickUtils *globalUtils = new QuickUtils(this);
    qmlRegisterSingletonInstance<QuickUtils>(uri, 1, 0, "QuickUtils", globalUtils);

    // 退出时清理界面和 QQmlengine
    connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
        Q_D(WindowManager);
        d->aboutToQuit();
    });
}

WindowManager::~WindowManager() { }

WindowManager *WindowManager::instance()
{
    static WindowManager manager;
    return &manager;
}

/*!
 * \return 返回当前界面使用的全局 QQmlEngine
 */
QSharedPointer<QQmlEngine> WindowManager::engine() const
{
    Q_D(const WindowManager);
    if (d->globalEngine) {
        return d->globalEngine->globalEngine();
    }

    return {};
}

/*!
 * \brief 根据传入的插件 \a pluginName 和组件 \a quickId 信息查找并创建对应的主窗体，\a var 为初始化时传递给主窗体组件的参数
 * \return 创建的 QQuickWindow 和 Panel，无法创建返回空的 Handle
 */
WindowHandlePtr WindowManager::createWindow(const QString &pluginName, const QString &quickId, const QVariantMap &var)
{
    Q_D(WindowManager);
    // 首次进入初始化
    AppletManager::instance()->initRootTemplates();

    std::unique_ptr<Panel> panel { AppletManager::instance()->createPanel(pluginName, quickId) };
    if (!panel) {
        return WindowHandlePtr::create("Create panel failed.");
    }

    SharedQmlEngine engine;
    if (!engine.create(panel.get())) {
        return WindowHandlePtr::create("Load url failed.");
    }
    if (!engine.completeCreation(var)) {
        return WindowHandlePtr::create("Complele compoment creation failed.");
    }
    // rootObject 父对象已设为 panel
    QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObject());
    if (!window) {
        return WindowHandlePtr::create("Window's base component is not QQuickWindow");
    }
    panel->dptr->setRootObject(window);

    // 异步创建子节点
    for (auto child : panel->applets()) {
        d->asyncLoadQuickItem(child);
    }

    connect(window, &QQuickWindow::destroyed, this, [this](QObject *obj) {
        Q_D(WindowManager);
        if (auto view = qobject_cast<QQuickWindow *>(obj)) {
            WindowHandlePtr handle = d->windows.take(view);
            if (handle->panel()) {
                handle->panel()->deleteLater();
            }
        }
    });

    WindowHandlePtr handle = WindowHandlePtr::create(panel.release(), window);
    d->windows.insert(window, handle);
    return handle;
}

void WindowManager::showWindow(const WindowHandlePtr &handle)
{
    if (handle && handle->isValid()) {
        handle->window()->show();
        Q_EMIT handle->panel()->currentUrlChanged(handle->panel()->currentUrl());
    }
}

DFMGUI_END_NAMESPACE
