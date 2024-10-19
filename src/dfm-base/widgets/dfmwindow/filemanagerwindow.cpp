// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/filemanagerwindow_p.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <DPlatformTheme>
#include <DSizeMode>
#include <DIconTheme>

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

int FileManagerWindowPrivate::loadSidebarState() const
{
    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "SplitterState").toMap();
    auto pos = state.value("sidebar", kDefaultLeftWidth).toInt();
    if (pos >= kMinimumLeftWidth && pos <= kMaximumLeftWidth)
        return pos;
    return kDefaultLeftWidth;
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

void FileManagerWindowPrivate::resetTitleBarSize()
{
    if (iconLabel) {
        QSize size(DSizeModeHelper::element(24, 32), DSizeModeHelper::element(24, 32));
        iconLabel->setIconSize(size);
    }
    if (expandButton) {
        QSize size(DSizeModeHelper::element(32, 48), DSizeModeHelper::element(32, 48));
        expandButton->setIconSize(size);
    }
    if (iconArea) {
        QSize size(DSizeModeHelper::element(66, 95), DSizeModeHelper::element(40, 50));
        iconArea->setFixedSize(size);
    }
}

void FileManagerWindowPrivate::resetSideBarSize()
{
    if (sideBar) {
        sideBar->setMaximumWidth(kMaximumLeftWidth);
        sideBar->setMinimumWidth(kMinimumLeftWidth);
        if (splitter)
            lastSidebarExpandedPostion = splitter->sizes().at(0);
    }
}

void FileManagerWindowPrivate::animateSplitter(bool expanded)
{
    if (!sideBar || !sidebarSep)
        return;

    auto handleVisible = [expanded, this]() {
        sideBar->setVisible(expanded);
        sidebarSep->setVisible(expanded);
    };

    auto releaseAnimation = [this]() {
        if (curSplitterAnimation) {
            delete curSplitterAnimation;
            curSplitterAnimation = nullptr;
        }
    };

    // read from dconfig
    bool animationEnable = DConfigManager::instance()->value(kAnimationDConfName, kAnimationEnable, true).toBool();
    int duration = DConfigManager::instance()->value(kAnimationDConfName, kAnimationSidebarDuration, 366).toInt();
    auto curve = static_cast<QEasingCurve::Type>(DConfigManager::instance()->value(kAnimationDConfName, kAnimationSidebarSidebarCurve, 22).toInt());

    if (!animationEnable) {
        handleVisible();
        return;
    }

    sideBar->setVisible(true);
    sideBar->setMinimumWidth(1);

    bool lastAnimationStopped { false };
    if (curSplitterAnimation && curSplitterAnimation->state() == QAbstractAnimation::Running) {
        lastAnimationStopped = true;
        curSplitterAnimation->stop();
        releaseAnimation();
    }

    int start = splitter->sizes().at(0);
    int end = 1;
    if (expanded) {
        start = 1;
        end = lastSidebarExpandedPostion;
        sidebarSep->setVisible(true);
    } else {
        lastSidebarExpandedPostion = lastAnimationStopped ? lastSidebarExpandedPostion : splitter->sizes().at(0);
    }

    curSplitterAnimation = new QPropertyAnimation(splitter, "splitPosition");
    curSplitterAnimation->setEasingCurve(curve);
    curSplitterAnimation->setDuration(duration);
    curSplitterAnimation->setStartValue(start);
    curSplitterAnimation->setEndValue(end);

    connect(curSplitterAnimation, &QPropertyAnimation::finished,
            this, [this, expanded, handleVisible, releaseAnimation]() {
                if (expanded)
                    resetSideBarSize();
                handleVisible();
                releaseAnimation();
            });

    connect(curSplitterAnimation, &QPropertyAnimation::valueChanged, q, &FileManagerWindow::windowSplitterWidthChanged);

    Q_EMIT q->aboutPlaySplitterAnimation(start, end);
    curSplitterAnimation->start();
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
    auto hideTitlebar = [this]() {
        // hide titlebar
        titlebar()->setHidden(true);
        titlebar()->setFixedHeight(0);
        setTitlebarShadowEnabled(false);
    };
    hideTitlebar();

    // size
    resize(d->kDefaultWindowWidth, d->kDefaultWindowHeight);
    setMinimumSize(d->kMinimumWindowWidth, d->kMinimumWindowHeight);

    d->centralView = new QFrame(this);
    d->centralView->setObjectName("CentralView");

    d->midLayout = new QHBoxLayout;
    d->midLayout->setContentsMargins(0, 0, 0, 0);
    d->midLayout->setSpacing(0);

    d->rightLayout = new QVBoxLayout;
    d->rightLayout->setContentsMargins(0, 0, 0, 0);
    d->rightLayout->setSpacing(0);

    d->rightBottomLayout = new QHBoxLayout;
    d->rightBottomLayout->setContentsMargins(0, 0, 0, 0);
    d->rightBottomLayout->setSpacing(0);

    // icon label
    d->iconLabel = new DIconButton(this);
    d->iconLabel->setWindowFlags(Qt::WindowTransparentForInput);
    d->iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    d->iconLabel->setFocusPolicy(Qt::NoFocus);
    d->iconLabel->setFlat(true);

    // expand button
    d->expandButton = new DIconButton(this);
    d->expandButton->setProperty("expand", true);
    d->expandButton->setFlat(true);

    // connections
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [hideTitlebar, this]() {
        hideTitlebar();
        d->resetTitleBarSize();
    });

    connect(DGuiApplicationHelper::instance()->systemTheme(), &DPlatformTheme::iconThemeNameChanged, this, [this]() {
        d->iconLabel->update();
    });

    connect(d->expandButton, &DIconButton::clicked, this, [this]() {
        bool isExpand = d->expandButton->property("expand").toBool();
        d->expandButton->setProperty("expand", !isExpand);
        d->animateSplitter(!isExpand);
    });
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
        initializeUi();

        emit this->titleBarInstallFinished();
    });
}

void FileManagerWindow::installSideBar(AbstractFrame *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null setSideBar");
    std::call_once(d->sideBarFlag, [this, w]() {
        d->sideBar = w;
        d->sideBar->setCurrentUrl(d->currentUrl);

        initializeUi();
        updateUi();   // setSizes is only valid when the splitter is non-empty
        emit this->sideBarInstallFinished();
    });
}

void FileManagerWindow::installWorkSpace(AbstractFrame *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null Workspace");
    std::call_once(d->workspaceFlag, [this, w]() {
        d->workspace = w;

        d->workspace->setCurrentUrl(d->currentUrl);
        d->workspace->installEventFilter(this);

        initializeUi();
        updateUi();   // setSizes is only valid when the splitter is non-empty
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
        d->rightBottomLayout->addWidget(d->detailSpace, 1);
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
    if (d->sideBar && d->titleBar && d->workspace) {
        // left area
        {
            d->sideBar->setContentsMargins(0, 0, 0, 0);
            d->resetSideBarSize();
            d->sidebarSep = new DVerticalLine(this);
            d->sidebarSep->setContentsMargins(0, 0, 0, 0);
            d->sidebarSep->setVisible(true);
            d->sideBar->layout()->addWidget(d->sidebarSep);
        }

        // icon area
        {
            d->iconArea = new QWidget(this);
            QHBoxLayout *leftAreaLayout = new QHBoxLayout(d->iconArea);
            d->iconArea->setWindowFlag(Qt::WindowTransparentForInput);
            d->iconArea->move(0, 0);
            leftAreaLayout->setContentsMargins(0, 0, 0, 0);
            leftAreaLayout->addSpacing(10);
            if (d->iconLabel) {
                auto icon = QIcon::fromTheme("dde-file-manager", QIcon::fromTheme("system-file-manager"));
                d->iconLabel->setIcon(icon);
                leftAreaLayout->addWidget(d->iconLabel, 0, Qt::AlignLeading | Qt::AlignVCenter);
            }
            if (d->expandButton) {
                d->expandButton->setIcon(DDciIcon::fromTheme("window_sidebar"));
                leftAreaLayout->addWidget(d->expandButton, 0, Qt::AlignLeading | Qt::AlignVCenter);
            }
            d->resetTitleBarSize();
            d->iconArea->show();
        }

        // right area
        {
            d->rightArea = new QFrame(this);
            auto minimumWidth = d->kMinimumWindowWidth - d->kMaximumLeftWidth;
            d->rightArea->setMinimumWidth(minimumWidth > 0 ? minimumWidth : 80);
            // NOTE(zccrs): 保证窗口宽度改变时只会调整right view的宽度，侧边栏保持不变
            //              QSplitter是使用QLayout的策略对widgets进行布局，所以此处
            //              设置size policy可以生效
            QSizePolicy sp = d->rightArea->sizePolicy();
            sp.setHorizontalStretch(1);
            d->rightArea->setSizePolicy(sp);

            d->rightLayout->addWidget(d->titleBar);
            d->rightLayout->addLayout(d->rightBottomLayout, 1);
            d->rightBottomLayout->addWidget(d->workspace, 1);
            d->rightArea->setLayout(d->rightLayout);
        }

        // splitter
        {
            d->splitter = new Splitter(Qt::Orientation::Horizontal, this);
            d->splitter->setChildrenCollapsible(false);
            d->splitter->setHandleWidth(0);
            d->splitter->addWidget(d->sideBar);
            d->splitter->addWidget(d->rightArea);
        }

        // central
        {
            QVBoxLayout *mainLayout = new QVBoxLayout;
            QWidget *midWidget = new QWidget;
            midWidget->setLayout(d->midLayout);
            mainLayout->addWidget(midWidget);
            mainLayout->setSpacing(0);
            mainLayout->setContentsMargins(0, 0, 0, 0);
            d->midLayout->insertWidget(0, d->splitter);
            d->centralView->setLayout(mainLayout);
            setCentralWidget(d->centralView);
        }
    }
}

void FileManagerWindow::updateUi()
{
    int splitterPos = d->loadSidebarState();
    d->setSplitterPosition(splitterPos);
}

}   // namespace dfmbase
