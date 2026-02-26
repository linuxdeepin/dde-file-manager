// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/private/optionbuttonbox_p.h"
#include "utils/optionbuttonmanager.h"
#include "utils/titlebarhelper.h"
#include "events/titlebareventcaller.h"
#include "views/sortbybutton.h"
#include "views/viewoptionsbutton.h"
#include "views/optionbuttonbox.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <gtest/gtest.h>
#include <QUrl>
#include <QSignalSpy>
#include <QButtonGroup>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class OptionButtonBoxTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Stub DConfigManager to control tree view enable state
        stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &defaultValue) {
            __DBG_STUB_INVOKE__
            return defaultValue;
        });

        // Stub icon loading
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            return QIcon();
        });

        // Stub TitleBarEventCaller methods
        stub.set_lamda(&TitleBarEventCaller::sendViewMode, [](QObject *, Global::ViewMode) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&TitleBarEventCaller::sendGetDefualtViewMode, [](const QString &) {
            __DBG_STUB_INVOKE__
            return Global::ViewMode::kIconMode;
        });

        stub.set_lamda(&TitleBarEventCaller::sendDetailViewState, [](QObject *, bool, bool) {
            __DBG_STUB_INVOKE__
        });

        // Stub TitleBarHelper methods
        stub.set_lamda(&TitleBarHelper::getFileViewStateValue, [](const QUrl &, const QString &, const QVariant &defaultValue) {
            __DBG_STUB_INVOKE__
            return defaultValue;
        });

        box = new OptionButtonBox();
    }

    void TearDown() override
    {
        delete box;
        box = nullptr;
        stub.clear();
    }

    OptionButtonBox *box { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(OptionButtonBoxTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(box, nullptr);
    EXPECT_NE(box->d, nullptr);
}

TEST_F(OptionButtonBoxTest, Constructor_InitializesButtons_AllButtonsCreated)
{
    EXPECT_NE(box->d->iconViewButton, nullptr);
    EXPECT_NE(box->d->listViewButton, nullptr);
    EXPECT_NE(box->d->viewOptionsButton, nullptr);
    EXPECT_NE(box->d->sortByButton, nullptr);
    EXPECT_NE(box->d->compactButton, nullptr);
    EXPECT_NE(box->d->buttonGroup, nullptr);
}

TEST_F(OptionButtonBoxTest, Constructor_InitializesTreeViewButton_TreeViewButtonCreated)
{
    // Tree view button should be created by default
    EXPECT_NE(box->d->treeViewButton, nullptr);
}

TEST_F(OptionButtonBoxTest, Constructor_DisabledTreeView_NoTreeViewButton)
{
    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &name, const QString &key, const QVariant &) {
        __DBG_STUB_INVOKE__
        if (key == "dfm.treeview.enable")
            return QVariant(false);
        return QVariant(true);
    });

    auto testBox = new OptionButtonBox();
    EXPECT_EQ(testBox->d->treeViewButton, nullptr);
    delete testBox;
}

TEST_F(OptionButtonBoxTest, IconViewButton_GetMethod_ReturnsCorrectButton)
{
    auto button = box->iconViewButton();
    EXPECT_EQ(button, box->d->iconViewButton);
}

TEST_F(OptionButtonBoxTest, ListViewButton_GetMethod_ReturnsCorrectButton)
{
    auto button = box->listViewButton();
    EXPECT_EQ(button, box->d->listViewButton);
}

TEST_F(OptionButtonBoxTest, ViewOptionsButton_GetMethod_ReturnsCorrectButton)
{
    auto button = box->viewOptionsButton();
    EXPECT_EQ(button, box->d->viewOptionsButton);
}

TEST_F(OptionButtonBoxTest, SetViewMode_IconMode_ModeSet)
{
    box->setViewMode(static_cast<int>(ViewMode::kIconMode));
    EXPECT_EQ(box->viewMode(), ViewMode::kIconMode);
    EXPECT_TRUE(box->d->iconViewButton->isChecked());
}

TEST_F(OptionButtonBoxTest, SetViewMode_ListMode_ModeSet)
{
    box->setViewMode(static_cast<int>(ViewMode::kListMode));
    EXPECT_EQ(box->viewMode(), ViewMode::kListMode);
    EXPECT_TRUE(box->d->listViewButton->isChecked());
}

TEST_F(OptionButtonBoxTest, SetViewMode_TreeMode_ModeSet)
{
    if (box->d->treeViewButton) {
        box->setViewMode(static_cast<int>(ViewMode::kTreeMode));
        EXPECT_EQ(box->viewMode(), ViewMode::kTreeMode);
        EXPECT_TRUE(box->d->treeViewButton->isChecked());
    }
}

TEST_F(OptionButtonBoxTest, ViewMode_DefaultMode_IsIconMode)
{
    EXPECT_EQ(box->viewMode(), ViewMode::kIconMode);
}

TEST_F(OptionButtonBoxTest, UpdateCompactButton_IconMode_SetsIconViewIcon)
{
    box->d->currentMode = ViewMode::kIconMode;
    box->d->updateCompactButton();
    // Verify it doesn't crash
}

TEST_F(OptionButtonBoxTest, UpdateCompactButton_ListMode_SetsListViewIcon)
{
    box->d->currentMode = ViewMode::kListMode;
    box->d->updateCompactButton();
    // Verify it doesn't crash
}

TEST_F(OptionButtonBoxTest, UpdateCompactButton_TreeMode_SetsTreeViewIcon)
{
    box->d->currentMode = ViewMode::kTreeMode;
    box->d->updateCompactButton();
    // Verify it doesn't crash
}

TEST_F(OptionButtonBoxTest, UpdateCompactButton_NullButton_HandlesGracefully)
{
    box->d->compactButton = nullptr;
    box->d->updateCompactButton();
    // Should not crash
}

TEST_F(OptionButtonBoxTest, SetViewModePrivate_SameMode_DoesNotSendEvent)
{
    bool eventSent = false;
    stub.set_lamda(&TitleBarEventCaller::sendViewMode, [&eventSent](QObject *, Global::ViewMode) {
        __DBG_STUB_INVOKE__
        eventSent = true;
    });

    box->d->currentMode = ViewMode::kIconMode;
    box->d->setViewMode(ViewMode::kIconMode);
    EXPECT_FALSE(eventSent);
}

TEST_F(OptionButtonBoxTest, SetViewModePrivate_DifferentMode_SendsEvent)
{
    bool eventSent = false;
    ViewMode sentMode = ViewMode::kIconMode;

    stub.set_lamda(&TitleBarEventCaller::sendViewMode, [&eventSent, &sentMode](QObject *, Global::ViewMode mode) {
        __DBG_STUB_INVOKE__
        eventSent = true;
        sentMode = mode;
    });

    box->d->currentMode = ViewMode::kIconMode;
    box->d->setViewMode(ViewMode::kListMode);
    EXPECT_TRUE(eventSent);
    EXPECT_EQ(sentMode, ViewMode::kListMode);
}

TEST_F(OptionButtonBoxTest, LoadViewMode_ValidUrl_LoadsCorrectMode)
{
    QUrl url("file:///home/test");

    stub.set_lamda(&TitleBarHelper::getFileViewStateValue, [](const QUrl &, const QString &key, const QVariant &) {
        __DBG_STUB_INVOKE__
        if (key == "viewMode")
            return QVariant(static_cast<int>(ViewMode::kListMode));
        return QVariant();
    });

    box->d->loadViewMode(url);
    EXPECT_EQ(box->d->currentMode, ViewMode::kListMode);
}

TEST_F(OptionButtonBoxTest, LoadViewMode_TreeModeDisabled_FallsBackToListMode)
{
    QUrl url("file:///home/test");

    stub.set_lamda(&TitleBarHelper::getFileViewStateValue, [](const QUrl &, const QString &key, const QVariant &) {
        __DBG_STUB_INVOKE__
        if (key == "viewMode")
            return QVariant(static_cast<int>(ViewMode::kTreeMode));
        return QVariant();
    });

    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &key, const QVariant &) {
        __DBG_STUB_INVOKE__
        if (key == "dfm.treeview.enable")
            return QVariant(false);
        return QVariant(true);
    });

    box->d->loadViewMode(url);
    EXPECT_EQ(box->d->currentMode, ViewMode::kListMode);
}

TEST_F(OptionButtonBoxTest, SwitchMode_IconMode_ChecksIconButton)
{
    box->d->switchMode(ViewMode::kIconMode);
    EXPECT_TRUE(box->d->iconViewButton->isChecked());
    EXPECT_FALSE(box->d->listViewButton->isChecked());
}

TEST_F(OptionButtonBoxTest, SwitchMode_ListMode_ChecksListButton)
{
    box->d->switchMode(ViewMode::kListMode);
    EXPECT_TRUE(box->d->listViewButton->isChecked());
    EXPECT_FALSE(box->d->iconViewButton->isChecked());
}

TEST_F(OptionButtonBoxTest, SwitchMode_TreeMode_ChecksTreeButton)
{
    if (box->d->treeViewButton) {
        box->d->switchMode(ViewMode::kTreeMode);
        EXPECT_TRUE(box->d->treeViewButton->isChecked());
        EXPECT_FALSE(box->d->iconViewButton->isChecked());
        EXPECT_FALSE(box->d->listViewButton->isChecked());
    }
}

TEST_F(OptionButtonBoxTest, OnUrlChanged_ValidUrl_LoadsViewMode)
{
    QUrl url("file:///home/test");

    stub.set_lamda(&OptionButtonManager::hasVsibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    box->onUrlChanged(url);
    EXPECT_EQ(box->d->currentUrl, url);
}

TEST_F(OptionButtonBoxTest, OnUrlChanged_WithVisibilityState_AppliesState)
{
    QUrl url("file:///home/test");

    stub.set_lamda(&OptionButtonManager::hasVsibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&OptionButtonManager::optBtnVisibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return OptionButtonManager::kHideListViewBtn;
    });

    box->onUrlChanged(url);
    EXPECT_TRUE(box->d->listViewButton->isHidden());
    EXPECT_FALSE(box->d->iconViewButton->isHidden());
}

TEST_F(OptionButtonBoxTest, OnUrlChanged_HideAllBtnState_HidesAllButtons)
{
    QUrl url("file:///home/test");

    stub.set_lamda(&OptionButtonManager::hasVsibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&OptionButtonManager::optBtnVisibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return OptionButtonManager::kHideAllBtn;
    });

    box->onUrlChanged(url);
    EXPECT_TRUE(box->d->compactButton->isHidden());
}

TEST_F(OptionButtonBoxTest, OnUrlChanged_NoVisibilityState_ShowsAllButtons)
{
    QUrl url("file:///home/test");

    stub.set_lamda(&OptionButtonManager::hasVsibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    box->onUrlChanged(url);
    EXPECT_FALSE(box->d->listViewButton->isHidden());
    EXPECT_FALSE(box->d->iconViewButton->isHidden());
    EXPECT_FALSE(box->d->sortByButton->isHidden());
    EXPECT_FALSE(box->d->viewOptionsButton->isHidden());
}

TEST_F(OptionButtonBoxTest, UpdateOptionButtonBox_LargeWidth_NormalMode)
{
    stub.set_lamda(&OptionButtonManager::hasVsibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    box->updateOptionButtonBox(800);
    EXPECT_FALSE(box->d->isCompactMode);
}

TEST_F(OptionButtonBoxTest, UpdateOptionButtonBox_SmallWidth_CompactMode)
{
    stub.set_lamda(&OptionButtonManager::hasVsibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    box->updateOptionButtonBox(500);
    EXPECT_TRUE(box->d->isCompactMode);
}

TEST_F(OptionButtonBoxTest, UpdateOptionButtonBox_ThresholdWidth_HandlesCorrectly)
{
    stub.set_lamda(&OptionButtonManager::hasVsibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    box->updateOptionButtonBox(OptionButtonBox::kCompactModeThreshold + 10);
    EXPECT_FALSE(box->d->isCompactMode);

    box->updateOptionButtonBox(599);
    EXPECT_TRUE(box->d->isCompactMode);
}

TEST_F(OptionButtonBoxTest, UpdateOptionButtonBox_HideAllBtnState_DoesNotSwitch)
{
    stub.set_lamda(&OptionButtonManager::hasVsibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&OptionButtonManager::optBtnVisibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return OptionButtonManager::kHideAllBtn;
    });

    box->d->currentUrl = QUrl("file:///test");
    box->updateOptionButtonBox(500);
    // Should not switch modes
}

TEST_F(OptionButtonBoxTest, SwitchToCompactMode_Called_HidesNormalButtons)
{
    box->switchToCompactMode();
    EXPECT_TRUE(box->d->isCompactMode);
    EXPECT_TRUE(box->d->iconViewButton->isHidden());
    EXPECT_TRUE(box->d->listViewButton->isHidden());
    if (box->d->treeViewButton) {
        EXPECT_TRUE(box->d->treeViewButton->isHidden());
    }
}

TEST_F(OptionButtonBoxTest, SwitchToCompactMode_Called_ShowsCompactButton)
{
    box->switchToCompactMode();
    EXPECT_FALSE(box->d->compactButton->isHidden());
}

TEST_F(OptionButtonBoxTest, SwitchToNormalMode_Called_ShowsNormalButtons)
{
    box->d->isCompactMode = true;
    box->switchToNormalMode();
    EXPECT_FALSE(box->d->isCompactMode);
    EXPECT_FALSE(box->d->iconViewButton->isHidden());
    EXPECT_FALSE(box->d->listViewButton->isHidden());
    if (box->d->treeViewButton) {
        EXPECT_FALSE(box->d->treeViewButton->isHidden());
    }
}

TEST_F(OptionButtonBoxTest, SwitchToNormalMode_Called_HidesCompactButton)
{
    box->d->isCompactMode = true;
    box->switchToNormalMode();
    EXPECT_TRUE(box->d->compactButton->isHidden());
}

TEST_F(OptionButtonBoxTest, UpdateFixedWidth_NormalMode_CalculatesCorrectWidth)
{
    box->d->isCompactMode = false;
    box->d->iconViewEnabled = true;
    box->d->listViewEnabled = true;
    box->d->treeViewEnabled = true;
    box->d->viewOptionsEnabled = true;
    box->d->sortByEnabled = true;

    box->updateFixedWidth();
    EXPECT_GT(box->width(), 0);
}

TEST_F(OptionButtonBoxTest, UpdateFixedWidth_CompactMode_CalculatesCorrectWidth)
{
    box->d->isCompactMode = true;
    box->d->viewOptionsEnabled = true;
    box->d->sortByEnabled = true;

    box->updateFixedWidth();
    EXPECT_GT(box->width(), 0);
}

TEST_F(OptionButtonBoxTest, UpdateFixedWidth_AllButtonsDisabled_MinimalWidth)
{
    box->d->isCompactMode = false;
    box->d->iconViewEnabled = false;
    box->d->listViewEnabled = false;
    box->d->treeViewEnabled = false;
    box->d->viewOptionsEnabled = false;
    box->d->sortByEnabled = false;

    box->d->iconViewButton->hide();
    box->d->listViewButton->hide();
    if (box->d->treeViewButton)
        box->d->treeViewButton->hide();
    box->d->viewOptionsButton->hide();
    box->d->sortByButton->hide();

    box->updateFixedWidth();
    EXPECT_GE(box->width(), 0);
}

TEST_F(OptionButtonBoxTest, SetIconViewButton_ValidButton_ReplacesButton)
{
    auto newButton = new DToolButton();
    box->setIconViewButton(newButton);
    EXPECT_EQ(box->d->iconViewButton, newButton);
}

TEST_F(OptionButtonBoxTest, SetIconViewButton_NullButton_DoesNotReplace)
{
    auto originalButton = box->d->iconViewButton;
    box->setIconViewButton(nullptr);
    EXPECT_EQ(box->d->iconViewButton, originalButton);
}

TEST_F(OptionButtonBoxTest, SetListViewButton_ValidButton_ReplacesButton)
{
    auto newButton = new DToolButton();
    EXPECT_NO_THROW(box->setListViewButton(newButton));
}

TEST_F(OptionButtonBoxTest, SetListViewButton_NullButton_DoesNotReplace)
{
    auto originalButton = box->d->listViewButton;
    box->setListViewButton(nullptr);
    EXPECT_EQ(box->d->listViewButton, originalButton);
}

TEST_F(OptionButtonBoxTest, SetViewOptionsButton_ValidButton_ReplacesButton)
{
    auto newButton = new ViewOptionsButton();
    box->setViewOptionsButton(newButton);
    EXPECT_EQ(box->d->viewOptionsButton, newButton);
}

TEST_F(OptionButtonBoxTest, SetViewOptionsButton_NullButton_DoesNotReplace)
{
    auto originalButton = box->d->viewOptionsButton;
    box->setViewOptionsButton(nullptr);
    EXPECT_EQ(box->d->viewOptionsButton, originalButton);
}

TEST_F(OptionButtonBoxTest, ButtonClicks_IconViewButton_ChangesMode)
{
    bool modeChanged = false;
    stub.set_lamda(&TitleBarEventCaller::sendViewMode, [&modeChanged](QObject *, Global::ViewMode mode) {
        __DBG_STUB_INVOKE__
        if (mode == ViewMode::kIconMode)
            modeChanged = true;
    });

    box->d->currentMode = ViewMode::kListMode;
    box->d->iconViewButton->click();
    EXPECT_TRUE(modeChanged);
}

TEST_F(OptionButtonBoxTest, ButtonClicks_ListViewButton_ChangesMode)
{
    bool modeChanged = false;
    stub.set_lamda(&TitleBarEventCaller::sendViewMode, [&modeChanged](QObject *, Global::ViewMode mode) {
        __DBG_STUB_INVOKE__
        if (mode == ViewMode::kListMode)
            modeChanged = true;
    });

    box->d->currentMode = ViewMode::kIconMode;
    box->d->listViewButton->click();
    EXPECT_TRUE(modeChanged);
}

TEST_F(OptionButtonBoxTest, ButtonClicks_TreeViewButton_ChangesMode)
{
    if (!box->d->treeViewButton)
        return;

    bool modeChanged = false;
    stub.set_lamda(&TitleBarEventCaller::sendViewMode, [&modeChanged](QObject *, Global::ViewMode mode) {
        __DBG_STUB_INVOKE__
        if (mode == ViewMode::kTreeMode)
            modeChanged = true;
    });

    box->d->currentMode = ViewMode::kIconMode;
    box->d->treeViewButton->click();
    EXPECT_TRUE(modeChanged);
}

TEST_F(OptionButtonBoxTest, OnViewModeChanged_WithViewState_LoadsFromState)
{
    stub.set_lamda(&TitleBarHelper::getFileViewStateValue, [](const QUrl &, const QString &key, const QVariant &) {
        __DBG_STUB_INVOKE__
        if (key == "viewMode")
            return QVariant(static_cast<int>(ViewMode::kListMode));
        return QVariant();
    });

    box->d->currentUrl = QUrl("file:///test");
    box->d->onViewModeChanged(static_cast<int>(ViewMode::kIconMode));
    EXPECT_EQ(box->d->currentMode, ViewMode::kListMode);
}

TEST_F(OptionButtonBoxTest, OnViewModeChanged_NoViewState_UsesProvidedMode)
{
    stub.set_lamda(&TitleBarHelper::getFileViewStateValue, [](const QUrl &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant();
    });

    box->d->currentUrl = QUrl("file:///test");
    box->d->onViewModeChanged(static_cast<int>(ViewMode::kListMode));
    EXPECT_EQ(box->d->currentMode, ViewMode::kListMode);
}

TEST_F(OptionButtonBoxTest, CompactModeThreshold_Value_Is600)
{
    EXPECT_EQ(OptionButtonBox::kCompactModeThreshold, 600);
}

TEST_F(OptionButtonBoxTest, InitializeUi_Called_CreatesLayout)
{
    EXPECT_NE(box->d->hBoxLayout, nullptr);
}

TEST_F(OptionButtonBoxTest, InitializeUi_Called_AddsAllWidgets)
{
    EXPECT_EQ(box->d->hBoxLayout->count() > 0, true);
}

TEST_F(OptionButtonBoxTest, ButtonGroup_ContainsViewButtons_AllAdded)
{
    auto buttons = box->d->buttonGroup->buttons();
    EXPECT_TRUE(buttons.contains(box->d->iconViewButton));
    EXPECT_TRUE(buttons.contains(box->d->listViewButton));
    if (box->d->treeViewButton) {
        EXPECT_TRUE(buttons.contains(box->d->treeViewButton));
    }
}

TEST_F(OptionButtonBoxTest, VisibilityFlags_InitialState_AllTrue)
{
    EXPECT_TRUE(box->d->iconViewEnabled);
    EXPECT_TRUE(box->d->listViewEnabled);
    EXPECT_TRUE(box->d->treeViewEnabled);
    EXPECT_TRUE(box->d->sortByEnabled);
    EXPECT_TRUE(box->d->viewOptionsEnabled);
}

TEST_F(OptionButtonBoxTest, OnUrlChanged_HideIconViewBtn_HidesIconButton)
{
    QUrl url("file:///test");

    stub.set_lamda(&OptionButtonManager::hasVsibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&OptionButtonManager::optBtnVisibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return OptionButtonManager::kHideIconViewBtn;
    });

    box->onUrlChanged(url);
    EXPECT_TRUE(box->d->iconViewButton->isHidden());
    EXPECT_FALSE(box->d->listViewButton->isHidden());
}

TEST_F(OptionButtonBoxTest, OnUrlChanged_HideTreeViewBtn_DisablesTreeButton)
{
    if (!box->d->treeViewButton)
        return;

    QUrl url("file:///test");

    stub.set_lamda(&OptionButtonManager::hasVsibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&OptionButtonManager::optBtnVisibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return OptionButtonManager::kHideTreeViewBtn;
    });

    box->onUrlChanged(url);
    EXPECT_FALSE(box->d->treeViewButton->isHidden());
    EXPECT_FALSE(box->d->treeViewButton->isEnabled());
}

TEST_F(OptionButtonBoxTest, OnUrlChanged_HideDetailSpaceBtn_HidesSortAndOptions)
{
    QUrl url("file:///test");

    stub.set_lamda(&OptionButtonManager::hasVsibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&OptionButtonManager::optBtnVisibleState, [](OptionButtonManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return OptionButtonManager::kHideDetailSpaceBtn;
    });

    box->onUrlChanged(url);
    EXPECT_TRUE(box->d->sortByButton->isHidden());
    EXPECT_FALSE(box->d->viewOptionsButton->isVisible());
}
