// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/optionbuttonbox_p.h"
#include "views/optionbuttonbox.h"
#include "events/titlebareventcaller.h"
#include "utils/optionbuttonmanager.h"
#include "utils/titlebarhelper.h"
#include "views/sortbybutton.h"
#include "views/viewoptionsbutton.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/widgets/dfmcustombuttons/customdtoolbutton.h>

#include <dfm-framework/event/event.h>

#include <DMenu>
#include <DGuiApplicationHelper>
#include <dtkwidget_global.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QAction>
#include <QDebug>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

OptionButtonBoxPrivate::OptionButtonBoxPrivate(OptionButtonBox *parent)
    : QObject(parent), q(parent)
{
}

void OptionButtonBoxPrivate::updateCompactButton()
{
    if (!compactButton) {
        fmWarning() << "Compact button is null, cannot update";
        return;
    }

    // Update icon based on current view mode
    switch (currentMode) {
    case ViewMode::kIconMode:
        compactButton->setIcon(QIcon::fromTheme("dfm_viewlist_icons"));
        break;
    case ViewMode::kListMode:
        compactButton->setIcon(QIcon::fromTheme("dfm_viewlist_details"));
        break;
    case ViewMode::kTreeMode:
        compactButton->setIcon(QIcon::fromTheme("dfm_viewlist_tree"));
        break;
    default:
        fmWarning() << "Unknown view mode for compact button:" << int(currentMode);
        break;
    }
}

void OptionButtonBoxPrivate::setViewMode(ViewMode mode)
{
    if (currentMode == mode) {
        fmDebug() << "The current mode already:" << int(mode);
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
    auto viewMode = static_cast<ViewMode>(TitleBarHelper::getFileViewStateValue(url, "viewMode", defaultViewMode).toInt());
    if (viewMode == ViewMode::kTreeMode && !DConfigManager::instance()->value(kViewDConfName, kTreeViewEnable, true).toBool()) {
        fmInfo() << "Tree view is disabled in config, switching to list mode";
        viewMode = ViewMode::kListMode;
    }

    switchMode(viewMode);
}

void OptionButtonBoxPrivate::switchMode(ViewMode mode)
{
    currentMode = mode;
    switch (mode) {
    case ViewMode::kIconMode:
        iconViewButton->setChecked(true);
        fmDebug() << "Icon view button checked";
        break;
    case ViewMode::kListMode:
        listViewButton->setChecked(true);
        fmDebug() << "List view button checked";
        break;
    case ViewMode::kTreeMode:
        treeViewButton->setChecked(true);
        fmDebug() << "Tree view button checked";
        break;
    default:
        fmWarning() << "Unknown view mode in switchMode:" << int(mode);
        break;
    }
    viewOptionsButton->switchMode(mode, currentUrl);
    updateCompactButton();
}

void OptionButtonBoxPrivate::onViewModeChanged(int mode)
{
    if (!TitleBarHelper::getFileViewStateValue(currentUrl, "viewMode").isNull()) {
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
    if (!button) {
        fmWarning() << "Trying to set null view options button";
        return;
    }

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
    } else {
        fmWarning() << "Failed to replace view options button";
    }
}

void OptionButtonBox::setViewMode(int mode)
{
    d->switchMode(static_cast<ViewMode>(mode));
}

ViewMode OptionButtonBox::viewMode() const
{
    return d->currentMode;
}

void OptionButtonBox::updateOptionButtonBox(int parentWidth)
{
    // If the current url scheme has no button visibility state set, do not hide buttons
    if (OptionButtonManager::instance()->hasVsibleState(d->currentUrl.scheme())
        && OptionButtonManager::instance()->optBtnVisibleState(d->currentUrl.scheme()) == OptionButtonManager::kHideAllBtn)
        return;

    if (parentWidth <= kCompactModeThreshold) {
        if (!d->isCompactMode) {
            switchToCompactMode();
            updateFixedWidth();
        }
    } else {
        if (d->isCompactMode) {
            switchToNormalMode();
            updateFixedWidth();
        }
    }
}

void OptionButtonBox::onUrlChanged(const QUrl &url)
{
    fmDebug() << "URL changed to:" << url.toString();
    d->currentUrl = url;
    d->loadViewMode(url);
    if (OptionButtonManager::instance()->hasVsibleState(url.scheme())) {
        auto state = OptionButtonManager::instance()->optBtnVisibleState(url.scheme());
        fmDebug() << "Button visibility state for scheme" << url.scheme() << "is" << state;

        // Store the state-based visibility
        d->listViewEnabled = !(state & OptionButtonManager::kHideListViewBtn);
        d->iconViewEnabled = !(state & OptionButtonManager::kHideIconViewBtn);
        // 判断是否需要禁用树视图按钮
        const bool needDisableTreeBtn = state & OptionButtonManager::kHideTreeViewBtn;
        // 保持按钮可见，但根据需要禁用它
        d->treeViewEnabled = d->treeViewButton;
        d->sortByEnabled = !(state & OptionButtonManager::kHideDetailSpaceBtn);
        d->viewOptionsEnabled = !(state & OptionButtonManager::kHideDetailSpaceBtn);

        // Apply the state visibility
        d->listViewButton->setHidden(!d->listViewEnabled);
        d->iconViewButton->setHidden(!d->iconViewEnabled);
        if (d->treeViewButton) {
            // 不隐藏按钮，只是在需要时禁用它
            d->treeViewButton->setHidden(false);
            d->treeViewButton->setEnabled(!needDisableTreeBtn);
        }
        d->sortByButton->setHidden(!d->sortByEnabled);
        d->viewOptionsButton->setVisible(d->viewOptionsEnabled);

        if (state == OptionButtonManager::kHideAllBtn) {
            setContentsMargins(0, 0, 0, 0);
            d->compactButton->hide();
        } else {
            setContentsMargins(5, 0, 0, 0);
        }
    } else {
        fmDebug() << "No visibility state found for scheme, showing all buttons";
        // Reset all state-based visibility to true
        d->listViewEnabled = true;
        d->iconViewEnabled = true;
        d->treeViewEnabled = true;
        d->sortByEnabled = true;
        d->viewOptionsEnabled = true;

        if (d->treeViewButton) {
            d->treeViewButton->setHidden(false);
            // 正常状态下，确保启用树视图按钮
            d->treeViewButton->setEnabled(true);
        }
        d->listViewButton->setHidden(false);
        d->iconViewButton->setHidden(false);
        d->sortByButton->setHidden(false);
        d->viewOptionsButton->setHidden(false);
        setContentsMargins(5, 0, 0, 0);
    }

    // Update button box size according to the parent widget width
    if (parent() && qobject_cast<QWidget *>(parent())) {
        if (OptionButtonManager::instance()->hasVsibleState(d->currentUrl.scheme())
            && OptionButtonManager::instance()->optBtnVisibleState(d->currentUrl.scheme()) == OptionButtonManager::kHideAllBtn) {
            setFixedWidth(0);
            return;
        }
        if (qobject_cast<QWidget *>(parent())->width() <= kCompactModeThreshold) {
            switchToCompactMode();
        } else {
            switchToNormalMode();
        }
        updateFixedWidth();
    }
}

void OptionButtonBox::initializeUi()
{
    QSize buttonSize(kToolButtonSize, kToolButtonSize);
    QSize buttonIconSize(kToolButtonIconSize, kToolButtonIconSize);

    setContentsMargins(5, 0, 0, 0);
    d->buttonGroup = new QButtonGroup(this);

    d->iconViewButton = new CustomDToolButton;
    d->iconViewButton->setCheckable(true);
    d->iconViewButton->setChecked(true);
    d->iconViewButton->setIcon(QIcon::fromTheme("dfm_viewlist_icons"));
    d->iconViewButton->setFixedSize(buttonSize);
    d->iconViewButton->setToolTip(tr("Icon view"));
    d->iconViewButton->setIconSize(buttonIconSize);
    d->iconViewButton->setFocusPolicy(Qt::NoFocus);

    d->listViewButton = new CustomDToolButton;
    d->listViewButton->setCheckable(true);
    d->listViewButton->setIcon(QIcon::fromTheme("dfm_viewlist_details"));
    d->listViewButton->setFixedSize(buttonSize);
    d->listViewButton->setToolTip(tr("List view"));
    d->listViewButton->setIconSize(buttonIconSize);
    d->listViewButton->setFocusPolicy(Qt::NoFocus);
    d->buttonGroup->addButton(d->iconViewButton);
    d->buttonGroup->addButton(d->listViewButton);

    if (DConfigManager::instance()->value(kViewDConfName, kTreeViewEnable, true).toBool()) {
        fmDebug() << "Tree view is enabled, creating tree view button";
        d->treeViewButton = new CustomDToolButton;
        d->treeViewButton->setCheckable(true);
        d->treeViewButton->setIcon(QIcon::fromTheme("dfm_viewlist_tree"));
        d->treeViewButton->setFixedSize(buttonSize);
        d->treeViewButton->setToolTip(tr("Tree view"));
        d->treeViewButton->setIconSize(buttonIconSize);
        d->treeViewButton->setFocusPolicy(Qt::NoFocus);
        d->buttonGroup->addButton(d->treeViewButton);
    } else {
        fmDebug() << "Tree view is disabled in configuration";
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

    d->compactButton = new CustomDToolButton(this);
    d->compactButton->setPopupMode(DToolButton::InstantPopup);
    d->compactButton->setFixedSize(48, kToolButtonSize);
    d->compactButton->setIconSize(QSize(kToolButtonIconSize, kToolButtonIconSize));
    d->compactButton->setVisible(false);

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

    connect(d->viewOptionsButton, &ViewOptionsButton::displayPreviewVisibleChanged, this, [this](bool checked, bool userAction) {
        TitleBarEventCaller::sendDetailViewState(this, checked, userAction);
    });

    connect(Application::instance(), &Application::viewModeChanged, d, &OptionButtonBoxPrivate::onViewModeChanged);

    auto menu = new DMenu(d->compactButton);
    auto iconAction = menu->addAction(tr("Icon view"));
    iconAction->setIcon(QIcon::fromTheme("dfm_viewlist_icons"));
    iconAction->setCheckable(true);

    auto listAction = menu->addAction(tr("List view"));
    listAction->setIcon(QIcon::fromTheme("dfm_viewlist_details"));
    listAction->setCheckable(true);

    auto treeAction = menu->addAction(tr("Tree view"));
    treeAction->setIcon(QIcon::fromTheme("dfm_viewlist_tree"));
    treeAction->setCheckable(true);

    auto updateCheckedState = [=]() {
        iconAction->setChecked(d->currentMode == ViewMode::kIconMode);
        listAction->setChecked(d->currentMode == ViewMode::kListMode);
        treeAction->setChecked(d->currentMode == ViewMode::kTreeMode);
    };

    connect(iconAction, &QAction::triggered, this, [this]() {
        d->setViewMode(ViewMode::kIconMode);
    });
    connect(listAction, &QAction::triggered, this, [this]() {
        d->setViewMode(ViewMode::kListMode);
    });
    connect(treeAction, &QAction::triggered, this, [this]() {
        d->setViewMode(ViewMode::kTreeMode);
    });

    connect(menu, &DMenu::aboutToShow, this, updateCheckedState);
    d->compactButton->setMenu(menu);

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
    d->hBoxLayout = new QHBoxLayout;
    d->hBoxLayout->setSpacing(0);
    d->hBoxLayout->setContentsMargins(0, 0, 0, 0);

    // Add compact button (for compact mode)
    d->hBoxLayout->addWidget(d->compactButton);
    d->hBoxLayout->addSpacing(10);

    // Add view mode buttons (left side)
    d->hBoxLayout->addWidget(d->iconViewButton);
    d->hBoxLayout->addWidget(d->listViewButton);
    if (d->treeViewButton) {
        d->hBoxLayout->addWidget(d->treeViewButton);
    }
    d->hBoxLayout->addWidget(d->viewOptionsButton);

    // Add controlled stretch to push sortByButton to the right while limiting total spacing
    d->hBoxLayout->addStretch(1);

    // Add sortByButton (right side - position will remain fixed)
    d->hBoxLayout->addWidget(d->sortByButton);

    setLayout(d->hBoxLayout);
}

void OptionButtonBox::updateFixedWidth()
{
    int fixedWidth = 10;   // Initial spacing

    // View mode buttons section (left side)
    if (d->isCompactMode) {
        fixedWidth += 48;   // Compact button width
        fixedWidth += 10;   // Spacing after compact button
        fixedWidth -= 20;
    } else {
        fixedWidth += 10;   // Spacing after compact button, using their actual width
        // Only count visible view mode buttons (each is kToolButtonSize = 30)
        if (d->iconViewEnabled && d->iconViewButton && !d->iconViewButton->isHidden())
            fixedWidth += d->iconViewButton->width();
        if (d->listViewEnabled && d->listViewButton && !d->listViewButton->isHidden())
            fixedWidth += d->listViewButton->width();
        if (d->treeViewEnabled && d->treeViewButton && !d->treeViewButton->isHidden())
            fixedWidth += d->treeViewButton->width();
    }

    // ViewOptions button (part of left side, also kToolButtonSize = 30)
    if (d->viewOptionsEnabled && d->viewOptionsButton && !d->viewOptionsButton->isHidden())
        fixedWidth += kToolButtonSize;

    // Small fixed spacing + controlled stretch space to achieve ~15px total
    fixedWidth += 5;   // Fixed spacing before stretch
    // fixedWidth += 10;   // Controlled stretch space (5px + 10px = 15px total)

    // SortByButton (right side - width is 46 when visible)
    if (d->sortByEnabled && d->sortByButton && !d->sortByButton->isHidden())
        fixedWidth += 46;   // kSortToolButtonWidth

    setFixedWidth(fixedWidth);
}

DToolButton *OptionButtonBox::listViewButton() const
{
    return d->listViewButton;
}

void OptionButtonBox::setListViewButton(DToolButton *listViewButton)
{
    if (!listViewButton) {
        fmWarning() << "Trying to set null list view button";
        return;
    }

    if (!d->hBoxLayout->replaceWidget(d->listViewButton, listViewButton)->isEmpty()) {

        if (d->listViewButton) {
            delete d->listViewButton;
            d->listViewButton = nullptr;
        }

        if (listViewButton)
            d->listViewButton = listViewButton;

        if (d->listViewButton->icon().isNull())
            d->listViewButton->setIcon(QIcon::fromTheme("dfviewlist_details"));

        d->listViewButton->setCheckable(true);
        d->listViewButton->setFocusPolicy(Qt::NoFocus);
    } else {
        fmWarning() << "Failed to replace list view button";
    }
}

OptionButtonBox::OptionButtonBox(QWidget *parent)
    : QWidget(parent), d(new OptionButtonBoxPrivate(this))
{
    initializeUi();
    initConnect();
    if (parent) {
        updateOptionButtonBox(parent->width());
    }
}

DToolButton *OptionButtonBox::iconViewButton() const
{
    return d->iconViewButton;
}

void OptionButtonBox::setIconViewButton(DToolButton *iconViewButton)
{
    if (!iconViewButton) {
        fmWarning() << "Trying to set null icon view button";
        return;
    }

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
    } else {
        fmWarning() << "Failed to replace icon view button";
    }
}

void OptionButtonBox::switchToCompactMode()
{
    // Hide normal buttons if they are enabled by state
    if (d->iconViewEnabled)
        d->iconViewButton->hide();
    if (d->listViewEnabled)
        d->listViewButton->hide();
    if (d->treeViewButton && d->treeViewEnabled)
        d->treeViewButton->hide();

    // Show compact button if any view mode is enabled
    if (d->compactButton && (d->iconViewEnabled || d->listViewEnabled || (d->treeViewButton && d->treeViewEnabled))) {
        d->compactButton->show();
        d->updateCompactButton();
    }

    d->isCompactMode = true;
}

void OptionButtonBox::switchToNormalMode()
{
    if (d->compactButton)
        d->compactButton->hide();

    // Show normal buttons if they are enabled by state
    if (d->iconViewEnabled)
        d->iconViewButton->show();
    if (d->listViewEnabled)
        d->listViewButton->show();
    if (d->treeViewButton && d->treeViewEnabled)
        d->treeViewButton->show();

    d->isCompactMode = false;
}
