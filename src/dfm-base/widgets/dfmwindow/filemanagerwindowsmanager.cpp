// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <dfm-base/utils/fileutils.h>

#include <QDebug>
#include <QEvent>
#include <QApplication>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include <QX11Info>
#endif
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
        if (window == nullptr)
            continue;

        auto cur = window->currentUrl();
        if (UniversalUtils::urlEquals(url, cur)
            || UniversalUtils::urlEquals(url, FileUtils::bindUrlTransform(cur))
            || UniversalUtils::urlEquals(cur, FileUtils::bindUrlTransform(url))) {
            qCInfo(logDFMBase) << "Find url: " << url << " window: " << window;
            if (window->isMinimized())
                window->setWindowState(window->windowState() & ~Qt::WindowMinimized);
            window->activateWindow();
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

void FileManagerWindowsManagerPrivate::onWindowClosed(FileManagerWindow *window)
{
    int count = windows.count();
    if (count <= 0) {
        qCDebug(logDFMBase) << "FileManagerWindowsManager::onWindowClosed: No windows to close";
        return;
    }

    if (!window) {
        qCWarning(logDFMBase) << "FileManagerWindowsManager::onWindowClosed: Null window pointer provided";
        return;
    }

    qCDebug(logDFMBase) << "FileManagerWindowsManager::onWindowClosed: Closing window with ID:"
                        << window->internalWinId() << "Remaining windows:" << (count - 1);

    if (count == 1) {   // last window
        auto isDefaultWindow = window->property("_dfm_isDefaultWindow");
        if (window->saveClosedSate() && (!isDefaultWindow.isValid() || !isDefaultWindow.toBool())) {
            qCDebug(logDFMBase) << "FileManagerWindowsManager::onWindowClosed: Saving window state for last window";
            window->saveState();
        }
        qCInfo(logDFMBase) << "FileManagerWindowsManager::onWindowClosed: Last window closing, ID:"
                           << window->internalWinId();
        emit manager->lastWindowClosed(window->internalWinId());
        window->deleteLater();
    } else {
        qCDebug(logDFMBase) << "FileManagerWindowsManager::onWindowClosed: Regular window closing, ID:"
                            << window->internalWinId();
        window->deleteLater();
    }

    lastActivedWindowIdList.removeOne(window->internalWinId());
    int removed = windows.remove(window->internalWinId());
    if (removed > 0 && previousActivedWindowId == window->internalWinId()) {
        qCDebug(logDFMBase) << "FileManagerWindowsManager::onWindowClosed: Clearing previous active window ID";
        previousActivedWindowId = 0;
    }
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

    qCDebug(logDFMBase) << "FileManagerWindowsManager::createWindow: Creating window for URL:" << url
                        << "New window:" << isNewWindow;

    QUrl showedUrl = Application::instance()->appUrlAttribute(Application::kUrlOfNewWindow);
    auto info = InfoFactory::create<FileInfo>(showedUrl);
    if (!info || !info->exists()) {
        showedUrl.setScheme(Global::Scheme::kComputer);
        showedUrl.setPath("/");
        Application::instance()->setAppAttribute(Application::kUrlOfNewWindow, showedUrl);
    }

    if (!url.isEmpty()) {
        const FileInfoPointer &info = InfoFactory::create<FileInfo>(url);
        if (info && info->isAttributes(OptInfoType::kIsFile)) {
            showedUrl = UrlRoute::urlParent(url);
            qCDebug(logDFMBase) << "FileManagerWindowsManager::createWindow: URL is file, using parent directory:" << showedUrl;
        } else {
            showedUrl = url;
        }
    }
    if (!d->isValidUrl(showedUrl, &error)) {
        qCWarning(logDFMBase) << "FileManagerWindowsManager::createWindow: Invalid URL:" << showedUrl
                              << "Error:" << error;
        // use home as showed url if default url is invalid
        showedUrl = UrlRoute::pathToReal(QDir::home().path());
        qCDebug(logDFMBase) << "FileManagerWindowsManager::createWindow: Using home directory as fallback:" << showedUrl;
        if (!d->isValidUrl(showedUrl, &error)) {
            qCCritical(logDFMBase) << "FileManagerWindowsManager::createWindow: Even home directory is invalid:"
                                   << showedUrl << "Error:" << error;
            return nullptr;
        }
    }

    // Directly active window if the window exists
    if (!isNewWindow) {
        qCDebug(logDFMBase) << "FileManagerWindowsManager::createWindow: Looking for existing window for:" << showedUrl;
        auto window = d->activeExistsWindowByUrl(showedUrl);
        if (!window) {
            qCDebug(logDFMBase) << "FileManagerWindowsManager::createWindow: No existing window found for:" << showedUrl;
        } else {
            qCInfo(logDFMBase) << "FileManagerWindowsManager::createWindow: Activated existing window for:" << showedUrl;
        }
        return window;
    }
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QX11Info::setAppTime(QX11Info::appUserTime());
#endif
    qCDebug(logDFMBase) << "FileManagerWindowsManager::createWindow: Creating new window instance";

    // you can inherit from FMWindow to implement a custom window (by call `setCustomWindowCreator`)
    FMWindow *window = d->customCreator ? d->customCreator(showedUrl)
                                        : new FMWindow(showedUrl);

    window->winId();

    {
        auto noLoad = window->property("_dfm_Disable_RestoreWindowState_");
        if (!noLoad.isValid() || !noLoad.toBool()) {
            qCDebug(logDFMBase) << "FileManagerWindowsManager::createWindow: Loading window state";
            window->loadState();
        } else {
            qCDebug(logDFMBase) << "FileManagerWindowsManager::createWindow: Skipping window state loading:" << noLoad;
        }
    }

    connect(window, &FileManagerWindow::aboutToClose, this, [this, window]() {
        qCDebug(logDFMBase) << "FileManagerWindowsManager: Window about to close:" << window->internalWinId();
        emit windowClosed(window->internalWinId());
        d->onWindowClosed(window);
    });

    connect(window, &FileManagerWindow::aboutToOpen, this, [this, window]() {
        auto &&id { window->internalWinId() };
        qCInfo(logDFMBase) << "FileManagerWindowsManager: Window opened with ID:" << id;
        emit windowOpened(id);
    });

    connect(window, &FileManagerWindow::reqShowHotkeyHelp, this, [this, window]() {
        qCDebug(logDFMBase) << "FileManagerWindowsManager: Hotkey help requested for window:" << window->internalWinId();
        d->onShowHotkeyHelp(window);
    });

    connect(window, &FileManagerWindow::currentUrlChanged, this, [this, window](const QUrl &url) {
        qCDebug(logDFMBase) << "FileManagerWindowsManager: Current URL changed for window:"
                            << window->winId() << "New URL:" << url;
        emit currentUrlChanged(window->winId(), url);
    });

    connect(window, &FileManagerWindow::windowActived, this, [this, window] {
        auto &&id = window->internalWinId();
        if (d->lastActivedWindowIdList.contains(id))
            d->lastActivedWindowIdList.removeOne(id);
        d->lastActivedWindowIdList.append(id);
    });

    // In order for the plugin to cache the current window (before the base frame is installed)
    qCInfo(logDFMBase) << "FileManagerWindowsManager::createWindow: New window created with ID:"
                       << window->winId() << "URL:" << showedUrl;

    d->windows.insert(window->internalWinId(), window);

    if (d->windows.size() == 1) {
        if (!(window->windowState() & Qt::WindowMaximized)) {
            qCDebug(logDFMBase) << "FileManagerWindowsManager::createWindow: Centering first window";
            window->moveCenter();
        }
    }
    emit windowCreated(window->internalWinId());

    finally.dismiss();
    return window;
}

void FileManagerWindowsManager::showWindow(FileManagerWindowsManager::FMWindow *window)
{
    Q_ASSERT(window);
    window->show();
    qApp->setActiveWindow(window);
    emit window->currentUrlChanged(window->currentUrl());   // The URL needs to notify the subscribers when the first window opened.
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

    qCWarning(logDFMBase) << "The `d->windows` cannot find winId: " << winId;
    for (QWidget *top : qApp->topLevelWidgets()) {
        if (top->internalWinId() == winId)
            return qobject_cast<FileManagerWindowsManager::FMWindow *>(top);
    }
    qCWarning(logDFMBase) << "Null window returned!";

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
    // Before creating a new window, save the current actived window id to `previousActivedWindowId`,
    // since many times we need to synchronize some informations from the trigger window to the new window
    // such as the sidebar expanding states, so `previousActivedWindowId` is help for that.
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

quint64 FileManagerWindowsManager::lastActivedWindowId()
{
    return d->lastActivedWindowIdList.isEmpty() ? 0 : d->lastActivedWindowIdList.last();
}

bool FileManagerWindowsManager::containsCurrentUrl(const QUrl &url, const QWidget *win)
{
    // TODO: Discuss with ligigang, maybe there is a better way.
    if (qAppName() != "dde-file-manager")
        return false;

    auto windows = d->windows.values();
    for (auto w : windows) {
        if (win == w || !w)
            continue;
        auto cur = w->currentUrl();
        if (UniversalUtils::urlEquals(url, cur) || UniversalUtils::urlEquals(url, FileUtils::bindUrlTransform(cur)) || UniversalUtils::urlEquals(cur, FileUtils::bindUrlTransform(url)))
            return true;
    }
    return false;
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
