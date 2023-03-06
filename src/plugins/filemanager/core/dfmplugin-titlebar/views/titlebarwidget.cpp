// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebarwidget.h"
#include "events/titlebareventcaller.h"
#include "utils/crumbinterface.h"
#include "utils/crumbmanager.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"
#include "dfm-base/utils/fileutils.h"

#include <dfm-framework/event/event.h>

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

void TitleBarWidget::startSpinner()
{
    addressBar->startSpinner();
}

void TitleBarWidget::stopSpinner()
{
    addressBar->stopSpinner();
}

void TitleBarWidget::handleHotkeyCtrlF()
{
    onSearchButtonClicked();
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
}

void TitleBarWidget::initializeUi()
{
    setFocusPolicy(Qt::NoFocus);

    // nav
    curNavWidget = new NavWidget;

    // address
    addressBar = new AddressBar;
    addressBar->installEventFilter(this);

    // crumb
    crumbBar = new CrumbBar;
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(crumbBar), AcName::kAcComputerTitleBarAddress);

    // search button
    searchButton = new DPushButton;
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(searchButton), AcName::kAcComputerTitleBarSearchBtn);
    searchButton->setFocusPolicy(Qt::NoFocus);
    searchButton->setFlat(true);
    searchButton->setIcon(QIcon::fromTheme("search"));

    // option button
    optionButtonBox = new OptionButtonBox;
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(optionButtonBox), AcName::kAcComputerTitleBarOptionBtnBox);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    showCrumbBar();

    initUiForSizeMode();
}

void TitleBarWidget::initConnect()
{
    connect(searchButton, &DPushButton::clicked, this, &TitleBarWidget::onSearchButtonClicked);
    connect(this, &TitleBarWidget::currentUrlChanged, optionButtonBox, &OptionButtonBox::onUrlChanged);
    connect(this, &TitleBarWidget::currentUrlChanged, crumbBar, &CrumbBar::onUrlChanged);
    connect(this, &TitleBarWidget::currentUrlChanged, curNavWidget, &NavWidget::onUrlChanged);
    connect(crumbBar, &CrumbBar::showAddressBarText, addressBar, [this](const QString &text) {
        crumbBar->hide();
        addressBar->show();
        addressBar->setFocus();
        addressBar->setText(text);
        searchBarActivated();
    });
    connect(crumbBar, &CrumbBar::hideAddressBar, this, [this](bool cd) {
        addressBar->hide();
        crumbBar->show();
        searchBarDeactivated();
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
    searchButton->setFixedSize(DSizeModeHelper::element(QSize(24, 24), QSize(36, 36)));
    QSize iconSize = searchButtonSwitchState ? QSize(16, 16) : QSize(30, 30);
    searchButton->setIconSize(DSizeModeHelper::element(QSize(16, 16), iconSize));

    if (titleBarLayout) {
        delete titleBarLayout;
        titleBarLayout = nullptr;
    }
    titleBarLayout = new QHBoxLayout(this);
    titleBarLayout->setMargin(0);
    titleBarLayout->setSpacing(0);
    titleBarLayout->addSpacing(DSizeModeHelper::element(10, 14));
    titleBarLayout->addWidget(curNavWidget, 0, Qt::AlignLeft);
    titleBarLayout->addSpacing(DSizeModeHelper::element(10, 4));
    titleBarLayout->addWidget(addressBar);
    titleBarLayout->addWidget(crumbBar);
    titleBarLayout->addWidget(searchButton);
    titleBarLayout->addSpacing(DSizeModeHelper::element(10, 4));
    titleBarLayout->addWidget(optionButtonBox, 0, Qt::AlignRight);
    setLayout(titleBarLayout);
#else
    crumbBar->setFixedHeight(36);
    addressBar->setFixedHeight(36);
    searchButton->setFixedSize(QSize(36, 36));
    QSize iconSize = searchButtonSwitchState ? QSize(16, 16) : QSize(30, 30);
    searchButton->setIconSize(iconSize);

    if (titleBarLayout) {
        delete titleBarLayout;
        titleBarLayout = nullptr;
    }
    titleBarLayout = new QHBoxLayout(this);
    titleBarLayout->setMargin(0);
    titleBarLayout->setSpacing(0);
    titleBarLayout->addSpacing(14);
    titleBarLayout->addWidget(curNavWidget, 0, Qt::AlignLeft);
    titleBarLayout->addSpacing(4);
    titleBarLayout->addWidget(addressBar);
    titleBarLayout->addWidget(crumbBar);
    titleBarLayout->addWidget(searchButton);
    titleBarLayout->addSpacing(4);
    titleBarLayout->addWidget(optionButtonBox, 0, Qt::AlignRight);
    setLayout(titleBarLayout);
#endif
}

void TitleBarWidget::showAddrsssBar(const QUrl &url)
{
    crumbBar->hide();
    addressBar->show();
    addressBar->setFocus();
    addressBar->setCurrentUrl(url);
    searchBarActivated();
}

void TitleBarWidget::showCrumbBar()
{
    showSearchButton();

    if (crumbBar)
        crumbBar->show();

    if (addressBar) {
        addressBar->clear();
        addressBar->hide();
    }
    setFocus();
}

void TitleBarWidget::showSearchButton()
{
    if (searchButton)
        searchButton->show();
}

void TitleBarWidget::showSearchFilterButton(bool visible)
{
    if (searchButtonSwitchState)
        searchButton->setVisible(visible);
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

    if (watched == addressBar && event->type() == QEvent::Hide && !crumbBar->controller()->isKeepAddressBar()) {
        showCrumbBar();
        return true;
    }

    return false;
}

void TitleBarWidget::toggleSearchButtonState(bool switchBtn)
{
    if (switchBtn) {
        searchButton->setHidden(true);
        searchButton->setObjectName("filterButton");
        searchButton->setIcon(QIcon::fromTheme("dfm_view_filter"));
        searchButton->setIconSize({ 16, 16 });
        searchButton->style()->unpolish(searchButton);
        searchButton->style()->polish(searchButton);
        searchButton->setFlat(true);
        searchButton->setProperty("showFilterView", false);
        searchButtonSwitchState = true;
    } else {
        searchButton->setHidden(false);
        searchButton->style()->unpolish(searchButton);
        searchButton->style()->polish(searchButton);
        searchButton->setIcon(QIcon::fromTheme("search"));
#ifdef DTKWIDGET_CLASS_DSizeMode
        searchButton->setIconSize(DSizeModeHelper::element(QSize(16, 16), QSize(30, 30)));
#else
        searchButton->setIconSize(QSize(30, 30));
#endif
        searchButton->setDown(false);
        searchButtonSwitchState = false;
        // TODO(zhangs): workspace->toggleAdvanceSearchBar(false);
    }
}

void TitleBarWidget::onSearchButtonClicked()
{
    if (!searchButtonSwitchState) {
        showAddrsssBar(QUrl());
    } else {
        bool oldState = searchButton->property("showFilterView").toBool();
        searchButton->setDown(!oldState);
        searchButton->setProperty("showFilterView", !oldState);
        TitleBarEventCaller::sendShowFilterView(this, !oldState);
    }
}

void TitleBarWidget::onAddressBarJump()
{
    const QString &currentDir = QDir::currentPath();
    if (dfmbase::FileUtils::isLocalFile(titlebarUrl))
        QDir::setCurrent(titlebarUrl.toLocalFile());
    QDir::setCurrent(currentDir);
}

void TitleBarWidget::searchBarActivated()
{
    toggleSearchButtonState(true);
}

void TitleBarWidget::searchBarDeactivated()
{
    toggleSearchButtonState(false);
}
