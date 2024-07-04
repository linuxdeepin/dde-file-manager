// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebarwidget.h"
#include "events/titlebareventcaller.h"
#include "utils/crumbinterface.h"
#include "utils/crumbmanager.h"

#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

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
    if (searchButtonSwitchState)
        searchButton->setChecked(!searchButton->isChecked());

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
    // Press ctrl+3
    if (mode == 2 && DConfigManager::instance()->value(kViewDConfName, kTreeViewEnable, true).toBool())
        TitleBarEventCaller::sendViewMode(this, ViewMode::kTreeMode);
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
    // search button
    searchButton = new DToolButton;
    searchButton->setIcon(QIcon::fromTheme("dfm_search_button"));
    searchButton->setFixedSize(36, 36);
    searchButton->setFocusPolicy(Qt::NoFocus);
    searchButton->setToolTip(tr("search"));
    // option button
    optionButtonBox = new OptionButtonBox;
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(crumbBar), AcName::kAcComputerTitleBarAddress);
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(searchButton), AcName::kAcComputerTitleBarSearchBtn);
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(optionButtonBox), AcName::kAcComputerTitleBarOptionBtnBox);
#endif

    titleBarLayout = new QHBoxLayout(this);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    titleBarLayout->setMargin(0);
#else
    titleBarLayout->setContentsMargins(0, 0, 0, 0);
#endif
    titleBarLayout->setSpacing(0);
    titleBarLayout->addSpacing(10);
    titleBarLayout->addWidget(curNavWidget, 0, Qt::AlignLeft);
    titleBarLayout->addSpacing(10);
    titleBarLayout->addWidget(addressBar);
    titleBarLayout->addWidget(crumbBar);
    titleBarLayout->addSpacing(10);
    titleBarLayout->addWidget(searchButton);
    titleBarLayout->addSpacing(5);
    titleBarLayout->addWidget(optionButtonBox, 0, Qt::AlignRight);
    setLayout(titleBarLayout);

    initUiForSizeMode();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    showCrumbBar();
}

void TitleBarWidget::initConnect()
{
    connect(searchButton, &DToolButton::clicked, this, &TitleBarWidget::onSearchButtonClicked);
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
        addressBar->stopSpinner();
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
#else
    crumbBar->setFixedHeight(36);
    addressBar->setFixedHeight(36);
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
        searchBarDeactivated();
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

    if (watched == addressBar) {
        switch (event->type()) {
        case QEvent::FocusOut: {
            // keep the `addressBar` displayed when click `filterButton`
            bool posContains = searchButton->geometry().contains(mapFromGlobal(QCursor::pos()));
            bool isChecked = searchButton->isChecked();
            if (posContains || isChecked) {
                addressBar->showOnFocusLostOnce();
                return false;
            }
        } break;
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

void TitleBarWidget::toggleSearchButtonState(bool switchBtn)
{
    if (switchBtn) {
        searchButton->setObjectName("filterButton");
        searchButton->setIcon(QIcon::fromTheme("dfm_view_filter"));
        searchButton->setCheckable(true);
        searchButton->setToolTip(tr("advanced search"));
        searchButtonSwitchState = true;
    } else {
        if (searchButton->isChecked())
            TitleBarEventCaller::sendShowFilterView(this, false);
        searchButton->setIcon(QIcon::fromTheme("dfm_search_button"));
        searchButton->setCheckable(false);
        searchButton->setToolTip(tr("search"));
        searchButtonSwitchState = false;
    }
}

void TitleBarWidget::onSearchButtonClicked()
{
    if (!searchButtonSwitchState) {
        showAddrsssBar(QUrl());
    } else {
        TitleBarEventCaller::sendShowFilterView(this, searchButton->isChecked());
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
