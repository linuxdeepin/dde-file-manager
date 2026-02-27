// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebarwidget.h"
#include "tabbar.h"
#include "searcheditwidget.h"
#include "events/titlebareventcaller.h"
#include "utils/crumbinterface.h"
#include "utils/crumbmanager.h"
#include "utils/titlebarhelper.h"
#include "utils/optionbuttonmanager.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/dialogmanager.h>
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
inline constexpr char kIsCustomTab[] { "isCustomTab" };

TitleBarWidget::TitleBarWidget(QFrame *parent)
    : AbstractFrame(parent)
{
    initializeUi();
    initConnect();
}

void TitleBarWidget::setCurrentUrl(const QUrl &url)
{
    // Check if this is a special pinned tab URL
    if (url.scheme() == TabDef::kTabScheme) {
        // Extract pinnedId from query
        QUrlQuery query(url.query());
        QString pinnedId = query.queryItemValue(TabDef::kPinnedId);

        if (!pinnedId.isEmpty()) {
            fmInfo() << "Detected pinned tab URL, storing pinnedId:" << pinnedId;
            pendingPinnedTabId = pinnedId;
            // Don't set titlebarUrl or emit signal for pinned scheme
            return;
        }
    }

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

CrumbBar *TitleBarWidget::titleCrumbBar() const
{
    return crumbBar;
}

void TitleBarWidget::openNewTab(const QUrl &url)
{
    tabBar()->appendTab();

    if (url.isEmpty())
        TitleBarEventCaller::sendCd(this, StandardPaths::location(StandardPaths::kHomePath));

    TitleBarEventCaller::sendCd(this, url);
}

void TitleBarWidget::openCustomFixedTabs()
{
    const auto &itemList = DConfigManager::instance()->value(kViewDConfName, kCunstomFixedTabs, {}).toStringList();
    for (const auto &item : itemList) {
        int index = tabBar()->appendInactiveTab(item);
        tabBar()->setTabUserData(index, kIsCustomTab, true);
    }
}

void TitleBarWidget::openPinnedTabs()
{
    QVariantList validPinnedTabs;
    const auto &pinnedTabs = DConfigManager::instance()->value(kViewDConfName, kPinnedTabs, QVariantList()).toList();
    fmInfo() << "Loading" << pinnedTabs.size() << "pinned tabs";

    for (int i = pinnedTabs.size() - 1; i >= 0; --i) {
        auto tabData = pinnedTabs[i].toMap();
        QString pinnedId = tabData[TabDef::kPinnedId].toString();
        QUrl url = QUrl(tabData[TabDef::kTabUrl].toString());
        if (!url.isValid() || pinnedId.isEmpty()) {
            fmWarning() << "Invalid pinned tab data, skipping:" << tabData;
            continue;
        }

        if (!(UrlRoute::isVirtual(url) && UrlRoute::isRootUrl(url))) {
            auto info = InfoFactory::create<FileInfo>(url);
            if (!info || !info->exists()) {
                fmWarning() << "The file is not exists, skipping:" << tabData;
                continue;
            }
        }

        validPinnedTabs << pinnedTabs[i];
        int index = tabBar()->insertInactiveTab(0, url, true);
        tabBar()->setTabUserData(index, TabDef::kPinnedId, pinnedId);
        fmInfo() << "Restored pinned tab:" << url << "at index:" << index << "with pinnedId:" << pinnedId;
    }

    if (validPinnedTabs.size() != pinnedTabs.size())
        DConfigManager::instance()->setValue(kViewDConfName, kPinnedTabs, validPinnedTabs);

    // Activate pending pinned tab if exists
    if (!pendingPinnedTabId.isEmpty()) {
        fmInfo() << "Activating pending pinned tab:" << pendingPinnedTabId;
        activatePinnedTab(pendingPinnedTabId);
        pendingPinnedTabId.clear();
    }
}

void TitleBarWidget::activatePinnedTab(const QString &pinnedId)
{
    if (pinnedId.isEmpty())
        return;

    // Find tab with matching pinnedId
    for (int i = 0; i < tabBar()->count(); ++i) {
        if (!tabBar()->isPinned(i))
            continue;

        QString tabPinnedId = tabBar()->tabUserData(i, TabDef::kPinnedId).toString();
        if (tabPinnedId == pinnedId) {
            fmInfo() << "Found and activating pinned tab at index" << i << "with pinnedId:" << pinnedId;
            tabBar()->setCurrentIndex(i);
            Q_EMIT tabBar()->currentChanged(i);
            return;
        }
    }

    fmWarning() << "Could not find pinned tab with pinnedId:" << pinnedId;
}

void TitleBarWidget::handleSplitterAnimation(const QVariant &position)
{
    int newWidth = qMax(0, 95 - position.toInt());
    if (newWidth == placeholder->width())
        return;

    placeholder->setFixedWidth(newWidth);
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
    const QString scheme = currentUrl().scheme();

    // Check if view mode is visible for current scheme
    if (!TitleBarHelper::isViewModeVisibleForScheme(mode, scheme)) {
        fmDebug() << "View mode" << mode << "is not supported for scheme:" << scheme;
        return;
    }

    // Tree view requires additional global enable check
    if (mode == 2 && !TitleBarHelper::isTreeViewGloballyEnabled()) {
        fmDebug() << "Tree view is globally disabled";
        return;
    }

    // Send view mode change event
    ViewMode viewMode;
    switch (mode) {
    case 0:
        viewMode = ViewMode::kIconMode;
        break;
    case 1:
        viewMode = ViewMode::kListMode;
        break;
    case 2:
        viewMode = ViewMode::kTreeMode;
        break;
    default:
        return;
    }

    TitleBarEventCaller::sendViewMode(this, viewMode);
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

    tabBar()->removeTab(tabBar()->currentIndex());
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

void TitleBarWidget::handleCreateTabList(const QList<QUrl> &urlList)
{
    for (const auto &url : urlList) {
        const FileInfoPointer &fileInfoPtr = InfoFactory::create<FileInfo>(url);
        if (fileInfoPtr && fileInfoPtr->isAttributes(OptInfoType::kIsDir)) {
            openNewTab(url);
        }
    }
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
    topBarCustomLayout->addSpacing(10);

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
    connect(searchEditWidget, &SearchEditWidget::searchStop, this, [this]() {
        TitleBarEventCaller::sendStopSearch(this);
    });

    connect(this, &TitleBarWidget::currentUrlChanged, searchEditWidget, &SearchEditWidget::onUrlChanged);

    connect(bottomBar, &TabBar::newTabCreated, this, &TitleBarWidget::onTabCreated);
    connect(bottomBar, &TabBar::requestCreateView, this, &TitleBarWidget::handleCreateView);
    connect(bottomBar, &TabBar::tabAboutToRemove, this, &TitleBarWidget::onTabAboutToRemove);
    connect(bottomBar, &TabBar::tabMoved, this, &TitleBarWidget::onTabMoved);
    connect(bottomBar, &TabBar::currentTabChanged, this, &TitleBarWidget::onTabCurrentChanged);
    connect(bottomBar, &TabBar::tabCloseRequested, this, &TitleBarWidget::onTabCloseRequested);
    connect(bottomBar, &TabBar::tabAddRequested, this, &TitleBarWidget::onTabAddButtonClicked);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        updateUiForSizeMode();
    });
#endif
}

void TitleBarWidget::updateUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    crumbBar->setFixedHeight(DSizeModeHelper::element(24, 30));
    addressBar->setFixedHeight(DSizeModeHelper::element(24, 30));
#else
    crumbBar->setFixedHeight(30);
    addressBar->setFixedHeight(30);
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

    return false;
}

void TitleBarWidget::saveTitleBarState(const QString &uniqueId)
{
    TitleBarState state;
    state.advancedSearchVisible = searchEditWidget->isAdvancedButtonVisible();
    state.advancedSearchChecked = searchEditWidget->isAdvancedButtonChecked();
    state.searchText = searchEditWidget->text();
    state.viewMode = optionButtonBox->viewMode();

    titleBarStateMap[uniqueId] = state;
}

void TitleBarWidget::restoreTitleBarState(const QString &uniqueId)
{
    if (titleBarStateMap.contains(uniqueId)) {
        const TitleBarState &state = titleBarStateMap[uniqueId];
        searchEditWidget->setAdvancedButtonVisible(state.advancedSearchVisible);
        searchEditWidget->setAdvancedButtonChecked(state.advancedSearchChecked);
        if (!state.searchText.isEmpty())
            searchEditWidget->setText(state.searchText);
        optionButtonBox->setViewMode(static_cast<int>(state.viewMode));
    }
}

bool TitleBarWidget::checkCustomFixedTab(int index)
{
    if (!tabBar()->tabUserData(index, kIsCustomTab).toBool())
        return true;

    const auto &url = tabBar()->tabUrl(index);
    auto info = InfoFactory::create<FileInfo>(url);
    if (info && info->exists())
        return true;

    // Some URLs cannot be checked by info, such as `computer:///`.
    if (UrlRoute::isVirtual(url) && UrlRoute::isRootUrl(url))
        return true;

    int ret = DialogManagerInstance->showMessageDialog(tr("Directory not found"),
                                                       tr("Directory not found. Remove it?"),
                                                       { tr("Cancel", "button"), tr("Remove", "button") });
    if (ret == 1) {
        auto urlList = DConfigManager::instance()->value(kViewDConfName, kCunstomFixedTabs, {}).toStringList();
        urlList.removeOne(url.toString());
        DConfigManager::instance()->setValue(kViewDConfName, kCunstomFixedTabs, urlList);
        return false;
    }
    return true;
}

void TitleBarWidget::onAddressBarJump()
{
    const QString &currentDir = QDir::currentPath();
    if (titlebarUrl.isLocalFile())
        QDir::setCurrent(titlebarUrl.toLocalFile());
    QDir::setCurrent(currentDir);
}

void TitleBarWidget::handleCreateView(const QString &uniqueId)
{
    TitleBarEventCaller::sendTabCreated(this, uniqueId);
}

void TitleBarWidget::onTabCreated()
{
    curNavWidget->addHistroyStack();
}

void TitleBarWidget::onTabAboutToRemove(int oldIndex, int nextIndex)
{
    TitleBarEventCaller::sendTabRemoved(this, tabBar()->tabUniqueId(oldIndex), tabBar()->tabUniqueId(nextIndex));
    curNavWidget->removeNavStackAt(oldIndex);
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

void TitleBarWidget::showEvent(QShowEvent *event)
{
    AbstractFrame::showEvent(event);

    // Sync DTitlebar's max button state when first shown
    // This handles the case where the window was maximized before the titlebar was installed
    static bool firstShow = true;
    if (firstShow && topBar) {
        firstShow = false;
        // DTitlebar installs its event filter in setVisible(true), which happens during this showEvent
        // We need to wait for the event filter to be fully installed before syncing the window state
        QTimer::singleShot(0, this, [this]() {
            auto window = qobject_cast<FileManagerWindow *>(this->window());
            if (window && window->isMaximized()) {
                // Send a WindowStateChange event to trigger DTitlebar's handleParentWindowStateChange()
                QEvent event(QEvent::WindowStateChange);
                QCoreApplication::sendEvent(window, &event);
            }
        });
    }
}

void TitleBarWidget::onTabCurrentChanged(int oldIndex, int newIndex)
{
    if (tabBar()->isTabValid(newIndex)) {
        if (!checkCustomFixedTab(newIndex)) {
            tabBar()->removeTab(newIndex, oldIndex);
            return;
        }

        if (oldIndex >= 0 && oldIndex < tabBar()->count()) {
            saveTitleBarState(tabBar()->tabUniqueId(oldIndex));
        }
        // switch tab must before change url! otherwise NavWidget can not work!
        curNavWidget->switchHistoryStack(newIndex);
        TitleBarEventCaller::sendTabChanged(this, tabBar()->tabUniqueId(newIndex));
        TitleBarEventCaller::sendChangeCurrentUrl(this, tabBar()->tabUrl(newIndex));
        restoreTitleBarState(tabBar()->tabUniqueId(newIndex));
    }
}

void TitleBarWidget::onTabCloseRequested(int index)
{
    tabBar()->removeTab(index);
}

void TitleBarWidget::onTabAddButtonClicked()
{
    QUrl url = Application::instance()->appUrlAttribute(Application::kUrlOfNewTab);
    auto tabUrl = tabBar()->tabUrl(tabBar()->currentIndex());
    if (!url.isValid() && tabUrl.isValid())
        url = tabUrl;

    openNewTab(url);
}

void TitleBarWidget::quitSearch()
{
    if (crumbBar && !UniversalUtils::urlEquals(crumbBar->lastUrl(), titlebarUrl))
        TitleBarEventCaller::sendCd(this, crumbBar->lastUrl());
}
