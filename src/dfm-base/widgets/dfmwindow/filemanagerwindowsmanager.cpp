// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/filemanagerwindowsmanager_p.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/finallyutil.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/shortcut/shortcut.h>

#include <QDebug>
#include <QEvent>
#include <QApplication>
#include <QX11Info>
#include <QScreen>
#include <QWindow>
#include <QTimer>
#include <QProcess>
#include <QDir>

#include <mutex>

DFMBASE_USE_NAMESPACE

enum NetWmState {
    kNetWmStateAbove = 0x1,
    kNetWmStateBelow = 0x2,
    kNetWmStateFullScreen = 0x4,
    kNetWmStateMaximizedHorz = 0x8,
    kNetWmStateMaximizedVert = 0x10,
    kNetWmStateModal = 0x20,
    kNetWmStateStaysOnTop = 0x40,
    kNetWmStateDemandsAttention = 0x80
};
Q_DECLARE_FLAGS(NetWmStates, NetWmState)

FileManagerWindowsManagerPrivate::FileManagerWindowsManagerPrivate(FileManagerWindowsManager *serv)
    : QObject(nullptr), manager(serv)
{
}

FileManagerWindow *FileManagerWindowsManagerPrivate::activeExistsWindowByUrl(const QUrl &url)
{
    int count = windows.count();

    for (int i = 0; i != count; ++i) {
        quint64 key = windows.keys().at(i);
        auto window = windows.value(key);
        if (window && UniversalUtils::urlEquals(window->currentUrl(), url)) {
            qInfo() << "Find url: " << url << " window: " << window;
            if (window->isMinimized())
                window->setWindowState(window->windowState() & ~Qt::WindowMinimized);
            qApp->setActiveWindow(window);
            return window;
        }
    }

    return nullptr;
}

bool FileManagerWindowsManagerPrivate::isValidUrl(const QUrl &url, QString *error)
{
    Q_ASSERT(error);

    if (url.isEmpty()) {
        *error = "Can't new window use empty url";
        return false;
    }

    if (!url.isValid()) {
        *error = "Can't new window use not valid ur";
        return false;
    }

    if (!UrlRoute::hasScheme(url.scheme())) {
        *error = QString("No related scheme is registered "
                         "in the route form %0")
                         .arg(url.scheme());
        return false;
    }
    return true;
}

void FileManagerWindowsManagerPrivate::loadWindowState(FileManagerWindow *window)
{
    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "WindowState").toMap();

    int width = state.value("width").toInt();
    int height = state.value("height").toInt();
    NetWmStates windowState = static_cast<NetWmStates>(state.value("state").toInt());

    // fix bug 30932,获取全屏属性，必须是width全屏和height全屏熟悉都满足，才判断是全屏
    if ((windows.size() == 0) && ((windowState & kNetWmStateMaximizedHorz) != 0 && (windowState & kNetWmStateMaximizedVert) != 0)) {
        // make window to be maximized.
        // the following calling is copyed from QWidget::showMaximized()
        window->setWindowState((window->windowState() & ~(Qt::WindowMinimized | Qt::WindowFullScreen))
                               | Qt::WindowMaximized);
    } else {
        window->resize(width, height);
    }
}

void FileManagerWindowsManagerPrivate::saveWindowState(FileManagerWindow *window)
{
    /// The power by dxcb platform plugin
    NetWmStates states = static_cast<NetWmStates>(window->window()->windowHandle()->property("_d_netWmStates").toInt());
    QVariantMap state;
    // fix bug 30932,获取全屏属性，必须是width全屏和height全屏熟悉都满足，才判断是全屏
    if ((states & kNetWmStateMaximizedHorz) == 0 || (states & kNetWmStateMaximizedVert) == 0) {
        state["width"] = window->size().width();
        state["height"] = window->size().height();
    } else {
        const QVariantMap &state1 = Application::appObtuselySetting()->value("WindowManager", "WindowState").toMap();
        state["width"] = state1.value("width").toInt();
        state["height"] = state1.value("height").toInt();
        state["state"] = static_cast<int>(states);
    }
    Application::appObtuselySetting()->setValue("WindowManager", "WindowState", state);
}

void FileManagerWindowsManagerPrivate::onWindowClosed(FileManagerWindow *window)
{
    int count = windows.count();
    if (count <= 0)
        return;

    if (!window)
        return;

    if (count == 1) {   // last window
        auto isDefaultWindow = window->property("_dfm_isDefaultWindow");
        if (window->saveClosedSate() && (!isDefaultWindow.isValid() || !isDefaultWindow.toBool()))
            saveWindowState(window);
        qInfo() << "Last window deletelater" << window->internalWinId();
        emit manager->lastWindowClosed();
        window->deleteLater();
    } else {
        qInfo() << "Window deletelater !";
        window->deleteLater();
    }

    int re = windows.remove(window->internalWinId());
    if (re > 0 && previousActivedWindowId == window->internalWinId())
        previousActivedWindowId = 0;
}

void FileManagerWindowsManagerPrivate::onShowHotkeyHelp(FileManagerWindow *window)
{
    QRect rect = window->geometry();
    QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
    Shortcut sc;
    QStringList args;
    QString param1 = "-j=" + sc.toStr();
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    args << param1 << param2;
    QProcess::startDetached("deepin-shortcut-viewer", args);
}

/*
 * FileManagerWindowsManager
 */

FileManagerWindowsManager &FileManagerWindowsManager::instance()
{
    static FileManagerWindowsManager ins;
    return ins;
}

void FileManagerWindowsManager::setCustomWindowCreator(FileManagerWindowsManager::WindowCreator creator)
{
    d->customCreator = creator;
}

FileManagerWindowsManager::FMWindow *FileManagerWindowsManager::createWindow(const QUrl &url, bool isNewWindow, QString *errorString)
{
    Q_ASSERT_X(thread() == qApp->thread(), "FileManagerWindowsManager", "Show window must in main thread!");
    QString error;
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { if (errorString) *errorString = error; });

    QUrl showedUrl = Application::instance()->appUrlAttribute(Application::kUrlOfNewWindow);
    if (!url.isEmpty()) {
        const FileInfoPointer &info = InfoFactory::create<FileInfo>(url);
        if (info && info->isAttributes(OptInfoType::kIsFile)) {
            showedUrl = UrlRoute::urlParent(url);
        } else {
            showedUrl = url;
        }
    }
    if (!d->isValidUrl(showedUrl, &error)) {
        qWarning() << "Url: " << showedUrl << "is Invalid, error: " << error;
        // use home as showed url if default url is invalid
        showedUrl = UrlRoute::pathToReal(QDir::home().path());
        if (!d->isValidUrl(showedUrl, &error))
            return nullptr;
    }

    // Directly active window if the window exists
    if (!isNewWindow) {
        auto window = d->activeExistsWindowByUrl(showedUrl);
        if (!window)
            qWarning() << "Cannot find a exists window by url: " << showedUrl;
        return window;
    }

    QX11Info::setAppTime(QX11Info::appUserTime());

    // you can inherit from FMWindow to implement a custom window (by call `setCustomWindowCreator`)
    FMWindow *window = d->customCreator ? d->customCreator(showedUrl)
                                        : new FMWindow(showedUrl);
    window->winId();

    {
        auto noLoad = window->property("_dfm_Disable_RestoreWindowState_");
        if (!noLoad.isValid() || !noLoad.toBool())
            d->loadWindowState(window);
        else
            qDebug() << "do not load window state" << window << noLoad;
    }

    connect(window, &FileManagerWindow::aboutToClose, this, [this, window]() {
        emit windowClosed(window->internalWinId());
        d->onWindowClosed(window);
    });

    connect(window, &FileManagerWindow::aboutToOpen, this, [this, window]() {
        auto &&id { window->internalWinId() };
        qInfo() << "Window showed" << id;
        emit windowOpened(id);
    });

    connect(window, &FileManagerWindow::reqShowHotkeyHelp, this, [this, window]() {
        d->onShowHotkeyHelp(window);
    });

    // In order for the plugin to cache the current window (before the base frame is installed)
    qInfo() << "New window created: " << window->winId() << showedUrl;

    d->windows.insert(window->internalWinId(), window);

    if (d->windows.size() == 1)
        window->moveCenter();
    emit windowCreated(window->internalWinId());
    finally.dismiss();
    return window;
}

void FileManagerWindowsManager::showWindow(FileManagerWindowsManager::FMWindow *window)
{
    Q_ASSERT(window);
    window->show();
    qApp->setActiveWindow(window);
}

/*!
 * \brief FileManagerWindowsManager::findWindowId find window id by input a window object pointer
 * \param window
 * \return window id
 */
quint64 FileManagerWindowsManager::findWindowId(const QWidget *window)
{
    if (!window)
        return 0;

    int count = d->windows.size();
    for (int i = 0; i != count; ++i) {
        quint64 key = d->windows.keys().at(i);
        if (d->windows.value(key) == window->topLevelWidget())
            return key;
    }

    // if `window` not contains `d->windows`, find the windows's base class
    const QWidget *newWindow = window;
    while (newWindow) {
        if (newWindow->inherits("FileManagerWindow"))
            return newWindow->winId();

        newWindow = newWindow->parentWidget();
    }

    return window->window()->internalWinId();
}

/*!
 * \brief FileManagerWindowsManager::findWindowById find a window by a window id
 * \param winId
 * \return window object pointer otherwise return nullptr
 */
FileManagerWindowsManager::FMWindow *FileManagerWindowsManager::findWindowById(quint64 winId)
{
    if (winId <= 0)
        return nullptr;

    if (d->windows.contains(winId))
        return d->windows.value(winId);

    qWarning() << "The `d->windows` cannot find winId: " << winId;
    for (QWidget *top : qApp->topLevelWidgets()) {
        if (top->internalWinId() == winId)
            return qobject_cast<FileManagerWindowsManager::FMWindow *>(top);
    }
    qWarning() << "Null window returned!";

    return nullptr;
}

/*!
 * \brief FileManagerWindowsManager::windowIdList
 * \return all windows id
 */
QList<quint64> FileManagerWindowsManager::windowIdList()
{
    return d->windows.keys();
}

void FileManagerWindowsManager::resetPreviousActivedWindowId()
{
    d->previousActivedWindowId = 0;
    QHashIterator<quint64, DFMBASE_NAMESPACE::FileManagerWindow *> it(d->windows);
    //Before creating a new window, save the current actived window id to `previousActivedWindowId`,
    //since many times we need to synchronize some informations from the trigger window to the new window
    //such as the sidebar expanding states, so `previousActivedWindowId` is help for that.
    while (it.hasNext()) {
        it.next();
        if (it.value()->isActiveWindow()) {
            d->previousActivedWindowId = it.key();
            break;
        }
    }
}

quint64 FileManagerWindowsManager::previousActivedWindowId()
{
    return d->previousActivedWindowId;
}

FileManagerWindowsManager::FileManagerWindowsManager(QObject *parent)
    : QObject(parent), d(new FileManagerWindowsManagerPrivate(this))

{
}

FileManagerWindowsManager::~FileManagerWindowsManager()
{
    for (auto val : d->windows.values()) {
        // 框架退出非程序退出，依然会存在QWidget相关操作，
        // 如果强制使用delete，那么将导致Qt机制的与懒汉单例冲突崩溃
        if (val)
            val->deleteLater();
    }
    d->windows.clear();
}
