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

#include <QHBoxLayout>

TitleBarWidget::TitleBarWidget(QFrame *parent)
    : QFrame(parent)
{
    initializeUi();
    initConnect();
}

void TitleBarWidget::initializeUi()
{
    // layout
    titleBarLayout = new QHBoxLayout(this);
    titleBarLayout->setMargin(0);
    titleBarLayout->setSpacing(0);

    // nav
    navWidget = new NavWidget;
    titleBarLayout->addSpacing(14);   // 导航栏左侧间隔14
    titleBarLayout->addWidget(navWidget, 0, Qt::AlignLeft);

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
    searchButton = new QToolButton;
    searchButton->setFixedSize({ 36, 36 });
    searchButton->setFocusPolicy(Qt::NoFocus);
    searchButton->setIcon(QIcon::fromTheme("search"));
    searchButton->setIconSize({ 16, 16 });
    titleBarLayout->addWidget(searchButton);

    // search filter button
    searchFilterButton = new QToolButton;
    searchFilterButton->setFixedSize({ 36, 36 });
    searchFilterButton->setFocusPolicy(Qt::NoFocus);
    searchFilterButton->setIcon(QIcon::fromTheme("dfview_filter"));
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
    QObject::connect(searchButton, &QToolButton::clicked,
                     this, &TitleBarWidget::onSearchButtonClicked);
}

void TitleBarWidget::showAddrsssBar()
{
    crumbBar->hide();
    addressBar->show();
    addressBar->setFocus();
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

void TitleBarWidget::showSearchFilterButton()
{
    if (searchButton)
        searchButton->hide();
    if (searchFilterButton)
        searchFilterButton->show();
}

bool TitleBarWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this && event->type() == QEvent::Show) {
        activateWindow();
        return false;
    }

    if (watched == addressBar && event->type() == QEvent::Hide) {
        showCrumbBar();
        return true;
    }

    return false;
}

void TitleBarWidget::onSearchButtonClicked()
{
    showAddrsssBar();
    searchButton->hide();
}
