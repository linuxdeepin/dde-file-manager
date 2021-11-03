/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "windowmanager.h"
#include "dfilemanagerwindow.h"
#include "dabstractfilewatcher.h"
#include "dabstractfileinfo.h"
#include "dfileservices.h"
#include "dfmapplication.h"
#include "dfmsettings.h"
#include "drootfilemanager.h"
#include "app/define.h"
#include "app/filesignalmanager.h"

#include "fileoperations/filejob.h"

#include "dialogs/dialogmanager.h"

#include "qobjecthelper.h"

#include "singleton.h"
#include "vault/vaulthelper.h"
#include "shutil/fileutils.h"
#include <DApplication>

#include <QThread>
#include <QDebug>
#include <QApplication>
#include <QX11Info>
#include <QScreen>
#include <QWindow>
#include <QTimer>
#include <QProcess>

DTK_USE_NAMESPACE

DFM_USE_NAMESPACE

enum NetWmState {
    NetWmStateAbove = 0x1,
    NetWmStateBelow = 0x2,
    NetWmStateFullScreen = 0x4,
    NetWmStateMaximizedHorz = 0x8,
    NetWmStateMaximizedVert = 0x10,
    NetWmStateModal = 0x20,
    NetWmStateStaysOnTop = 0x40,
    NetWmStateDemandsAttention = 0x80
};

Q_DECLARE_FLAGS(NetWmStates, NetWmState)

QHash<const QWidget *, quint64> WindowManager::m_windows;
int WindowManager::m_count = 0;

WindowManager::WindowManager(QObject *parent) : QObject(parent)
{
#ifdef AUTO_RESTART_DEAMON
    m_restartProcessTimer = new QTimer(this);
    m_restartProcessTimer->setInterval(1000 * 60 * 5);
#endif
    initConnect();
}

class WindowManager_ : public WindowManager {};
Q_GLOBAL_STATIC(WindowManager_, wmGlobal)

WindowManager *WindowManager::instance()
{
    return wmGlobal;
}

WindowManager::~WindowManager()
{

}

void WindowManager::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::requestOpenNewWindowByUrl, this, &WindowManager::showNewWindow);
    connect(fileSignalManager, &FileSignalManager::aboutToCloseLastActivedWindow, this, &WindowManager::onLastActivedWindowClosed);
    connect(qApp, &QApplication::aboutToQuit, this, [ = ]() {
        fileSignalManager->requestCloseListen();
        DFMGlobal::setAppQuiting();
        qInfo() << "app quiting !";
    });
    connect(fileSignalManager, &FileSignalManager::requestShowNewWindows, this, &WindowManager::onShowNewWindow,
            Qt::QueuedConnection);
    connect(DRootFileManager::instance(), &DRootFileManager::queryRootFileFinsh, this, &WindowManager::onShowNewWindow,
            Qt::QueuedConnection);
    connect(fileSignalManager, &FileSignalManager::requestRemoveSmbUrl, this, &WindowManager::onRemoveNeedShowSmbUrl);

#ifdef AUTO_RESTART_DEAMON
    connect(m_restartProcessTimer, &QTimer::timeout, this, &WindowManager::reastartAppProcess);
#endif
}

void WindowManager::loadWindowState(DFileManagerWindow *window)
{
    const QVariantMap &state = DFMApplication::appObtuselySetting()->value("WindowManager", "WindowState").toMap();

    int width = state.value("width").toInt();
    int height = state.value("height").toInt();
    NetWmStates windowState = static_cast<NetWmStates>(state.value("state").toInt());
    // fix bug 30932,获取全屏属性，必须是width全屏和height全屏熟悉都满足，才判断是全屏
    if ((m_windows.size() == 0) && ((windowState & NetWmStateMaximizedHorz) != 0 && (windowState & NetWmStateMaximizedVert) != 0)) {
        window->showMaximized();
    } else {
        window->resize(width, height);
    }
}


void WindowManager::saveWindowState(DFileManagerWindow *window)
{
    /// The power by dxcb platform plugin
    NetWmStates states = static_cast<NetWmStates>(window->window()->windowHandle()->property("_d_netWmStates").toInt());
    QVariantMap state;
    // fix bug 30932,获取全屏属性，必须是width全屏和height全屏熟悉都满足，才判断是全屏
    if ((states & NetWmStateMaximizedHorz) == 0 || (states & NetWmStateMaximizedVert) == 0) {
        state["width"] = window->size().width();
        state["height"] = window->size().height();
    } else {
        const QVariantMap &state1 = DFMApplication::appObtuselySetting()->value("WindowManager", "WindowState").toMap();
        state["width"] = state1.value("width").toInt();
        state["height"] = state1.value("height").toInt();
        state["state"] = static_cast<int>(states);
    }
    DFMApplication::appObtuselySetting()->setValue("WindowManager", "WindowState", state);
}

DUrl WindowManager::getUrlByWindowId(quint64 windowId)
{
    if (getWindowById(windowId)) {
        DFileManagerWindow *window = qobject_cast<DFileManagerWindow *>(getWindowById(windowId));
        return window->currentUrl();
    }
    return DUrl::fromLocalFile(QDir::homePath());
}

bool WindowManager::tabAddableByWinId(const quint64 &winId)
{
    DFileManagerWindow *window = qobject_cast<DFileManagerWindow *>(getWindowById(winId));
    if (window)
        return window->tabAddable();
    return false;

}

bool WindowManager::enableAutoQuit() const
{
#ifdef AUTO_RESTART_DEAMON
    return m_enableAutoQuit;
#else
    return false;
#endif
}

void WindowManager::clearWindowActions()
{
    for (auto win : m_windows.keys()) {
        QWidget *window = const_cast<QWidget *>(win);
        DFileManagerWindow *fmWin = qobject_cast<DFileManagerWindow *>(window);
        if (fmWin)
            fmWin->clearActions();
    }
}

void WindowManager::showNewWindow(const DUrl &url, const bool &isNewWindow)
{
    if (!DFMGlobal::isInitAppOver() || DFMGlobal::isAppQuiting()) {
        qInfo() << "window app not init over or app quit! " << isNewWindow << url;
        return;
    }
    bool tempNewWindow = isNewWindow;
    if (url.scheme() == SMB_SCHEME && !url.host().isEmpty() && !url.path().isEmpty())
        tempNewWindow = true;
    if (!tempNewWindow) {
        for (int i = 0; i < m_windows.count(); i++) {
            QWidget *window = const_cast<QWidget *>(m_windows.keys().at(i));
            DUrl currentUrl = static_cast<DFileManagerWindow *>(window)->currentUrl();
            if (currentUrl == url) {
                DFileManagerWindow *wd = static_cast<DFileManagerWindow *>(window);
                qDebug() << currentUrl << wd;
                if (wd->isMinimized()) {
                    wd->setWindowState(wd->windowState() & ~Qt::WindowMinimized);
                }
                qApp->setActiveWindow(wd);
                return;
            }
        }
    }
    // 处理同一个smb在挂载时，第二个窗口不要启动，等待这个一个smb挂载结束了在启动窗口
    if (url.scheme() == SMB_SCHEME && !url.host().isEmpty() && !url.path().isEmpty()) {
        DUrl smbUrl(url);
        QString smbShareName = url.path().mid(1);
        smbShareName = smbShareName.mid(0, smbShareName.indexOf("/"));
        smbUrl.setPath("/" + smbShareName);
        if (!DRootFileManager::instance()->isRootFileInited() ||
                !DRootFileManager::instance()->isRootFileContainSmb(url)) {
            QMutexLocker lk(&m_smbPointUrlMutex);
            if (m_smbPointUrl.keys().contains(smbUrl)) {
                m_smbPointUrl.insert(smbUrl, url);
                return;
            } else {
                m_smbPointUrl.insert(smbUrl, DUrl());
            }
        }
    }
    QX11Info::setAppTime(QX11Info::appUserTime());
    DFileManagerWindow *window = new DFileManagerWindow(url.isEmpty() ? DFMApplication::instance()->appUrlAttribute(DFMApplication::AA_UrlOfNewWindow) : url);
    loadWindowState(window);
    // fix bug 59239 drag事件的接受者的drop事件和发起drag事件的发起者的mousemove事件处理完成才能
    // 析构本窗口，所以去掉属性Qt::WA_DeleteOnClose

    qInfo() << "new window" << window->winId() << url;

    connect(window, &DFileManagerWindow::aboutToClose,
            this, &WindowManager::onWindowClosed);

    m_windows.insert(window, window->winId());
    window->requestToSelectUrls(); //###: here, when selected files and then drag a tab to create a new window.
    //     will select these files again in new window.

    if (m_windows.count() == 1) {
        QPoint pos = QCursor::pos();
        QRect currentScreenGeometry;

        for (QScreen *screen : qApp->screens()) {
            if (screen->geometry().contains(pos)) {
                currentScreenGeometry = screen->geometry();
            }
        }

        if (currentScreenGeometry.isEmpty()) {
            currentScreenGeometry = qApp->primaryScreen()->geometry();
        }

        window->moveCenter(currentScreenGeometry.center());
    }

    //fix bug71721 先移动到对应位置再show防止出现卡其他位置画面的问题
    window->show();

    // 修复bug-45226 文管弹出一个模态窗后，再次弹出文管框，
    // 该模态框将失去焦点，无法正常显示
    if (!VaultHelper::isModel)
        qApp->setActiveWindow(window);
}


quint64 WindowManager::getWindowId(const QWidget *window)
{
    int winId = static_cast<int>(m_windows.value(window->topLevelWidget(), 0));

    if (winId != 0)
        return static_cast<quint64>(winId);

    const QWidget *newW = window;

    while (newW) {
        if (newW->inherits("DFileManagerWindow")) {
            return newW->winId();
        }

        newW = newW->parentWidget();
    }

    return window->window()->internalWinId();
}

QWidget *WindowManager::getWindowById(quint64 winId)
{
    if (winId <= 0)
        return Q_NULLPTR;

    const QWidget *widget = m_windows.key(winId);

    if (widget)
        return const_cast<QWidget *>(widget);

    for (QWidget *top_level_widget : qApp->topLevelWidgets()) {
        if (top_level_widget->internalWinId() == winId)
            return top_level_widget;
    }

    return Q_NULLPTR;
}

void WindowManager::reastartAppProcess()
{
    if (m_windows.count() == 0) {
        if (dialogManager->isTaskDialogEmpty()) {
            // 当没有顶级窗口时才允许应用自动退出
            if (qApp->topLevelWindows().isEmpty()) {
                qApp->quit();
            }
        }
    }
}

void WindowManager::setEnableAutoQuit(bool enableAutoQuit)
{
#ifdef AUTO_RESTART_DEAMON
    if (m_enableAutoQuit == enableAutoQuit)
        return;

    m_enableAutoQuit = enableAutoQuit;

    if (m_enableAutoQuit) {
        m_restartProcessTimer->start();
    } else {
        m_restartProcessTimer->stop();
    }
#else
    Q_UNUSED(enableAutoQuit)
#endif
}

void WindowManager::onWindowClosed()
{
    if (m_windows.count() <= 0)
        return;
    DFileManagerWindow *window = qobject_cast<DFileManagerWindow *>(sender());
    if (m_windows.count() == 1) {
        if (window && window->getCanDestruct()) {
            if (!m_lastWindow && window != m_lastWindow)
                m_lastWindow->deleteLater();
            m_lastWindow = window;
        }
        saveWindowState(window);
        dialogManager->closeAllPropertyDialog();
    } else if (window && window->getCanDestruct()) {
        // fix bug 59239 drag事件的接受者的drop事件和发起drag事件的发起者的mousemove事件处理完成才能
        // 析构本窗口，检查当前窗口是否可以析构
        QPointer<DFileManagerWindow> pwindow = window;
        QTimer::singleShot(1000, this, [ = ]() {
            if (pwindow)
                pwindow->deleteLater();
        });
        qInfo() << "window deletelater !";
    }
    m_windows.remove(static_cast<const QWidget *>(sender()));

    if (window && window->currentUrl().scheme() == PLUGIN_SCHEME) {
        qInfo() << "delete plugin view:" << window->currentUrl().host();
        // NOTE [REN] 防止插件窗口关闭后，后台将信号发送到该窗口导致崩溃
        window->getFileView()->deleteLater();
    }
}

void WindowManager::onLastActivedWindowClosed(quint64 winId)
{
    QList<quint64> winIds = m_windows.values();
    foreach (quint64 id, winIds) {
        if (id != winId) {
            getWindowById(id)->close();
        }
    }

    if (QWidget *window = getWindowById(winId))
        window->close();
    //fix bug 32774 在复制大量文件时，用菜单的退出按钮，会造成这里qApp退出（等待拷贝线程），dfilesystemmodel中的addfile中qApp->processEvents
    //上的while循环等待，卡死主进程，所以复制进度条也卡死
//    qApp->quit();
}


/*!
 * \brief onShowNewWindow 同一个smb共享下的url正在挂载时或者启动没有获取挂载目录结束阻塞不显示，
 *
 *  当挂载这个smb或者获取挂载目录结束后，在创建相应的window并显示
 */
void WindowManager::onShowNewWindow()
{
    if (!DRootFileManager::instance()->isRootFileInited())
        return;
    QMutexLocker lk(&m_smbPointUrlMutex);
    QList<DUrl> smbUrlKey = m_smbPointUrl.keys();
    QList<DUrl>::Iterator it = smbUrlKey.begin();
    while (it != smbUrlKey.end()) {
        if (DRootFileManager::instance()->isRootFileContainSmb(*it)) {
            QList<DUrl> showUrls = m_smbPointUrl.values(*it);
            m_smbPointUrl.remove(*it);
            for (auto showUrl : showUrls) {
                if (showUrl.isValid())
                    showNewWindow(showUrl, true);
            }
        }
        ++it;
    }
}
/*!
 * \brief onRemoveNeedShowSmbUrl 同一个smb共享下的url正在挂载时或者启动没有获取挂载目录结束阻塞不显示，
 *
 *  当挂载失败后，清理掉要显示的窗口
 *
 * \param url 挂载失败的smb的url
 */
void WindowManager::onRemoveNeedShowSmbUrl(const DUrl &url)
{
    if (url.scheme() != SMB_SCHEME || url.host().isEmpty() || url.path().isNull()) {
        qWarning() << "url is not a valid smb url, url = " << url;
        return;
    }
    DUrl smbUrl(url);
    QString shareName = url.path();
    shareName = shareName.mid(1);
    shareName = shareName.mid(0, shareName.indexOf("/"));
    smbUrl.setPath("/" + shareName);
    QMutexLocker lk(&m_smbPointUrlMutex);
    m_smbPointUrl.remove(smbUrl);
}
