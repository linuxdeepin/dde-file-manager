// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/filemanagerwindow_p.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QUrl>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QHideEvent>
#include <QApplication>
#include <QScreen>
#include <QWindow>

namespace dfmbase {

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

/*!
 * \class FileManagerWindowPrivate
 * \brief
 */

FileManagerWindowPrivate::FileManagerWindowPrivate(const QUrl &url, FileManagerWindow *qq)
    : QObject(nullptr),
      q(qq),
      currentUrl(url)
{
    q->setWindowTitle(currentUrl.fileName());
}

bool FileManagerWindowPrivate::processKeyPressEvent(QKeyEvent *event)
{
    switch (event->modifiers()) {
    case Qt::NoModifier: {
        switch (event->key()) {
        case Qt::Key_F5:
            emit q->reqRefresh();
            return true;
        }
        break;
    }
    case Qt::ControlModifier: {
        switch (event->key()) {
        case Qt::Key_Tab:
            emit q->reqActivateNextTab();
            return true;
        case Qt::Key_Backtab:
            emit q->reqActivatePreviousTab();
            return true;
        case Qt::Key_F:
            emit q->reqSearchCtrlF();
            return true;
        case Qt::Key_L:
            emit q->reqSearchCtrlL();
            return true;
        case Qt::Key_Left:
            emit q->reqBack();
            return true;
        case Qt::Key_Right:
            emit q->reqForward();
            return true;
        case Qt::Key_W:
            emit q->reqCloseCurrentTab();
            return true;
        case Qt::Key_T:
            emit q->reqCreateTab();
            return true;
        case Qt::Key_N:
            emit q->reqCreateWindow();
            return true;
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
            emit q->reqTriggerActionByIndex(event->key() - Qt::Key_1);
            return true;
        }
        break;
    }
    case Qt::AltModifier:
    case Qt::AltModifier | Qt::KeypadModifier:
        if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_8) {
            emit q->reqActivateTabByIndex(event->key() - Qt::Key_1);
            return true;
        }

        switch (event->key()) {
        case Qt::Key_Left:
            emit q->reqBack();
            return true;
        case Qt::Key_Right:
            emit q->reqForward();
            return true;
        }
        break;
    case Qt::ControlModifier | Qt::ShiftModifier:
        if (event->key() == Qt::Key_Question) {
            emit q->reqShowHotkeyHelp();
            return true;
        } else if (event->key() == Qt::Key_Backtab) {
            emit q->reqActivatePreviousTab();
            return true;
        }
        break;
    }
    return false;
}

int FileManagerWindowPrivate::splitterPosition() const
{
    if (!splitter || splitter->sizes().isEmpty())
        return kDefaultLeftWidth;
    return splitter->sizes().at(0);
}

void FileManagerWindowPrivate::setSplitterPosition(int pos)
{
    if (splitter)
        splitter->setSizes({ pos, splitter->width() - pos - splitter->handleWidth() });
}

void FileManagerWindowPrivate::loadWindowState()
{
    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "WindowState").toMap();

    int width = state.value("width").toInt();
    int height = state.value("height").toInt();
    NetWmStates windowState = static_cast<NetWmStates>(state.value("state").toInt());

    // fix bug 30932,获取全屏属性，必须是width全屏和height全屏属性都满足，才判断是全屏
    if ((FMWindowsIns.windowIdList().isEmpty()) && ((windowState & kNetWmStateMaximizedHorz) != 0 && (windowState & kNetWmStateMaximizedVert) != 0)) {
        // make window to be maximized.
        // the following calling is copyed from QWidget::showMaximized()
        q->setWindowState((q->windowState() & ~(Qt::WindowMinimized | Qt::WindowFullScreen))
                          | Qt::WindowMaximized);
    } else {
        q->resize(width, height);
    }
}

void FileManagerWindowPrivate::saveWindowState()
{
    NetWmStates states { 0 };
    if (WindowUtils::isWayLand()) {
        if (q->isMaximized())
            states = static_cast<NetWmState>(kNetWmStateMaximizedHorz | kNetWmStateMaximizedVert);
    } else {
        /// The power by dxcb platform plugin
        states = static_cast<NetWmStates>(q->window()->windowHandle()->property("_d_netWmStates").toInt());
    }

    QVariantMap state;
    // fix bug 30932,获取全屏属性，必须是width全屏和height全屏属性都满足，才判断是全屏
    if ((states & kNetWmStateMaximizedHorz) == 0 || (states & kNetWmStateMaximizedVert) == 0) {
        state["width"] = q->size().width();
        state["height"] = q->size().height();
    } else {
        const QVariantMap &state1 = Application::appObtuselySetting()->value("WindowManager", "WindowState").toMap();
        state["width"] = state1.value("width").toInt();
        state["height"] = state1.value("height").toInt();
        state["state"] = static_cast<int>(states);
    }
    Application::appObtuselySetting()->setValue("WindowManager", "WindowState", state);
}

void FileManagerWindowPrivate::saveSidebarState()
{
    int width = splitterPosition();
    if (width >= kMinimumLeftWidth && width <= kMaximumLeftWidth) {
        QVariantMap state;
        state["sidebar"] = width;
        Application::appObtuselySetting()->setValue("WindowManager", "SplitterState", state);
    }
}

/*!
 * \class FileManagerWindow
 * \brief
 */

FileManagerWindow::FileManagerWindow(const QUrl &url, QWidget *parent)
    : DMainWindow(parent),
      d(new FileManagerWindowPrivate(url, this))
{
    initializeUi();
}

FileManagerWindow::~FileManagerWindow()
{
    auto menu = titlebar()->menu();
    if (menu) {
        delete menu;
        menu = nullptr;
    }
}

void FileManagerWindow::cd(const QUrl &url)
{
    d->currentUrl = url;
    if (d->titleBar)
        d->titleBar->setCurrentUrl(url);
    if (d->sideBar)
        d->sideBar->setCurrentUrl(url);
    if (d->workspace)
        d->workspace->setCurrentUrl(url);
    if (d->detailSpace)
        d->detailSpace->setCurrentUrl(url);
    emit currentUrlChanged(url);
}

bool FileManagerWindow::saveClosedSate() const
{
    return true;
}

QUrl FileManagerWindow::currentUrl() const
{
    return d->currentUrl;
}

void FileManagerWindow::moveCenter()
{
    QScreen *cursorScreen = WindowUtils::cursorScreen();
    if (!cursorScreen)
        return;
    int x = (cursorScreen->availableGeometry().width() - width()) / 2;
    int y = (cursorScreen->availableGeometry().height() - height()) / 2;
    move(QPoint(x, y) + cursorScreen->geometry().topLeft());
}

void FileManagerWindow::installTitleBar(AbstractFrame *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null TitleBar");
    std::call_once(d->titleBarFlag, [this, w]() {
        d->titleBar = w;
        d->titleBar->setCurrentUrl(d->currentUrl);
        titlebar()->setCustomWidget(d->titleBar);

        emit this->titleBarInstallFinished();
    });
}

void FileManagerWindow::installSideBar(AbstractFrame *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null setSideBar");
    std::call_once(d->sideBarFlag, [this, w]() {
        d->sideBar = w;
        d->splitter->insertWidget(0, d->sideBar);
        updateUi();   // setSizes is only valid when the splitter is non-empty

        d->sideBar->setContentsMargins(0, 0, 0, 0);
        d->sideBar->setMaximumWidth(d->kMaximumLeftWidth);
        d->sideBar->setMinimumWidth(d->kMinimumLeftWidth);
        d->sideBar->setCurrentUrl(d->currentUrl);

        emit this->sideBarInstallFinished();
    });
}

void FileManagerWindow::installWorkSpace(AbstractFrame *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null Workspace");
    std::call_once(d->workspaceFlag, [this, w]() {
        d->workspace = w;
        d->splitter->insertWidget(1, d->workspace);
        updateUi();   // setSizes is only valid when the splitter is non-empty

        // NOTE(zccrs): 保证窗口宽度改变时只会调整right view的宽度，侧边栏保持不变
        //              QSplitter是使用QLayout的策略对widgets进行布局，所以此处
        //              设置size policy可以生效
        QSizePolicy sp = d->workspace->sizePolicy();
        sp.setHorizontalStretch(1);
        d->workspace->setSizePolicy(sp);
        d->workspace->setCurrentUrl(d->currentUrl);
        d->workspace->installEventFilter(this);
        emit this->workspaceInstallFinished();
    });
}

/*!
 * \brief NOTE: shouldn't call it if detail button not clicked in titlebar
 * \param w
 */
void FileManagerWindow::installDetailView(AbstractFrame *w)
{
    d->detailSpace = w;
    if (d->detailSpace) {
        d->midLayout->setSpacing(0);
        d->midLayout->addWidget(d->detailSpace, 1);
        d->detailSpace->setVisible(false);
    }

    emit this->detailViewInstallFinished();
}

AbstractFrame *FileManagerWindow::titleBar() const
{
    return d->titleBar;
}

AbstractFrame *FileManagerWindow::sideBar() const
{
    return d->sideBar;
}

AbstractFrame *FileManagerWindow::workSpace() const
{
    return d->workspace;
}

AbstractFrame *FileManagerWindow::detailView() const
{
    return d->detailSpace;
}

void FileManagerWindow::loadState()
{
    d->loadWindowState();
}

void FileManagerWindow::saveState()
{
    d->saveSidebarState();
    d->saveWindowState();
}

void FileManagerWindow::closeEvent(QCloseEvent *event)
{
    // NOTE(zhangs): bug 59239
    emit aboutToClose();
    DMainWindow::closeEvent(event);
}

void FileManagerWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->y() <= d->titleBar->height()) {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    } else {
        DMainWindow::mouseDoubleClickEvent(event);
    }
}

void FileManagerWindow::moveEvent(QMoveEvent *event)
{
    DMainWindow::moveEvent(event);

    emit positionChanged(event->pos());
}

void FileManagerWindow::keyPressEvent(QKeyEvent *event)
{
    if (!d->processKeyPressEvent(event))
        return DMainWindow::keyPressEvent(event);
}

bool FileManagerWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (!d->workspace || watched != d->workspace)
        return false;

    if (event->type() != QEvent::KeyPress)
        return false;

    return d->processKeyPressEvent(static_cast<QKeyEvent *>(event));
}

void FileManagerWindow::initializeUi()
{
    titlebar()->setIcon(QIcon::fromTheme("dde-file-manager", QIcon::fromTheme("system-file-manager")));

    // size
    resize(d->kDefaultWindowWidth, d->kDefaultWindowHeight);
    setMinimumSize(d->kMinimumWindowWidth, d->kMinimumWindowHeight);

    // title bar
    titlebar()->setContentsMargins(0, 0, 0, 0);

    // splitter
    d->splitter = new Splitter(Qt::Orientation::Horizontal, this);
    d->splitter->setChildrenCollapsible(false);
    d->splitter->setHandleWidth(0);

    // central
    d->centralView = new QFrame(this);
    d->centralView->setObjectName("CentralView");
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QWidget *midWidget = new QWidget;
    d->midLayout = new QHBoxLayout;
    midWidget->setLayout(d->midLayout);
    d->midLayout->setContentsMargins(0, 0, 0, 0);
    d->midLayout->addWidget(d->splitter);
    mainLayout->addWidget(midWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    d->centralView->setLayout(mainLayout);
    setCentralWidget(d->centralView);
}

void FileManagerWindow::updateUi()
{
    if (d->sideBar && d->workspace) {
        const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "SplitterState").toMap();
        int splitterPos = state.value("sidebar", d->kDefaultLeftWidth).toInt();
        d->setSplitterPosition(splitterPos);
    }
}

}
