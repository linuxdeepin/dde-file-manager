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

#include "dtoolbar.h"
#include "dfmcrumbbar.h"
#include "historystack.h"
#include "historystack.h"
#include "windowmanager.h"
#include "dfileservices.h"
#include "dfmeventdispatcher.h"
#include "dfmaddressbar.h"

#include "dfmevent.h"
#include "app/define.h"
#include "app/filesignalmanager.h"

#include "dfilemenumanager.h"

#include "singleton.h"
#include "views/dfileview.h"
#include "views/dfilemanagerwindow.h"
#include "views/dfmactionbutton.h"
#include "models/networkfileinfo.h"
#include "gvfs/gvfsmountmanager.h"
#include "accessibility/ac-lib-file-manager.h"

#include <DButtonBox>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QToolButton>
#include <QPushButton>
#include <QtGlobal>

const int DToolBar::ButtonWidth = 36;
const int DToolBar::ButtonHeight = 36;
static const QSize iconSize(16, 16);

/*!
 * \class DToolBar
 *
 * \brief DToolBar is the toolbar (topbar) of the file manager
 *
 * This widget is the container of the navigation arrows, switching view buttons, menu button,
 * search button and the crumb bar (and also the address bar).
 *
 * \sa DFMCrumbBar, DFMAddressBar
 */

DToolBar::DToolBar(QWidget *parent) : QFrame(parent)
{
    AC_SET_OBJECT_NAME( this, AC_COMPUTER_TITLE_BAR);
    AC_SET_ACCESSIBLE_NAME( this, AC_COMPUTER_TITLE_BAR);

    initData();
    initUI();
    initConnect();
}

DToolBar::~DToolBar()
{
//    delete m_navStack;
}

void DToolBar::initData()
{
//    m_navStack = new HistoryStack(65536);
}

void DToolBar::initUI()
{
    setFocusPolicy(Qt::NoFocus);
    initAddressToolBar();
    initContollerToolBar();

    m_detailButton = new QToolButton(this);
    m_detailButton->setFixedWidth(ButtonWidth);
    m_detailButton->setFixedHeight(ButtonHeight);
    m_detailButton->setObjectName("detailButton");
    AC_SET_ACCESSIBLE_NAME( m_detailButton, AC_COMPUTER_TITLE_BAR_DETAIL_BTN);
    m_detailButton->setCheckable(true);
    m_detailButton->setFocusPolicy(Qt::NoFocus);
    m_detailButton->setIcon(QIcon::fromTheme("dfm_rightview_detail"));
    m_detailButton->setIconSize(iconSize);
    m_detailButton->setFixedSize(36, 36);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_addressToolBar);
    //    mainLayout->addSpacing(22);
    mainLayout->addWidget(m_contollerToolBar);

    //    mainLayout->addSpacing(22);
    mainLayout->addWidget(m_detailButton);

    mainLayout->addSpacing(0);
    mainLayout->setContentsMargins(14, 0, 4, 0);
    setLayout(mainLayout);
}

void DToolBar::initAddressToolBar()
{
    m_addressToolBar = new QFrame;
    m_addressToolBar->setObjectName("AddressToolBar");
    AC_SET_OBJECT_NAME( m_addressToolBar, AC_COMPUTER_TITLE_BAR_ADRESS);
    //m_addressToolBar->setFixedHeight(40);

    QHBoxLayout *backForwardLayout = new QHBoxLayout;

    m_backButton = new DButtonBoxButton(QStyle::SP_ArrowBack);
    AC_SET_OBJECT_NAME( m_backButton, AC_COMPUTER_TITLE_BAR_BACK_BTN);
    AC_SET_ACCESSIBLE_NAME( m_backButton, AC_COMPUTER_TITLE_BAR_BACK_BTN);
    m_backButton->setDisabled(true);
    m_backButton->setFixedWidth(36);

    m_forwardButton = new DButtonBoxButton(QStyle::SP_ArrowForward);
    AC_SET_OBJECT_NAME( m_forwardButton, AC_COMPUTER_TITLE_BAR_FORWARD_BTN);
    AC_SET_ACCESSIBLE_NAME( m_forwardButton, AC_COMPUTER_TITLE_BAR_FORWARD_BTN);
    m_forwardButton->setDisabled(true);
    m_forwardButton->setFixedWidth(36);

    QList<DButtonBoxButton *> buttonList;
    buttonList << m_backButton << m_forwardButton;

    DButtonBox *buttonBox = new DButtonBox(this);
    AC_SET_OBJECT_NAME( buttonBox, AC_COMPUTER_TITLE_BAR_BTN_BOX);
    AC_SET_ACCESSIBLE_NAME( buttonBox, AC_COMPUTER_TITLE_BAR_BTN_BOX);
    buttonBox->setButtonList(buttonList, false);
    buttonBox->setFocusPolicy(Qt::NoFocus);

    m_searchButton = new QPushButton(this);
    m_searchButton->setObjectName("searchButton");
    AC_SET_ACCESSIBLE_NAME( m_searchButton, AC_COMPUTER_TITLE_BAR_SERACH_BTN);
    m_searchButton->setFixedSize(ButtonWidth, ButtonHeight);
    m_searchButton->setFocusPolicy(Qt::NoFocus);
    m_searchButton->setFlat(true);
    m_searchButton->setIcon(QIcon::fromTheme("search"));
    m_searchButton->setIconSize(iconSize);


    backForwardLayout->addWidget(buttonBox);
    backForwardLayout->setSpacing(0);
    backForwardLayout->setContentsMargins(0, 0, 0, 0);


    QFrame *crumbAndSearch = new QFrame;
    m_crumbWidget = new DFMCrumbBar(this);
    crumbAndSearch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    AC_SET_OBJECT_NAME( crumbAndSearch, AC_COMPUTER_TITLE_BAR_CRUMB_SERACH_FRAME);
    AC_SET_ACCESSIBLE_NAME( crumbAndSearch, AC_COMPUTER_TITLE_BAR_CRUMB_SERACH_FRAME);

    QHBoxLayout *comboLayout = new QHBoxLayout;
    comboLayout->addWidget(m_crumbWidget);
    comboLayout->addWidget(m_searchButton);
    //    comboLayout->setSpacing(10);
    comboLayout->setContentsMargins(0, 0, 0, 0);


    crumbAndSearch->setLayout(comboLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(backForwardLayout);
    mainLayout->addWidget(crumbAndSearch);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    //    mainLayout->setSpacing(10);
    m_addressToolBar->setLayout(mainLayout);

}

void DToolBar::initContollerToolBar()
{
    m_contollerToolBar = new QFrame;
    m_contollerToolBar->setObjectName("ContollerToolBar");
    AC_SET_ACCESSIBLE_NAME( m_contollerToolBar, AC_COMPUTER_TITLE_BAR_CONTOLLER_TOOL_BAR);
    m_contollerToolBar->setFrameShape(QFrame::NoFrame);
    m_contollerToolBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_contollerToolBarContentLayout = new QHBoxLayout(m_contollerToolBar);
    m_contollerToolBarContentLayout->setContentsMargins(14, 1, 14, 1);
    m_contollerToolBarContentLayout->setSpacing(20);
}

void DToolBar::initConnect()
{
    connect(m_detailButton, &QAbstractButton::clicked, this, &DToolBar::detailButtonClicked);
    connect(m_backButton, &DButtonBoxButton::clicked, this, &DToolBar::onBackButtonClicked);
    connect(m_forwardButton, &DButtonBoxButton::clicked, this, &DToolBar::onForwardButtonClicked);
    connect(m_crumbWidget, &DFMCrumbBar::addressBarContentEntered, this, &DToolBar::searchBarTextEntered);
    connect(m_crumbWidget, &DFMCrumbBar::crumbListItemSelected, this, [this](const DUrl & url) {
        //判断网络文件是否可以到达
        if (DFileService::instance()->checkGvfsMountfileBusy(url)) {
            return;
        }
        DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(m_crumbWidget, url, window());
    });
    connect(m_crumbWidget, &DFMCrumbBar::addressBarShown, this, &DToolBar::searchBarActivated);
    connect(m_crumbWidget, &DFMCrumbBar::addressBarHidden, this, &DToolBar::searchBarDeactivated);
    connect(m_searchButton, &QPushButton::clicked, this, &DToolBar::onSearchButtonClicked);
    connect(fileSignalManager, &FileSignalManager::requestSearchCtrlF, this, &DToolBar::handleHotkeyCtrlF);
    connect(fileSignalManager, &FileSignalManager::requestSearchCtrlL, this, &DToolBar::handleHotkeyCtrlL);
    connect(this, &DToolBar::toolbarUrlChanged, m_crumbWidget, &DFMCrumbBar::updateCurrentUrl);
    connect(gvfsMountManager, &GvfsMountManager::mount_removed, this, [this](const QDiskInfo& diskInfo) {
        Q_UNUSED(diskInfo)
        this->updateBackForwardButtonsState();
    });

    DFileManagerWindow *window = qobject_cast<DFileManagerWindow *>(parent());

    if (window) {
        connect(window, &DFileManagerWindow::currentViewStateChanged, this, [this, window] {
            // fix bug 87535 搜索栏只有搜索状态才显示加载动画
            if (window->currentUrl().isSearchFile()) {
                if (window->currentViewState() == DFMBaseView::ViewBusy)
                    m_crumbWidget->playAddressBarAnimation();
                else
                    m_crumbWidget->stopAddressBarAnimation();
            }
        });
    }
}

DFMCrumbBar *DToolBar::getCrumbWidget()
{
    return m_crumbWidget;
}

void DToolBar::searchBarActivated()
{
    toggleSearchButtonState(true);
}

void DToolBar::searchBarDeactivated()
{
    toggleSearchButtonState(false);
}

/**
 * \brief DToolBar::searchBarTextEntered
 *
 * Set the tab bar when return press is detected
 * on search bar.
 */
void DToolBar::searchBarTextEntered(const QString textEntered)
{
    QString text = textEntered;

    if (text.isEmpty()) {
        //m_searchBar->clearText();
        return;
    }

    const QString &currentDir = QDir::currentPath();
    DUrl currentUrl;
    if (auto fmWindow = qobject_cast<DFileManagerWindow *>(topLevelWidget()))
        currentUrl = fmWindow->currentUrl();
    else
        qCritical() << "DFileManagerWindow is null";
    //判断网络文件是否可以到达
    if (DFileService::instance()->checkGvfsMountfileBusy(currentUrl, true))
        return;

    if (currentUrl.isLocalFile()) {
        QDir::setCurrent(currentUrl.toLocalFile());
    }

    setEnterText(text);

    DUrl inputUrl = DUrl::fromUserInput(text, false); ///###: here, judge whether the text is a local file path.

    QDir::setCurrent(currentDir);

    //fix bug 32652 当连接了同一台机器的smb共享时，就缓存了它，第二次再去连接smb访问时，使用了缓存
    NetworkManager::NetworkNodes.remove(inputUrl);

    DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(this, inputUrl, window());
}

void DToolBar::onSearchButtonClicked()
{
    if (!m_searchButtonAsbState) {
        m_crumbWidget->showAddressBar("");
    } else {
        // toggle asb visible
        if (DFileManagerWindow *dfmWindow = qobject_cast<DFileManagerWindow *>(window())) {
            bool oldState = dfmWindow->isAdvanceSearchBarVisible();
            dfmWindow->toggleAdvanceSearchBar(!oldState);
            m_searchButton->setDown(!oldState);
        }
        else {
            qCritical() << "DFileManagerWindow is null";
        }
    }
}

void DToolBar::currentUrlChanged(const DFMEvent &event)
{
    if (event.windowId() != WindowManager::getWindowId(this)) {
        return;
    }

    if (event.sender() == m_crumbWidget) {
        pushUrlToHistoryStack(event.fileUrl());
        return;
    }

    emit toolbarUrlChanged(event.fileUrl());

    if (event.sender() == this) {
        return;
    }

    pushUrlToHistoryStack(event.fileUrl());
}

void DToolBar::back()
{
    DUrl url = m_navStack->back();
    //判断网络文件是否可以到达
    if (DFileService::instance()->checkGvfsMountfileBusy(url)) {
        return;
    }
    if (!url.isEmpty()) {
        updateBackForwardButtonsState();
        DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(this, url, window());
    }
}

void DToolBar::forward()
{
    DUrl url = m_navStack->forward();
    //判断网络文件是否可以到达
    if (DFileService::instance()->checkGvfsMountfileBusy(url)) {
        return;
    }
    if (!url.isEmpty()) {
        updateBackForwardButtonsState();
        DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(this, url, window());
    }
}

void DToolBar::handleHotkeyCtrlF(quint64 winId)
{
    // fix bug 65436
    // 仅当搜索/过滤按钮可见时才响应Ctrl+F
    if (winId == WindowManager::getWindowId(this) && m_searchButton->isVisible()) {
        onSearchButtonClicked();
    }
}

void DToolBar::handleHotkeyCtrlL(quint64 winId)
{
    if (winId == WindowManager::getWindowId(this)) {
        m_crumbWidget->showAddressBar(qobject_cast<DFileManagerWindow *>(topLevelWidget())->currentUrl());
    }
}

void DToolBar::pushUrlToHistoryStack(DUrl url)
{
    if (!m_navStack) {
        return;
    }

    m_navStack->append(url);
    updateBackForwardButtonsState();
}

void DToolBar::addHistoryStack()
{
    m_navStacks.append(std::shared_ptr<HistoryStack>(new HistoryStack(65536)));
}

void DToolBar::switchHistoryStack(const int index)
{
    m_navStack = m_navStacks.at(index);
    if (!m_navStack)
        return;
    updateBackForwardButtonsState();
}

/*!
 * \brief Switch Search Button State
 *
 * \param asb Make search button control Advance Search Bar
 */
void DToolBar::toggleSearchButtonState(bool asb)
{
    if (asb) {
        m_searchButton->setHidden(true);
        m_searchButton->setObjectName("filterButton");
        m_searchButton->setIcon(QIcon::fromTheme("dfm_view_filter"));
        m_searchButton->style()->unpolish(m_searchButton);
        m_searchButton->style()->polish(m_searchButton);
        m_searchButton->setFlat(true);
        m_searchButtonAsbState = true;
    } else {
        m_searchButton->setHidden(false);
        m_searchButton->style()->unpolish(m_searchButton);
        m_searchButton->style()->polish(m_searchButton);
        m_searchButton->setIcon(QIcon::fromTheme("search"));
        m_searchButton->setDown(false);
        m_searchButtonAsbState = false;
        if (DFileManagerWindow *dfmWindow = qobject_cast<DFileManagerWindow *>(window())) {
            dfmWindow->toggleAdvanceSearchBar(false);
        }
        else {
            qCritical() << "window() is null or faile to cast to DFileManagerWindow.";
        }
    }
}

void DToolBar::removeNavStackAt(int index)
{
    m_navStacks.removeAt(index);

    if (index < m_navStacks.count())
        m_navStack = m_navStacks.at(index);
    else
        m_navStack = m_navStacks.at(m_navStacks.count() - 1);

    if (!m_navStack)
        return;
    if (m_navStack->size() > 1)
        m_backButton->setEnabled(true);
    else
        m_backButton->setEnabled(false);

    if (m_navStack->isLast())
        m_forwardButton->setEnabled(false);
    else
        m_forwardButton->setEnabled(true);
}

void DToolBar::moveNavStacks(int from, int to)
{
    m_navStacks.move(from, to);
}

int DToolBar::navStackCount() const
{
    return m_navStacks.count();
}

void DToolBar::updateBackForwardButtonsState()
{
    if (!m_navStack || m_navStack->size() <= 1) {
        m_backButton->setEnabled(false);
        m_forwardButton->setEnabled(false);
    } else {
        //前目录不存在时（外设被拔出、网络目录被卸载），回退按钮需要置灰
        if (m_navStack->isFirst() || !m_navStack->backIsExist()) {
            m_backButton->setEnabled(false);
        } else {
            m_backButton->setEnabled(true);
        }

        //后目录不存在时（外设被拔出、网络目录被卸载），向前按钮需要置灰
        if (m_navStack->isLast() || !m_navStack->forwardIsExist()) {
            m_forwardButton->setEnabled(false);
        } else {
            m_forwardButton->setEnabled(true);
        }
    }
}

void DToolBar::setCustomActionList(const QList<QAction *> &list)
{
    m_actionList = list;

    for (DFMActionButton *button : m_contollerToolBar->findChildren<DFMActionButton *>()) {
        m_contollerToolBarContentLayout->removeWidget(button);
        button->deleteLater();
    }

    for (int i = 0; i < list.count(); ++i) {
        DFMActionButton *button = new DFMActionButton(this);
        button->setFixedSize(36, 36);
        button->setFocusPolicy(Qt::NoFocus);
        button->setAction(list.at(i));
        button->setIconSize(iconSize);

        QString acMark = QString("action_botton_%1").arg(i+1);
        AC_SET_OBJECT_NAME( button, acMark);
        AC_SET_ACCESSIBLE_NAME( button, acMark);

        m_contollerToolBarContentLayout->addWidget(button);

        if (list.at(i)->isVisible()) {
            button->show();
        }
    }

    m_contollerToolBar->setHidden(list.isEmpty());

    if (m_detailButton) {
        m_detailButton->setHidden(list.isEmpty());
        if (m_detailButton->isChecked() && list.isEmpty())
            m_detailButton->click();
    }
}

void DToolBar::triggerActionByIndex(int index)
{
    if (QAction *action = m_actionList.value(index)) {
        if (action->isVisible()) {
            action->trigger();
        }
    }
}

void DToolBar::setSearchButtonVisible(bool visble)
{
    if (m_searchButton->isVisible() != visble)
        m_searchButton->setVisible(visble);
}

void DToolBar::onBackButtonClicked()
{
    DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMBackEvent>(this), qobject_cast<DFileManagerWindow *>(window()));
}

void DToolBar::onForwardButtonClicked()
{
    DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMForwardEvent>(this), qobject_cast<DFileManagerWindow *>(window()));
}

void DToolBar::setEnterText(QString &text)
{
    //【recent:】后无斜杠、单斜杠、双斜杠、三斜杠均响应跳转只最近使用页面，无视其后输入的文件名是否存在
    if (text.startsWith("recent:")) {
        text = "recent:///";
    } else if (text.startsWith("usershare:")) {
        text = "usershare:///";
    } else if (text.startsWith("computer:")) {
        text = "computer:///";
    }
}
