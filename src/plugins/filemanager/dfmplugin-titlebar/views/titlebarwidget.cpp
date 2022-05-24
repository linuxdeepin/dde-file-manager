/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "titlebarwidget.h"

#include "events/titlebareventcaller.h"
#include "utils/crumbinterface.h"
#include "utils/crumbmanager.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"

#include <QHBoxLayout>
#include <QEvent>

DPTITLEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

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

void TitleBarWidget::initializeUi()
{
    setFocusPolicy(Qt::NoFocus);

    // layout
    titleBarLayout = new QHBoxLayout(this);
    titleBarLayout->setMargin(0);
    titleBarLayout->setSpacing(0);

    // nav
    curNavWidget = new NavWidget;
    titleBarLayout->addSpacing(14);   // 导航栏左侧间隔14
    titleBarLayout->addWidget(curNavWidget, 0, Qt::AlignLeft);

    // address
    addressBar = new AddressBar;
    addressBar->setFixedHeight(36);
    addressBar->installEventFilter(this);
    titleBarLayout->addSpacing(4);
    titleBarLayout->addWidget(addressBar);

    // crumb
    crumbBar = new CrumbBar;
    titleBarLayout->addWidget(crumbBar);

    // search button
    searchButton = new QPushButton;
    searchButton->setFixedSize({ 36, 36 });
    searchButton->setFocusPolicy(Qt::NoFocus);
    searchButton->setFlat(true);
    searchButton->setIcon(QIcon::fromTheme("search"));
    searchButton->setIconSize({ 16, 16 });
    titleBarLayout->addWidget(searchButton);

    // search filter button
    searchFilterButton = new QToolButton;
    searchFilterButton->setFixedSize({ 36, 36 });
    searchFilterButton->setFocusPolicy(Qt::NoFocus);
    searchFilterButton->setIcon(QIcon::fromTheme("dfm_view_filter"));
    searchFilterButton->setIconSize({ 16, 16 });
    titleBarLayout->addWidget(searchFilterButton);

    // option button
    optionButtonBox = new OptionButtonBox;
    titleBarLayout->addSpacing(4);
    titleBarLayout->addWidget(optionButtonBox, 0, Qt::AlignRight);

    setLayout(titleBarLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    showCrumbBar();
}

void TitleBarWidget::initConnect()
{
    connect(searchButton, &QToolButton::clicked, this, &TitleBarWidget::onSearchButtonClicked);
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
    if (searchFilterButton)
        searchFilterButton->hide();
}

void TitleBarWidget::showSearchFilterButton(bool visible)
{
    if (searchButtonSwitchState)
        searchButton->setVisible(visible);
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
    if (titlebarUrl.isLocalFile())
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
