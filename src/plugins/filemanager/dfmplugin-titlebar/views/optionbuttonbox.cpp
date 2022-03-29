/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "private/optionbuttonbox_p.h"
#include "views/optionbuttonbox.h"
#include "events/titlebareventcaller.h"
#include "utils/optionbuttonmanager.h"

#include "services/filemanager/workspace/workspaceservice.h"

#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"

#include <QDebug>

DPTITLEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

OptionButtonBoxPrivate::OptionButtonBoxPrivate(OptionButtonBox *parent)
    : QObject(parent), q(parent)
{
}

void OptionButtonBoxPrivate::setViewMode(ViewMode mode)
{
    if (currentMode == mode) {
        qDebug() << "The current mode already : " << int(mode);
        return;
    }

    currentMode = mode;
    // send viewchanged event
    // Note: `workspace` should save view mode to config file
    TitleBarEventCaller::sendViewMode(q, currentMode);
}

void OptionButtonBoxPrivate::loadViewMode(const QUrl &url)
{
    int defaultViewMode = static_cast<int>(WorkspaceService::service()->getDefaultViewMode(url.scheme()));
    auto viewMode = static_cast<ViewMode>(Application::appObtuselySetting()->value("FileViewState", url).toMap().value("viewMode", defaultViewMode).toInt());

    switchMode(viewMode);
}

void OptionButtonBoxPrivate::switchMode(ViewMode mode)
{
    currentMode = mode;
    switch (mode) {
    case ViewMode::kIconMode:
        iconViewButton->setChecked(true);
        break;
    case ViewMode::kListMode:
        listViewButton->setChecked(true);
        break;
    default:
        break;
    }
}

void OptionButtonBoxPrivate::onViewModeChanged(int mode)
{
    auto viewMode = static_cast<ViewMode>(mode);
    switchMode(viewMode);
}

QToolButton *OptionButtonBox::detailButton() const
{
    return d->detailButton;
}

void OptionButtonBox::setDetailButton(QToolButton *detailButton)
{
    if (!detailButton) return;

    if (!d->hBoxLayout->replaceWidget(detailButton, detailButton)->isEmpty()) {

        if (d->detailButton) {
            delete d->detailButton;
            d->detailButton = nullptr;
        }

        if (!d->detailButton)
            d->detailButton = detailButton;

        if (d->listViewButton->icon().isNull())
            d->detailButton->setIcon(QIcon::fromTheme("dfrightview_detail"));

        d->detailButton->setCheckable(true);
        d->detailButton->setFocusPolicy(Qt::NoFocus);
        d->detailButton->setIconSize({ 16, 16 });
        d->detailButton->setFixedSize(36, 36);
    }
}

void OptionButtonBox::onUrlChanged(const QUrl &url)
{
    d->loadViewMode(url);
    if (OptionButtonManager::instance()->hasVsibleState(url.scheme())) {
        auto state = OptionButtonManager::instance()->optBtnVisibleState(url.scheme());
        if (state & OptionButtonManager::kHideListViewBtn)
            d->listViewButton->setHidden(true);

        if (state & OptionButtonManager::kHideIconViewBtn)
            d->iconViewButton->setHidden(true);

        if (state & OptionButtonManager::kHideDetailSpaceBtn) {
            d->detailButton->setHidden(true);
            if (d->detailButton->isChecked())
                d->detailButton->click();
        }
    } else {
        d->listViewButton->setHidden(false);
        d->iconViewButton->setHidden(false);
        d->detailButton->setHidden(false);
    }
}

void OptionButtonBox::initializeUi()
{
    d->buttonGroup = new QButtonGroup(this);

    d->iconViewButton = new QToolButton;
    d->iconViewButton->setCheckable(true);
    d->iconViewButton->setChecked(true);
    d->iconViewButton->setIcon(QIcon::fromTheme("dfm_viewlist_icons"));
    d->iconViewButton->setIconSize({ 16, 16 });
    d->iconViewButton->setFixedSize(36, 36);

    d->listViewButton = new QToolButton;
    d->listViewButton->setCheckable(true);
    d->listViewButton->setIcon(QIcon::fromTheme("dfm_viewlist_details"));
    d->listViewButton->setIconSize({ 16, 16 });
    d->listViewButton->setFixedSize(36, 36);

    d->buttonGroup->addButton(d->iconViewButton);
    d->buttonGroup->addButton(d->listViewButton);

    d->detailButton = new QToolButton;
    d->detailButton->setCheckable(true);
    d->detailButton->setFocusPolicy(Qt::NoFocus);
    d->detailButton->setIcon(QIcon::fromTheme("dfm_rightview_detail"));
    d->detailButton->setIconSize({ 16, 16 });
    d->detailButton->setFixedSize(36, 36);
    d->detailButton->show();

    d->hBoxLayout = new QHBoxLayout;
    d->hBoxLayout->addSpacing(15);
    d->hBoxLayout->addWidget(d->iconViewButton);
    d->hBoxLayout->addWidget(d->listViewButton);
    d->hBoxLayout->addWidget(d->detailButton);
    d->hBoxLayout->setContentsMargins(0, 0, 4, 0);
    d->hBoxLayout->setSpacing(18);

    setLayout(d->hBoxLayout);
}

void OptionButtonBox::initConnect()
{
    connect(d->iconViewButton, &QToolButton::clicked, this, [this]() {
        d->setViewMode(ViewMode::kIconMode);
    });

    connect(d->listViewButton, &QToolButton::clicked, this, [this]() {
        d->setViewMode(ViewMode::kListMode);
    });

    connect(d->detailButton, &QToolButton::clicked, this, [this](bool checked) {
        TitleBarEventCaller::sendDetailViewState(this, checked);
    });

    connect(Application::instance(), &Application::viewModeChanged, d, &OptionButtonBoxPrivate::onViewModeChanged);
}

QToolButton *OptionButtonBox::listViewButton() const
{
    return d->listViewButton;
}

void OptionButtonBox::setListViewButton(QToolButton *listViewButton)
{
    if (!listViewButton) return;

    if (!d->hBoxLayout->replaceWidget(listViewButton, listViewButton)->isEmpty()) {

        if (d->listViewButton) {
            delete d->listViewButton;
            d->listViewButton = nullptr;
        }

        if (!listViewButton)
            d->listViewButton = listViewButton;

        if (d->listViewButton->icon().isNull())
            d->listViewButton->setIcon(QIcon::fromTheme("dfviewlist_details"));

        d->listViewButton->setCheckable(true);
        d->listViewButton->setFocusPolicy(Qt::NoFocus);
        d->listViewButton->setIconSize({ 16, 16 });
        d->listViewButton->setFixedSize(36, 36);
    }
}

OptionButtonBox::OptionButtonBox(QWidget *parent)
    : QWidget(parent), d(new OptionButtonBoxPrivate(this))
{
    initializeUi();
    initConnect();
}

QToolButton *OptionButtonBox::iconViewButton() const
{
    return d->iconViewButton;
}

void OptionButtonBox::setIconViewButton(QToolButton *iconViewButton)
{
    if (!iconViewButton) return;

    if (!d->hBoxLayout->replaceWidget(iconViewButton, iconViewButton)->isEmpty()) {

        if (d->iconViewButton) {
            delete d->iconViewButton;
            d->iconViewButton = nullptr;
        }

        if (!iconViewButton)
            d->iconViewButton = iconViewButton;

        if (d->iconViewButton->icon().isNull())
            d->iconViewButton->setIcon(QIcon::fromTheme("dfviewlist_icons"));

        d->iconViewButton->setCheckable(true);
        d->iconViewButton->setFocusPolicy(Qt::NoFocus);
        d->iconViewButton->setIconSize({ 16, 16 });
        d->iconViewButton->setFixedSize(36, 36);
    }
}
