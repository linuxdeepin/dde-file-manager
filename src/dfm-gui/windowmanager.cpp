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
#include "sharedqmlengine_p.h"
#include "windowmanager_p.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/windowutils.h>

#include <qqml.h>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <qqmlpropertymap.h>

#include <QDebug>

DFMBASE_USE_NAMESPACE

DFMGUI_BEGIN_NAMESPACE

WindowManagerPrivate::WindowManagerPrivate(WindowManager *q)
    : q_ptr(q), globalEngine(new SharedQmlEngine)
{
}

WindowManager::Handle WindowManagerPrivate::createQuickWindow(const QString &pluginName, const QString &quickId, const QVariantMap &var)
{
    // 首次进入初始化
    AppletManager::instance()->initRootTemplates();

    std::unique_ptr<Panel> panel { AppletManager::instance()->createPanel(pluginName, quickId) };
    if (!panel) {
        lastError = QStringLiteral("Create panel failed.");
        return nullptr;
    }

    SharedQmlEngine engine;
    if (!engine.create(panel.get())) {
        lastError = QStringLiteral("Load url failed.");
        return nullptr;
    }
    if (!engine.completeCreation(var)) {
        lastError = QStringLiteral("Complele compoment creation failed.");
        return nullptr;
    }
    // rootObject 父对象已设为 panel
    QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObject());
    if (!window) {
        lastError = QStringLiteral("Window's base component is not QQuickWindow");
        return nullptr;
    }
    panel->dptr->setRootObject(window);

    // 异步创建子节点
    for (auto child : panel->applets()) {
        asyncLoadQuickItem(child);
    }

    WindowManager::Handle handle { panel.release() };
    windows.insert(window->winId(), handle);
    return handle;
}

void WindowManagerPrivate::connectWindowHandle(const WindowManager::Handle &handle)
{
    if (!handle) {
        return;
    }
    Q_Q(WindowManager);

    QObject::connect(handle->window(), &QQuickWindow::destroyed, q, [this](QObject *obj) {
        if (auto view = qobject_cast<QQuickWindow *>(obj)) {
            WindowManager::Handle deleteHandle = windows.take(view->winId());
            if (deleteHandle) {
                deleteHandle->deleteLater();
            } else {
                qCWarning(logDFMGui) << QStringLiteral("Unmanaged destroyed widnow:") << view << view->winId();
            }
        }
    });

    QObject::connect(handle, &Panel::aboutToClose, q, [this, q, handle]() {
        this->windowClosed(handle->windId());
        Q_EMIT q->windowClosed(handle->windId());
    });

    QObject::connect(handle, &Panel::aboutToOpen, q, [q, handle]() {
        Q_EMIT q->windowOpened(handle->windId());
    });

    QObject::connect(handle, &Panel::currentUrlChanged, q, [q, handle](const QUrl &url) {
        Q_EMIT q->currentUrlChanged(handle->windId(), url);
    });
}

bool WindowManagerPrivate::isValidUrl(const QUrl &url, QString &error) const
{
    if (url.isEmpty()) {
        error = QStringLiteral("Can't new window use empty url");
        return false;
    }

    if (!url.isValid()) {
        error = QStringLiteral("Can't new window use not valid ur");
        return false;
    }

    if (!UrlRoute::hasScheme(url.scheme())) {
        error = QString("No related scheme is registered "
                        "in the route form %0")
                        .arg(url.scheme());
        return false;
    }
    return true;
}

/*!
 * \return 返回 Url \a cur 和 \a other 是否等同
 */
bool WindowManagerPrivate::compareUrl(const QUrl &cur, const QUrl &other) const
{
    return UniversalUtils::urlEquals(other, cur)
            || UniversalUtils::urlEquals(other, FileUtils::bindUrlTransform(cur))
            || UniversalUtils::urlEquals(cur, FileUtils::bindUrlTransform(other));
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

void WindowManagerPrivate::moveWindowToCenter(QQuickWindow *window)
{
    QScreen *cursorScreen = WindowUtils::cursorScreen();
    if (!cursorScreen)
        return;
    int x = (cursorScreen->availableGeometry().width() - window->width()) / 2;
    int y = (cursorScreen->availableGeometry().height() - window->height()) / 2;
    window->setPosition(QPoint(x, y) + cursorScreen->geometry().topLeft());
}

void WindowManagerPrivate::windowClosed(quint64 windId)
{
    if (windows.isEmpty()) {
        return;
    }

    WindowManager::Handle handle = windows.value(windId);
    if (!handle) {
        return;
    }
    auto curWindow = handle->window();

    Q_Q(WindowManager);
    if (1 == windows.size()) {
        auto isDefaultWindow = curWindow->property("_dfm_isDefaultWindow");
        if (!isDefaultWindow.isValid() || !isDefaultWindow.toBool()) {
            handle->saveState();
        }

        qCInfo(logDFMGui) << QStringLiteral("Last window deletelator:") << curWindow << windId;
        Q_EMIT q->lastWindowClosed(windId);
    } else {
        qCInfo(logDFMGui) << QStringLiteral("Window deletelater:") << curWindow << windId;
    }

    // 在 Destoryed 信号处理 Panel 和 Windows 列表缓存中的释放
    curWindow->deleteLater();

    if (windId == previousActivedWindowId) {
        previousActivedWindowId = 0;
    }
}

/*!
 * \brief 应用退出时调用，手动释放全局的 QQmlEngine ，否则可能阻塞退出流程
 */
void WindowManagerPrivate::aboutToQuit()
{
    // Note: QQmlApplicationEnginePrivate::cleanUp() 实例化的根对象必须在引擎之前销毁
    for (const WindowManager::Handle &handle : std::as_const(windows)) {
        handle->window()->disconnect(globalEngine.get());

        delete handle->window();
        delete handle;
    }
    windows.clear();

    // 检测是否正常释放的探针
    auto detectPointer = SharedQmlEnginePrivate::engine().toWeakRef();
    // 手动释放
    globalEngine.reset();
    Q_ASSERT_X(detectPointer.isNull(), "Release global qml engine",
               "Cleaning the qml engine failed, maybe it's cached somewhere?");
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
    qmlRegisterUncreatableType<Panel>(uri, 1, 0, "Panel", "Panel attached");
    qmlRegisterExtendedType<Panel, PanelAttached>(uri, 1, 0, "Panel");
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
 * \brief 返回当前界面使用的全局 QQmlEngine , 可用于对 QQmlEngine 进行初始化设置。
 * \warning 不要持久化保留此函数返回的指针，当 qApp 抛出 aboutToQuit() 信号时，会释放此
 *  单例持有的全局 QQmlEngine 以正常退出应用。
 * \return 全局 QQmlEngine ，在 QCoreApplication::aboutToQuit() 信号触发后返回 nullptr
 */
QSharedPointer<QQmlEngine> WindowManager::engine() const
{
    Q_D(const WindowManager);
    if (d->globalEngine) {
        return SharedQmlEnginePrivate::engine();
    }

    return {};
}

/*!
 * \brief 根据传入的插件 \a pluginName 和组件 \a quickId 信息查找并创建对应的主窗体，\a var 为初始化时传递给主窗体组件的参数，
 *  创建的窗口会被当前管理类保留管理.
 * \return 创建的 QQuickWindow 和 Panel，无法创建返回空的 Handle
 */
WindowManager::Handle WindowManager::createWindow(const QUrl &url, const QString &pluginName,
                                                  const QString &quickId, const QVariantMap &var)
{
    Q_ASSERT_X(thread() == qApp->thread(), "FileManagerWindowsManager", "Show window must in main thread!");
    Q_D(WindowManager);

    // 校验Url
    QUrl showedUrl = Application::instance()->appUrlAttribute(Application::kUrlOfNewWindow);
    if (!url.isEmpty()) {
        const FileInfoPointer &info = InfoFactory::create<FileInfo>(url);
        if (info && info->isAttributes(OptInfoType::kIsFile)) {
            showedUrl = url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash);
        } else {
            showedUrl = url;
        }
    }

    if (!d->isValidUrl(showedUrl, d->lastError)) {
        qCWarning(logDFMGui) << "Url: " << showedUrl << "is Invalid, error: " << d->lastError;
        // use home as showed url if default url is invalid
        showedUrl = UrlRoute::pathToReal(QDir::home().path());
        if (!d->isValidUrl(showedUrl, d->lastError))
            return nullptr;
    }

    Handle handle = d->createQuickWindow(pluginName, quickId, var);
    if (handle) {
        handle->loadState();
        handle->setCurrentUrl(url);
        d->connectWindowHandle(handle);

        if (1 == d->windows.size()) {
            if (!(handle->window()->windowStates() & Qt::WindowMaximized)) {
                d->moveWindowToCenter(handle->window());
            }
        }

        Q_EMIT windowCreated(handle->window()->winId());
    }

    return handle;
}

void WindowManager::showWindow(const Handle &handle) const
{
    if (handle) {
        handle->window()->show();
        handle->window()->requestActivate();

        Q_EMIT handle->currentUrlChanged(handle->currentUrl());
    }
}

/*!
 * \brief 查找当前窗口管理中第一个匹配当前 Url 为 \a url 的窗口并激活
 * \return 是否成功查找并激活窗口
 */
bool WindowManager::activeExistsWindowByUrl(const QUrl &url) const
{
    Handle handle = findWindowByUrl(url);
    if (handle) {
        auto windStates = handle->window()->windowStates();
        handle->window()->setWindowStates(windStates & ~Qt::WindowMinimized);
        // 等同 QWidget::activateWindow
        handle->window()->requestActivate();
        return true;
    }

    return false;
}

/*!
 * \return 查找 Quick 对象指针 \a itemObject 对应的窗口 ID
 */
quint64 WindowManager::findWindowId(const QObject *itemObject) const
{
    if (!itemObject) {
        return 0;
    }

    if (auto item = qobject_cast<const QQuickItem *>(itemObject)) {
        return item->window() ? item->window()->winId() : 0;
    }

    QObject *parentObject = itemObject->parent();
    while (parentObject) {
        if (auto window = qobject_cast<const QWindow *>(parentObject)) {
            return window->winId();
        }

        parentObject = parentObject->parent();
    }

    return 0;
}

/*!
 * \return 查找 Applet 关联 Quick 组件的窗口 ID
 */
quint64 WindowManager::findWindowIdFromApplet(Applet *applet) const
{
    if (!applet || !applet->rootObject()) {
        return 0;
    }

    return findWindowId(applet->rootObject());
}

/*!
 * \return 返回当前窗口管理中打开的 Url 为 \a url 的首个窗口
 */
WindowManager::Handle WindowManager::findWindowByUrl(const QUrl &url) const
{
    Q_D(const WindowManager);
    for (auto handleItr : d->windows) {
        if (handleItr
            && d->compareUrl(handleItr->currentUrl(), url)) {
            return handleItr;
        }
    }

    return nullptr;
}

/*!
 * \return 返回当前窗口管理中窗口 ID 为 \a winId 的窗口信息
 */
WindowManager::Handle WindowManager::findWindowById(quint64 windId) const
{
    return d_func()->windows.value(windId, nullptr);
}

/*!
 * \return 返回当前窗口管理中已创建管理的窗口ID列表
 */
QList<quint64> WindowManager::windowIdList() const
{
    return d_func()->windows.keys();
}

void WindowManager::resetPreviousActivedWindowId()
{
    Q_D(WindowManager);
    d->previousActivedWindowId = 0;

    // Before creating a new window, save the current actived window id to `previousActivedWindowId`,
    // since many times we need to synchronize some informations from the trigger window to the new window
    // such as the sidebar expanding states, so `previousActivedWindowId` is help for that.
    for (const Handle &handle : d->windows) {
        if (handle->window()->isActive()) {
            d->previousActivedWindowId = handle->windId();
            break;
        }
    }
}

quint64 WindowManager::previousActivedWindowId() const
{
    return d_func()->previousActivedWindowId;
}

bool WindowManager::containsCurrentUrl(const QUrl &url, const QQuickWindow *win) const
{
    Q_D(const WindowManager);
    for (const Handle &handle : d->windows) {
        if (handle->window() == win || !handle->window()) {
            continue;
        }

        auto cur = handle->currentUrl();
        if (d->compareUrl(cur, url)) {
            return true;
        }
    }

    return false;
}

QString WindowManager::lastError() const
{
    return d_func()->lastError;
}

DFMGUI_END_NAMESPACE
