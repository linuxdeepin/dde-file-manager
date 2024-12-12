// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebarwidget.h"
#include "tabbar.h"
#include "searcheditwidget.h"
#include "events/titlebareventcaller.h"
#include "utils/crumbinterface.h"
#include "utils/crumbmanager.h"
#include "utils/titlebarhelper.h"
#include "views/tab.h"

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
#include <QResizeEvent>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

inline constexpr int kSpacing { 10 };

TitleBarWidget::TitleBarWidget(QFrame *parent)
    : AbstractFrame(parent)
{
    initializeUi();
    initConnect();
}

void TitleBarWidget::setCurrentUrl(const QUrl &url)
{
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

TabBar *TitleBarWidget::tabBar() const
{
    return bottomBar;
}

void TitleBarWidget::openNewTab(const QUrl &url)
{
    if (!tabBar()->tabAddable())
        return;

    tabBar()->createTab();

    if (url.isEmpty())
        TitleBarEventCaller::sendCd(this, StandardPaths::location(StandardPaths::kHomePath));

    TitleBarEventCaller::sendCd(this, url);
}

void TitleBarWidget::startSpinner()
{
    searchEditWidget->startSpinner();
}

void TitleBarWidget::stopSpinner()
{
    searchEditWidget->stopSpinner();
}

void TitleBarWidget::handleSplitterAnimation(const QVariant &position)
{
    if (position == splitterEndValue) {
        splitterStartValue = -1;
        splitterEndValue = -1;
        isSplitterAnimating = false;
    }
 
    int newWidth = qMax(0, 95 - position.toInt());
    if (newWidth == placeholder->width())
        return;

    placeholder->setFixedWidth(newWidth);
}

void TitleBarWidget::handleAboutToPlaySplitterAnim(int startValue, int endValue)
{
    isSplitterAnimating = true;
    splitterStartValue = startValue;
    splitterEndValue = endValue;
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
    if (tabBar()->count() == 1) {
        auto winId = TitleBarHelper::windowId(this);
        auto window = FMWindowsIns.findWindowById(winId);
        if (window)
            window->close();

        return;
    }

    tabBar()->removeTab(tabBar()->getCurrentIndex());
}

void TitleBarWidget::handleHotketNextTab()
{
    tabBar()->activateNextTab();
}

void TitleBarWidget::handleHotketPreviousTab()
{
    tabBar()->activatePreviousTab();
}

void TitleBarWidget::handleHotketCreateNewTab()
{
    // If a directory is selected, open NewTab through the URL of the selected directory
    auto winId = TitleBarHelper::windowId(this);
    QList<QUrl> urls = dpfSlotChannel->push("dfmplugin_workspace", "slot_View_GetSelectedUrls", winId).value<QList<QUrl>>();
    if (urls.count() == 1) {
        const FileInfoPointer &fileInfoPtr = InfoFactory::create<FileInfo>(urls.at(0));
        if (fileInfoPtr && fileInfoPtr->isAttributes(OptInfoType::kIsDir)) {
            openNewTab(urls.at(0));
            return;
        }
    }

    openNewTab(currentUrl());
}

void TitleBarWidget::handleHotketActivateTab(const int index)
{
    tabBar()->setCurrentIndex(index);
}

void TitleBarWidget::initializeUi()
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::NoFocus);

    // titlebar
    topBar = new DTitlebar;
    topBar->setFixedHeight(DSizeModeHelper::element(24, 40));
    auto topBarLayout = topBar->layout();
    if (topBarLayout) {
        topBarLayout->setContentsMargins(0, 0, 0, 0);
        topBarLayout->setSpacing(0);
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

    // tabbar
    bottomBar = new TabBar;
    bottomBar->installEventFilter(this);
    topBarCustomLayout->addWidget(bottomBar, 1);

    // nav
    curNavWidget = new NavWidget;
    curNavWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // address
    addressBar = new AddressBar;
    addressBar->installEventFilter(this);

    // crumb
    crumbBar = new CrumbBar(this);

    // search widget
    searchEditWidget = new SearchEditWidget(this);
    searchEditWidget->setFixedHeight(30);

    // option button
    optionButtonBox = new OptionButtonBox(this);
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
    bottomBarLayout->addWidget(crumbBar, 1);
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

    updateUiForSizeMode();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    showCrumbBar();
}

int TitleBarWidget::calculateRemainingWidth() const
{
    int totalWidth = width();
    int remainingWidth = totalWidth;

    // Subtract navigation button width
    remainingWidth -= curNavWidget->width();
    // Subtract address bar width
    if (addressBar->isVisible())
        remainingWidth -= addressBar->width();
    // Subtract option button width
    remainingWidth -= optionButtonBox->width();
    // Subtract search widget width
    remainingWidth -= searchEditWidget->width();
    // Subtract spacing
    remainingWidth -= kSpacing * 4;

    return remainingWidth;
}

void TitleBarWidget::initConnect()
{
    connect(this, &TitleBarWidget::currentUrlChanged, bottomBar, &TabBar::setCurrentUrl);
    connect(this, &TitleBarWidget::currentUrlChanged, optionButtonBox, &OptionButtonBox::onUrlChanged);
    connect(this, &TitleBarWidget::currentUrlChanged, crumbBar, &CrumbBar::onUrlChanged);
    connect(this, &TitleBarWidget::currentUrlChanged, curNavWidget, &NavWidget::onUrlChanged);
    connect(crumbBar, &CrumbBar::showAddressBarText, addressBar, [this](const QString &text) {
        crumbBar->hide();
        addressBar->show();
        addressBar->setFocus();
        addressBar->setText(text);
    });
    connect(crumbBar, &CrumbBar::hideAddressBar, this, [this] {
        addressBar->hide();
        crumbBar->show();
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

    connect(searchEditWidget, &SearchEditWidget::searchQuit, this, &TitleBarWidget::quitSearch);

    connect(this, &TitleBarWidget::currentUrlChanged, searchEditWidget, &SearchEditWidget::onUrlChanged);

    connect(bottomBar, &TabBar::newTabCreated, this, &TitleBarWidget::onTabCreated);
    connect(bottomBar, &TabBar::tabRemoved, this, &TitleBarWidget::onTabRemoved);
    connect(bottomBar, &TabBar::tabMoved, this, &TitleBarWidget::onTabMoved);
    connect(bottomBar, &TabBar::currentChanged, this, &TitleBarWidget::onTabCurrentChanged);
    connect(bottomBar, &TabBar::tabCloseRequested, this, &TitleBarWidget::onTabCloseRequested);
    connect(bottomBar, &TabBar::tabAddButtonClicked, this, &TitleBarWidget::onTabAddButtonClicked);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        updateUiForSizeMode();
    });
#endif
}

void TitleBarWidget::updateUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    crumbBar->setFixedHeight(DSizeModeHelper::element(24, 36));
    addressBar->setFixedHeight(DSizeModeHelper::element(24, 36));
#else
    crumbBar->setFixedHeight(36);
    addressBar->setFixedHeight(36);
#endif
    topBar->setFixedHeight(40);

    auto optionBtn = topBar->findChild<DWindowOptionButton *>("DTitlebarDWindowOptionButton");
    if (optionBtn)
        optionBtn->setFixedSize(40, 40);

    auto closeBtn = topBar->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
    if (closeBtn)
        closeBtn->setFixedSize(40, 40);

    auto minBtn = topBar->findChild<DWindowMinButton *>("DTitlebarDWindowMinButton");
    if (minBtn)
        minBtn->setFixedSize(40, 40);

    auto maxBtn = topBar->findChild<DWindowMaxButton *>("DTitlebarDWindowMaxButton");
    if (maxBtn)
        maxBtn->setFixedSize(40, 40);
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
            showCrumbBar();
            return true;
        default:
            break;
        }
    }

    // if the splitter is animating, do not process the resize event of tabbar
    // otherwise, the tabbar width will be changed twice(once by the resizeEvent and once by placeholder size changed)
    if (watched == bottomBar && event->type() == QEvent::Resize) {
        if (isSplitterAnimating)
            return true;
    }

    return false;
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
    optionButtonBox->updateOptionButtonBox(totalWidth);
    searchEditWidget->updateSearchEditWidget(totalWidth);
}

void TitleBarWidget::onTabCurrentChanged(int tabIndex)
{
    Tab *tab = tabBar()->tabAt(tabIndex);
    if (tab) {
        // switch tab must before change url! otherwise NavWidget can not work!
        curNavWidget->switchHistoryStack(tabIndex);
        TitleBarEventCaller::sendChangeCurrentUrl(this, tab->getCurrentUrl());
    }
}

void TitleBarWidget::onTabCloseRequested(int index, bool remainState)
{
    tabBar()->removeTab(index, remainState);
}

void TitleBarWidget::onTabAddButtonClicked()
{
    QUrl url = Application::instance()->appUrlAttribute(Application::kUrlOfNewTab);
    auto tab = tabBar()->currentTab();
    if (!url.isValid() && tab)
        url = tab->getCurrentUrl();

    openNewTab(url);
}

void TitleBarWidget::quitSearch()
{
    if (crumbBar && !UniversalUtils::urlEquals(crumbBar->lastUrl(), titlebarUrl))
        TitleBarEventCaller::sendCd(this, crumbBar->lastUrl());
}
