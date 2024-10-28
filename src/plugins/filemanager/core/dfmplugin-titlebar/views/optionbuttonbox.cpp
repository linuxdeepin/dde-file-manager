// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/optionbuttonbox_p.h"
#include "views/optionbuttonbox.h"
#include "events/titlebareventcaller.h"
#include "utils/optionbuttonmanager.h"
#include "views/sortbybutton.h"
#include "views/viewoptionsbutton.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/event/event.h>

#include <DGuiApplicationHelper>
#include <dtkwidget_global.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
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
        fmDebug() << "The current mode already : " << int(mode);
        return;
    }

    currentMode = mode;
    // send viewchanged event
    // Note: `workspace` should save view mode to config file
    TitleBarEventCaller::sendViewMode(q, currentMode);
}

void OptionButtonBoxPrivate::loadViewMode(const QUrl &url)
{
    QUrl tmpUrl = url.adjusted(QUrl::RemoveQuery);
    auto defaultViewMode = static_cast<int>(TitleBarEventCaller::sendGetDefualtViewMode(tmpUrl.scheme()));
    auto viewMode = static_cast<ViewMode>(Application::appObtuselySetting()->value("FileViewState", tmpUrl).toMap().value("viewMode", defaultViewMode).toInt());
    if (viewMode == ViewMode::kTreeMode && !DConfigManager::instance()->value(kViewDConfName, kTreeViewEnable, true).toBool())
        viewMode = ViewMode::kListMode;

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
    case ViewMode::kTreeMode:
        treeViewButton->setChecked(true);
        break;
    default:
        break;
    }
    viewOptionsButton->switchMode(mode, currentUrl);
}

void OptionButtonBoxPrivate::onViewModeChanged(int mode)
{
    if (Application::appObtuselySetting()->value("FileViewState", currentUrl).toMap().contains("viewMode")) {
        loadViewMode(currentUrl);
    } else {
        auto viewMode = static_cast<ViewMode>(mode);
        switchMode(viewMode);
    }
}

ViewOptionsButton *OptionButtonBox::viewOptionsButton() const
{
    return d->viewOptionsButton;
}

void OptionButtonBox::setViewOptionsButton(ViewOptionsButton *button)
{
    if (!button) return;

    if (!d->hBoxLayout->replaceWidget(d->viewOptionsButton, button)->isEmpty()) {

        if (d->viewOptionsButton) {
            delete d->viewOptionsButton;
            d->viewOptionsButton = nullptr;
        }

        if (!d->viewOptionsButton)
            d->viewOptionsButton = button;

        if (d->viewOptionsButton->icon().isNull())
            d->viewOptionsButton->setIcon(QIcon::fromTheme("dfm_viewoption"));

        d->viewOptionsButton->setCheckable(false);
        d->viewOptionsButton->setFocusPolicy(Qt::NoFocus);
    }
}

void OptionButtonBox::setViewMode(int mode)
{
    d->switchMode(static_cast<ViewMode>(mode));
}

void OptionButtonBox::onUrlChanged(const QUrl &url)
{
    d->currentUrl = url;
    d->loadViewMode(url);
    if (OptionButtonManager::instance()->hasVsibleState(url.scheme())) {
        auto state = OptionButtonManager::instance()->optBtnVisibleState(url.scheme());
        d->listViewButton->setHidden(state & OptionButtonManager::kHideListViewBtn);
        d->iconViewButton->setHidden(state & OptionButtonManager::kHideIconViewBtn);
        if (d->treeViewButton)
            d->treeViewButton->setHidden(state & OptionButtonManager::kHideTreeViewBtn);

        d->sortByButton->setHidden(state & OptionButtonManager::kHideDetailSpaceBtn);
        d->viewOptionsButton->setVisible(!(state & OptionButtonManager::kHideDetailSpaceBtn));

        if (state == OptionButtonManager::kHideAllBtn)
            setContentsMargins(0, 0, 0, 0);
        else
            setContentsMargins(5, 0, 15, 0);
    } else {
        if (d->treeViewButton)
            d->treeViewButton->setHidden(false);
        d->listViewButton->setHidden(false);
        d->iconViewButton->setHidden(false);
        d->sortByButton->setHidden(false);
        d->viewOptionsButton->setHidden(false);
        setContentsMargins(5, 0, 15, 0);
    }
}

void OptionButtonBox::initializeUi()
{
    QSize buttonSize(kToolButtonSize, kToolButtonSize);
    QSize buttonIconSize(kToolButtonIconSize, kToolButtonIconSize);

    setContentsMargins(5, 0, 5, 0);
    d->buttonGroup = new QButtonGroup(this);

    d->iconViewButton = new DToolButton;
    d->iconViewButton->setCheckable(true);
    d->iconViewButton->setChecked(true);
    d->iconViewButton->setIcon(QIcon::fromTheme("dfm_viewlist_icons"));
    d->iconViewButton->setFixedSize(buttonSize);
    d->iconViewButton->setToolTip(tr("icons mode"));
    d->iconViewButton->setIconSize(buttonIconSize);

    d->listViewButton = new DToolButton;
    d->listViewButton->setCheckable(true);
    d->listViewButton->setIcon(QIcon::fromTheme("dfm_viewlist_details"));
    d->listViewButton->setFixedSize(buttonSize);
    d->listViewButton->setToolTip(tr("list mode"));
    d->listViewButton->setIconSize(buttonIconSize);
    d->buttonGroup->addButton(d->iconViewButton);
    d->buttonGroup->addButton(d->listViewButton);

    if (DConfigManager::instance()->value(kViewDConfName, kTreeViewEnable, true).toBool()) {
        d->treeViewButton = new DToolButton;
        d->treeViewButton->setCheckable(true);
        d->treeViewButton->setIcon(QIcon::fromTheme("dfm_viewlist_tree"));
        d->treeViewButton->setFixedSize(buttonSize);
        d->treeViewButton->setToolTip(tr("tree mode"));
        d->treeViewButton->setIconSize(buttonIconSize);
        d->buttonGroup->addButton(d->treeViewButton);
    }

    d->viewOptionsButton = new ViewOptionsButton(this);
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(d->listViewButton), AcName::kAcComputerTitleBarListViewBtn);
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(d->iconViewButton), AcName::kAcComputerTitleBarIconViewBtn);
    if (d->treeViewButton) {
        dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                             qobject_cast<QWidget *>(d->treeViewButton), AcName::kAcComputerTitleBarTreeViewBtn);
    }
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(d->viewOptionsButton), AcName::kAcComputerTitleBarViewOptionsBtn);
#endif
    d->sortByButton = new SortByButton(this);
    d->sortByButton->setFocusPolicy(Qt::NoFocus);
    d->sortByButton->setToolTip(tr("Sort by"));
    d->sortByButton->setIconSize(buttonIconSize);

    d->viewOptionsButton->setFocusPolicy(Qt::NoFocus);
    d->viewOptionsButton->setIcon(QIcon::fromTheme("dfm_viewoption"));
    d->viewOptionsButton->setFixedSize(buttonSize);
    d->viewOptionsButton->setToolTip(tr("View options"));
    d->viewOptionsButton->setIconSize(buttonIconSize);
    d->viewOptionsButton->setCheckable(false);
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

    connect(d->treeViewButton, &DToolButton::clicked, this, [this]() {
        d->setViewMode(ViewMode::kTreeMode);
    });

    connect(d->viewOptionsButton, &ViewOptionsButton::displayPreviewVisibleChanged, this, [this](bool checked) {
        TitleBarEventCaller::sendDetailViewState(this, checked);
    });

    connect(Application::instance(), &Application::viewModeChanged, d, &OptionButtonBoxPrivate::onViewModeChanged);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
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
    int fixedWidth = 10;
    d->hBoxLayout = new QHBoxLayout;
    d->hBoxLayout->setContentsMargins(0, 0, 0, 0);
    d->hBoxLayout->addWidget(d->iconViewButton);
    d->hBoxLayout->addWidget(d->listViewButton);
    fixedWidth += d->iconViewButton->width() + d->listViewButton->width();
    if (d->treeViewButton) {
        d->hBoxLayout->addWidget(d->treeViewButton);
        fixedWidth += d->treeViewButton->width();
    }
    d->hBoxLayout->addWidget(d->viewOptionsButton);
    fixedWidth += d->viewOptionsButton->width();
    d->hBoxLayout->addSpacing(10);
    d->hBoxLayout->addWidget(d->sortByButton);
    fixedWidth += d->sortByButton->width() + 10;

    setLayout(d->hBoxLayout);
    setFixedWidth(fixedWidth);
}

DToolButton *OptionButtonBox::listViewButton() const
{
    return d->listViewButton;
}

void OptionButtonBox::setListViewButton(DToolButton *listViewButton)
{
    if (!listViewButton) return;

    if (!d->hBoxLayout->replaceWidget(d->listViewButton, listViewButton)->isEmpty()) {

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

    if (!d->hBoxLayout->replaceWidget(d->iconViewButton, iconViewButton)->isEmpty()) {

        if (d->iconViewButton) {
            delete d->iconViewButton;
            d->iconViewButton = nullptr;
        }

        // Fix logic error, should set the new button
        d->iconViewButton = iconViewButton;

        if (d->iconViewButton->icon().isNull())
            d->iconViewButton->setIcon(QIcon::fromTheme("dfviewlist_icons"));

        d->iconViewButton->setCheckable(true);
        d->iconViewButton->setFocusPolicy(Qt::NoFocus);
    }
}
