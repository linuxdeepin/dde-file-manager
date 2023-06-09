// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/optionbuttonbox_p.h"
#include "views/optionbuttonbox.h"
#include "events/titlebareventcaller.h"
#include "utils/optionbuttonmanager.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

#include <dfm-framework/event/event.h>

#include <DGuiApplicationHelper>
#include <dtkwidget_global.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#include <DSizeMode>
#endif

#include <QDebug>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

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
    auto defaultViewMode = static_cast<int>(TitleBarEventCaller::sendGetDefualtViewMode(url.scheme()));
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

DToolButton *OptionButtonBox::detailButton() const
{
    return d->detailButton;
}

void OptionButtonBox::setDetailButton(DToolButton *detailButton)
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
    }
}

void OptionButtonBox::setViewMode(int mode)
{
    d->switchMode(static_cast<ViewMode>(mode));
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

    d->iconViewButton = new DToolButton;
    d->iconViewButton->setCheckable(true);
    d->iconViewButton->setChecked(true);
    d->iconViewButton->setIcon(QIcon::fromTheme("dfm_viewlist_icons"));

    d->listViewButton = new DToolButton;
    d->listViewButton->setCheckable(true);
    d->listViewButton->setIcon(QIcon::fromTheme("dfm_viewlist_details"));

    d->buttonGroup->addButton(d->iconViewButton);
    d->buttonGroup->addButton(d->listViewButton);

    d->detailButton = new DToolButton;
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(d->detailButton), AcName::kAcComputerTitleBarDetailBtn);
#endif
    d->detailButton->setCheckable(true);
    d->detailButton->setFocusPolicy(Qt::NoFocus);
    d->detailButton->setIcon(QIcon::fromTheme("dfm_rightview_detail"));

    initUiForSizeMode();
}

void OptionButtonBox::initConnect()
{
    connect(d->iconViewButton, &DToolButton::clicked, this, [this]() {
        d->setViewMode(ViewMode::kIconMode);
    });

    connect(d->listViewButton, &DToolButton::clicked, this, [this]() {
        d->setViewMode(ViewMode::kListMode);
    });

    connect(d->detailButton, &DToolButton::clicked, this, [this](bool checked) {
        TitleBarEventCaller::sendDetailViewState(this, checked);
    });

    connect(Application::instance(), &Application::viewModeChanged, d, &OptionButtonBoxPrivate::onViewModeChanged);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this](){
        initUiForSizeMode();
    });
#endif
}

void OptionButtonBox::initUiForSizeMode()
{
    if (d->hBoxLayout) {
        delete d->hBoxLayout;
        d->hBoxLayout = nullptr;
    }
    d->hBoxLayout = new QHBoxLayout;
    d->hBoxLayout->addWidget(d->iconViewButton);
    d->hBoxLayout->addWidget(d->listViewButton);
    d->hBoxLayout->addWidget(d->detailButton);
#ifdef DTKWIDGET_CLASS_DSizeMode
    d->hBoxLayout->setSpacing(DSizeModeHelper::element(10, 18));
#else
    d->hBoxLayout->setSpacing(18);
#endif
    setLayout(d->hBoxLayout);
}

DToolButton *OptionButtonBox::listViewButton() const
{
    return d->listViewButton;
}

void OptionButtonBox::setListViewButton(DToolButton *listViewButton)
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
    }
}

OptionButtonBox::OptionButtonBox(QWidget *parent)
    : QWidget(parent), d(new OptionButtonBoxPrivate(this))
{
    initializeUi();
    initConnect();
}

DToolButton *OptionButtonBox::iconViewButton() const
{
    return d->iconViewButton;
}

void OptionButtonBox::setIconViewButton(DToolButton *iconViewButton)
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
    }
}
