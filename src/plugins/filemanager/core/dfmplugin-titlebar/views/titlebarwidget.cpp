// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebarwidget.h"
#include "tabbar.h"
#include "tabbar.h"
#include "searcheditwidget.h"
#include "events/titlebareventcaller.h"
#include "utils/crumbinterface.h"
#include "utils/crumbmanager.h"
#include "utils/titlebarhelper.h"
#include "utils/tabbarmanager.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-framework/event/event.h>

#include <DTitlebar>
#include <DFrame>
#include <DWindowCloseButton>
#include <DWindowMaxButton>
#include <DWindowMinButton>
#include <DWindowOptionButton>
#include <DSearchEdit>
#include <DGuiApplicationHelper>
#include <dtkwidget_global.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QHBoxLayout>
#include <QEvent>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

inline constexpr int kSpacing { 10 };
inline constexpr int kCriticalWidth { 650 };
inline constexpr int kSearchEditMaxWidth { 240 };

TitleBarWidget::TitleBarWidget(QFrame *parent)
    : AbstractFrame(parent)
{
    initializeUi();
    initConnect();
}

void TitleBarWidget::setCurrentUrl(const QUrl &url)
{
    TabBarManager::instance()->setCurrentUrl(this, url);

    titlebarUrl = url;
    emit currentUrlChanged(url);
}

QUrl TitleBarWidget::currentUrl() const
{
    return titlebarUrl;
}

NavWidget *TitleBarWidget::navWidget() const
{
    return curNavWidget;
}

DTitlebar *TitleBarWidget::titleBar() const
{
    return topBar;
}

void TitleBarWidget::startSpinner()
{
    searchEditWidget->startSpinner();
}

void TitleBarWidget::stopSpinner()
{
    searchEditWidget->stopSpinner();
}

void TitleBarWidget::initTabBar(const quint64 windowId)
{
    topBarCustomLayout->addWidget(createTabBar(windowId), 1);
}

void TitleBarWidget::currentTabChanged(const int index)
{
    curNavWidget->switchHistoryStack(index);
}

void TitleBarWidget::handleSplitterAnimation(const QVariant &position)
{
    placeholder->setFixedWidth(qMax(0, 95 - position.toInt()));
}

void TitleBarWidget::handleHotkeyCtrlF()
{
    searchEditWidget->activateEdit();
}

void TitleBarWidget::handleHotkeyCtrlL()
{
    showAddrsssBar(currentUrl());
}

void TitleBarWidget::handleHotketSwitchViewMode(int mode)
{
    // Press ctrl+1
    if (mode == 0)
        TitleBarEventCaller::sendViewMode(this, ViewMode::kIconMode);
    // Press ctrl+2
    if (mode == 1)
        TitleBarEventCaller::sendViewMode(this, ViewMode::kListMode);
    // Press ctrl+3
    if (mode == 2 && DConfigManager::instance()->value(kViewDConfName, kTreeViewEnable, true).toBool())
        TitleBarEventCaller::sendViewMode(this, ViewMode::kTreeMode);
}

void TitleBarWidget::handleHotketCloseCurrentTab()
{
    quint64 winId = TitleBarHelper::windowId(this);
    TabBarManager::instance()->closeCurrentTab(winId);
}

void TitleBarWidget::handleHotketNextTab()
{
    quint64 winId = TitleBarHelper::windowId(this);
    TabBarManager::instance()->activateNextTab(winId);
}

void TitleBarWidget::handleHotketPreviousTab()
{
    quint64 winId = TitleBarHelper::windowId(this);
    TabBarManager::instance()->activatePreviousTab(winId);
}

void TitleBarWidget::handleHotketCreateNewTab()
{
    quint64 winId = TitleBarHelper::windowId(this);
    TabBarManager::instance()->createNewTab(winId);
}

void TitleBarWidget::handleHotketActivateTab(const int index)
{
    quint64 winId = TitleBarHelper::windowId(this);
    TabBarManager::instance()->activateTab(winId, index);
}

void TitleBarWidget::initializeUi()
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::NoFocus);

    topBar = new DTitlebar;
    topBar->setFixedHeight(40);
    auto topBarLayout = topBar->layout();
    if (topBarLayout) {
        topBarLayout->setContentsMargins(0, 0, 0, 0);
        topBarLayout->setSpacing(0);
    }

    auto optionBtn = topBar->findChild<DWindowOptionButton *>("DTitlebarDWindowOptionButton");
    if (optionBtn) {
        optionBtn->setFixedSize(40, 40);
    }
    auto closeBtn = topBar->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
    if (closeBtn) {
        closeBtn->setFixedSize(40, 40);
    }
    auto minBtn = topBar->findChild<DWindowMinButton *>("DTitlebarDWindowMinButton");
    if (minBtn) {
        minBtn->setFixedSize(40, 40);
    }
    auto maxBtn = topBar->findChild<DWindowMaxButton *>("DTitlebarDWindowMaxButton");
    if (maxBtn) {
        maxBtn->setFixedSize(40, 40);
    }

    QWidget *topCustomWidget = new QWidget(topBar);
    topBarCustomLayout = new QHBoxLayout;
    topBarCustomLayout->setContentsMargins(0, 0, 0, 0);
    topBarCustomLayout->setSpacing(0);

    placeholder = new QWidget(topCustomWidget);
    placeholder->setFixedHeight(40);
    placeholder->setFixedWidth(0);
    placeholder->setVisible(true);
    placeholder->setObjectName("Placeholder");
    placeholder->setAttribute(Qt::WA_TranslucentBackground);
    placeholder->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    topBarCustomLayout->addWidget(placeholder);

    topCustomWidget->setLayout(topBarCustomLayout);
    topBar->setCustomWidget(topCustomWidget);

    // nav
    curNavWidget = new NavWidget;
    curNavWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // address
    addressBar = new AddressBar;
    addressBar->installEventFilter(this);

    // crumb
    crumbBar = new CrumbBar;

    // search widget
    searchEditWidget = new SearchEditWidget(this);
    searchEditWidget->setFixedHeight(30);
    searchEditWidget->setMaximumWidth(kSearchEditMaxWidth);

    // option button
    optionButtonBox = new OptionButtonBox;
    optionButtonBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(crumbBar), AcName::kAcComputerTitleBarAddress);
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(optionButtonBox), AcName::kAcComputerTitleBarOptionBtnBox);
#endif

    titleBarLayout = new QVBoxLayout(this);
    titleBarLayout->setContentsMargins(0, 0, 0, 0);
    titleBarLayout->setSpacing(0);
    titleBarLayout->addWidget(topBar);

    bottomBarLayout = new QHBoxLayout;
    bottomBarLayout->setContentsMargins(kSpacing, 5, kSpacing, 5);
    bottomBarLayout->setSpacing(0);

    bottomBarLayout->addWidget(curNavWidget);
    bottomBarLayout->addWidget(addressBar);
    bottomBarLayout->addWidget(crumbBar);
    bottomBarLayout->addWidget(optionButtonBox, 0, Qt::AlignRight);

    bottomBarLayout->addSpacing(10);
    bottomBarLayout->addWidget(searchEditWidget, 1);


    titleBarLayout->addLayout(bottomBarLayout);

    DHorizontalLine *line = new DHorizontalLine(this);
    line->setFixedHeight(1);
    line->setContentsMargins(0, 0, 0, 0);
    line->setVisible(true);
    titleBarLayout->addWidget(line);

    setLayout(titleBarLayout);

    initUiForSizeMode();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    showCrumbBar();
}

void TitleBarWidget::initConnect()
{
    connect(this, &TitleBarWidget::currentUrlChanged, optionButtonBox, &OptionButtonBox::onUrlChanged);
    connect(this, &TitleBarWidget::currentUrlChanged, crumbBar, &CrumbBar::onUrlChanged);
    connect(this, &TitleBarWidget::currentUrlChanged, curNavWidget, &NavWidget::onUrlChanged);
    connect(crumbBar, &CrumbBar::showAddressBarText, addressBar, [this](const QString &text) {
        crumbBar->hide();
        addressBar->show();
        addressBar->setFocus();
        addressBar->setText(text);
    });
    connect(crumbBar, &CrumbBar::hideAddressBar, this, [this](bool cd) {
        addressBar->hide();
        crumbBar->show();
        if (cd)
            TitleBarEventCaller::sendCd(this, crumbBar->lastUrl());
    });
    connect(crumbBar, &CrumbBar::selectedUrl, this, [this](const QUrl &url) {
        TitleBarEventCaller::sendCd(this, url);
    });
    connect(crumbBar, &CrumbBar::editUrl, this, &TitleBarWidget::showAddrsssBar);

    connect(addressBar, &AddressBar::escKeyPressed, this, [this]() {
        if (crumbBar->controller())
            crumbBar->controller()->processAction(CrumbInterface::kEscKeyPressed);
    });
    connect(addressBar, &AddressBar::lostFocus, this, [this]() {
        if (crumbBar->controller())
            crumbBar->controller()->processAction(CrumbInterface::kAddressBarLostFocus);
    });
    connect(addressBar, &AddressBar::clearButtonPressed, this, [this]() {
        if (crumbBar->controller())
            crumbBar->controller()->processAction(CrumbInterface::kClearButtonPressed);
    });

    connect(addressBar, &AddressBar::urlChanged, this, &TitleBarWidget::onAddressBarJump);
    connect(addressBar, &AddressBar::pauseButtonClicked, this, [this]() {
        TitleBarEventCaller::sendStopSearch(this);
    });

    connect(searchEditWidget, &SearchEditWidget::clearButtonClicked, this, [this]() {
        TitleBarEventCaller::sendCd(this, crumbBar->lastUrl());
    });

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        initUiForSizeMode();
    });
#endif
}

void TitleBarWidget::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    crumbBar->setFixedHeight(DSizeModeHelper::element(24, 36));
    addressBar->setFixedHeight(DSizeModeHelper::element(24, 36));
#else
    crumbBar->setFixedHeight(36);
    addressBar->setFixedHeight(36);
#endif
    topBar->setFixedHeight(40);
}

void TitleBarWidget::showAddrsssBar(const QUrl &url)
{
    crumbBar->hide();
    addressBar->show();
    addressBar->setFocus();
    addressBar->setCurrentUrl(url);
}

void TitleBarWidget::showCrumbBar()
{
    if (crumbBar)
        crumbBar->show();

    if (addressBar) {
        addressBar->clear();
        addressBar->hide();
    }
    setFocus();
}

void TitleBarWidget::showSearchFilterButton(bool visible)
{
    searchEditWidget->setAdvancedButtonVisible(visible);
}

void TitleBarWidget::setViewModeState(int mode)
{
    if (optionButtonBox)
        optionButtonBox->setViewMode(mode);
}

bool TitleBarWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this && event->type() == QEvent::Show) {
        activateWindow();
        return false;
    }

    if (watched == addressBar) {
        switch (event->type()) {
        case QEvent::Hide:
            if (!crumbBar->controller()->isKeepAddressBar()) {
                showCrumbBar();
                return true;
            }
            break;
        default:
            break;
        }
    }

    return false;
}

TabBar *TitleBarWidget::createTabBar(const quint64 windowId)
{
    auto tabBar = TabBarManager::instance()->createTabBar(windowId, this);
    if (!tabBar)
        return nullptr;

    connect(tabBar, &TabBar::newTabCreated, this, &TitleBarWidget::onTabCreated);
    connect(tabBar, &TabBar::tabRemoved, this, &TitleBarWidget::onTabRemoved);
    connect(tabBar, &TabBar::tabMoved, this, &TitleBarWidget::onTabMoved);
    return tabBar;
}

void TitleBarWidget::onAddressBarJump()
{
    const QString &currentDir = QDir::currentPath();
    if (dfmbase::FileUtils::isLocalFile(titlebarUrl))
        QDir::setCurrent(titlebarUrl.toLocalFile());
    QDir::setCurrent(currentDir);
}

void TitleBarWidget::onTabCreated()
{
    curNavWidget->addHistroyStack();
}

void TitleBarWidget::onTabRemoved(int index)
{
    curNavWidget->removeNavStackAt(index);
}

void TitleBarWidget::onTabMoved(int from, int to)
{
    curNavWidget->moveNavStacks(from, to);
}

void TitleBarWidget::resizeEvent(QResizeEvent *event)
{
    AbstractFrame::resizeEvent(event);
    
    int totalWidth = width();
    
    // Calculate the total width of other fixed-width components
    int fixedWidth = curNavWidget->width() + optionButtonBox->width() + kSpacing * 5;
    
    // Get the width of the currently visible bar (crumbBar or addressBar)
    int currentBarWidth = crumbBar->isVisible() ? crumbBar->width() : addressBar->width();
    
    if (totalWidth >= kCriticalWidth) {
        // When total width is greater than or equal to the critical point, prioritize setting searchEditWidget to maximum width
        searchEditWidget->setFixedWidth(kSearchEditMaxWidth);
        
        // Calculate remaining width and allocate to crumbBar or addressBar
        int remainingWidth = totalWidth - fixedWidth - kSearchEditMaxWidth;
        if (crumbBar->isVisible()) {
            crumbBar->setFixedWidth(remainingWidth);
        } else if (addressBar->isVisible()) {
            addressBar->setFixedWidth(remainingWidth);
        }
    } else {
        // When total width is less than the critical point, execute the previous algorithm
        int idealSearchWidth = totalWidth - fixedWidth - currentBarWidth;
        
        // Adjust the width of searchEditWidget
        int searchEditWidth = qBound(searchEditWidget->getMinimumWidth(), idealSearchWidth, kSearchEditMaxWidth);
        searchEditWidget->setFixedWidth(searchEditWidth);
        
        // Only adjust the width of crumbBar or addressBar when searchEditWidget reaches its boundary values
        int remainingWidth = totalWidth - fixedWidth - searchEditWidth;
        if (searchEditWidth == searchEditWidget->getMinimumWidth() || searchEditWidth == kSearchEditMaxWidth) {
            if (crumbBar->isVisible()) {
                crumbBar->setFixedWidth(remainingWidth);
            } else if (addressBar->isVisible()) {
                addressBar->setFixedWidth(remainingWidth);
            }
        }
    }
}
