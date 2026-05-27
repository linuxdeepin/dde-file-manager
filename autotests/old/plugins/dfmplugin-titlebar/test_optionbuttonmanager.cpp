// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/optionbuttonmanager.h"

#include <gtest/gtest.h>
#include <QHash>

using namespace dfmplugin_titlebar;

class OptionButtonManagerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        manager = OptionButtonManager::instance();
        // Clear the state map before each test
        manager->stateMap.clear();
    }

    void TearDown() override
    {
        // Clear the state map after each test to ensure test independence
        manager->stateMap.clear();
        stub.clear();
    }

    OptionButtonManager *manager { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(OptionButtonManagerTest, Instance_Singleton_ReturnsSameInstance)
{
    auto manager1 = OptionButtonManager::instance();
    auto manager2 = OptionButtonManager::instance();
    EXPECT_EQ(manager1, manager2);
    EXPECT_NE(manager1, nullptr);
}

TEST_F(OptionButtonManagerTest, SetOptBtnVisibleState_ValidScheme_StateSet)
{
    QString scheme = "file";
    OptionButtonManager::OptBtnVisibleState state = OptionButtonManager::kDoNotHide;

    manager->setOptBtnVisibleState(scheme, state);
    EXPECT_TRUE(manager->stateMap.contains(scheme));
    EXPECT_EQ(manager->stateMap[scheme], state);
}

TEST_F(OptionButtonManagerTest, SetOptBtnVisibleState_HideListViewBtn_StateSet)
{
    QString scheme = "file";
    OptionButtonManager::OptBtnVisibleState state = OptionButtonManager::kHideListViewBtn;

    manager->setOptBtnVisibleState(scheme, state);
    EXPECT_EQ(manager->stateMap[scheme], OptionButtonManager::kHideListViewBtn);
}

TEST_F(OptionButtonManagerTest, SetOptBtnVisibleState_HideIconViewBtn_StateSet)
{
    QString scheme = "file";
    OptionButtonManager::OptBtnVisibleState state = OptionButtonManager::kHideIconViewBtn;

    manager->setOptBtnVisibleState(scheme, state);
    EXPECT_EQ(manager->stateMap[scheme], OptionButtonManager::kHideIconViewBtn);
}

TEST_F(OptionButtonManagerTest, SetOptBtnVisibleState_HideDetailSpaceBtn_StateSet)
{
    QString scheme = "trash";
    OptionButtonManager::OptBtnVisibleState state = OptionButtonManager::kHideDetailSpaceBtn;

    manager->setOptBtnVisibleState(scheme, state);
    EXPECT_EQ(manager->stateMap[scheme], OptionButtonManager::kHideDetailSpaceBtn);
}

TEST_F(OptionButtonManagerTest, SetOptBtnVisibleState_HideTreeViewBtn_StateSet)
{
    QString scheme = "computer";
    OptionButtonManager::OptBtnVisibleState state = OptionButtonManager::kHideTreeViewBtn;

    manager->setOptBtnVisibleState(scheme, state);
    EXPECT_EQ(manager->stateMap[scheme], OptionButtonManager::kHideTreeViewBtn);
}

TEST_F(OptionButtonManagerTest, SetOptBtnVisibleState_HideListHeightOpt_StateSet)
{
    QString scheme = "recent";
    OptionButtonManager::OptBtnVisibleState state = OptionButtonManager::kHideListHeightOpt;

    manager->setOptBtnVisibleState(scheme, state);
    EXPECT_EQ(manager->stateMap[scheme], OptionButtonManager::kHideListHeightOpt);
}

TEST_F(OptionButtonManagerTest, SetOptBtnVisibleState_HideAllBtn_StateSet)
{
    QString scheme = "vault";
    OptionButtonManager::OptBtnVisibleState state = OptionButtonManager::kHideAllBtn;

    manager->setOptBtnVisibleState(scheme, state);
    EXPECT_EQ(manager->stateMap[scheme], OptionButtonManager::kHideAllBtn);
}

TEST_F(OptionButtonManagerTest, SetOptBtnVisibleState_CombinedFlags_StateSet)
{
    QString scheme = "smb";
    OptionButtonManager::OptBtnVisibleState state =
        static_cast<OptionButtonManager::OptBtnVisibleState>(
            OptionButtonManager::kHideListViewBtn | OptionButtonManager::kHideIconViewBtn);

    manager->setOptBtnVisibleState(scheme, state);
    EXPECT_EQ(manager->stateMap[scheme], state);
}

TEST_F(OptionButtonManagerTest, SetOptBtnVisibleState_UpdateExistingScheme_StateUpdated)
{
    QString scheme = "file";

    manager->setOptBtnVisibleState(scheme, OptionButtonManager::kHideListViewBtn);
    EXPECT_EQ(manager->stateMap[scheme], OptionButtonManager::kHideListViewBtn);

    manager->setOptBtnVisibleState(scheme, OptionButtonManager::kHideIconViewBtn);
    EXPECT_EQ(manager->stateMap[scheme], OptionButtonManager::kHideIconViewBtn);
}

TEST_F(OptionButtonManagerTest, SetOptBtnVisibleState_MultipleSchemes_AllStatesSet)
{
    manager->setOptBtnVisibleState("file", OptionButtonManager::kHideListViewBtn);
    manager->setOptBtnVisibleState("trash", OptionButtonManager::kHideIconViewBtn);
    manager->setOptBtnVisibleState("recent", OptionButtonManager::kHideDetailSpaceBtn);

    EXPECT_EQ(manager->stateMap["file"], OptionButtonManager::kHideListViewBtn);
    EXPECT_EQ(manager->stateMap["trash"], OptionButtonManager::kHideIconViewBtn);
    EXPECT_EQ(manager->stateMap["recent"], OptionButtonManager::kHideDetailSpaceBtn);
    EXPECT_EQ(manager->stateMap.size(), 3);
}

TEST_F(OptionButtonManagerTest, OptBtnVisibleState_ExistingScheme_ReturnsCorrectState)
{
    QString scheme = "file";
    OptionButtonManager::OptBtnVisibleState expectedState = OptionButtonManager::kHideListViewBtn;

    manager->setOptBtnVisibleState(scheme, expectedState);
    OptionButtonManager::OptBtnVisibleState actualState = manager->optBtnVisibleState(scheme);

    EXPECT_EQ(actualState, expectedState);
}

TEST_F(OptionButtonManagerTest, OptBtnVisibleState_NonExistingScheme_ReturnsDefaultState)
{
    QString scheme = "nonexistent";
    OptionButtonManager::OptBtnVisibleState state = manager->optBtnVisibleState(scheme);

    // Should return default constructed value (0)
    EXPECT_EQ(state, OptionButtonManager::kDoNotHide);
}

TEST_F(OptionButtonManagerTest, OptBtnVisibleState_DoNotHideState_ReturnsZero)
{
    QString scheme = "file";
    manager->setOptBtnVisibleState(scheme, OptionButtonManager::kDoNotHide);

    OptionButtonManager::OptBtnVisibleState state = manager->optBtnVisibleState(scheme);
    EXPECT_EQ(state, OptionButtonManager::kDoNotHide);
    EXPECT_EQ(state, 0x00);
}

TEST_F(OptionButtonManagerTest, OptBtnVisibleState_HideAllBtnState_ReturnsCorrectValue)
{
    QString scheme = "file";
    manager->setOptBtnVisibleState(scheme, OptionButtonManager::kHideAllBtn);

    OptionButtonManager::OptBtnVisibleState state = manager->optBtnVisibleState(scheme);
    EXPECT_EQ(state, OptionButtonManager::kHideAllBtn);
    // Verify kHideAllBtn is combination of flags
    EXPECT_EQ(state, (OptionButtonManager::kHideListViewBtn |
                      OptionButtonManager::kHideIconViewBtn |
                      OptionButtonManager::kHideDetailSpaceBtn |
                      OptionButtonManager::kHideTreeViewBtn));
}

TEST_F(OptionButtonManagerTest, OptBtnVisibleState_MultipleSchemes_ReturnsCorrectStates)
{
    manager->setOptBtnVisibleState("file", OptionButtonManager::kHideListViewBtn);
    manager->setOptBtnVisibleState("trash", OptionButtonManager::kHideIconViewBtn);
    manager->setOptBtnVisibleState("recent", OptionButtonManager::kDoNotHide);

    EXPECT_EQ(manager->optBtnVisibleState("file"), OptionButtonManager::kHideListViewBtn);
    EXPECT_EQ(manager->optBtnVisibleState("trash"), OptionButtonManager::kHideIconViewBtn);
    EXPECT_EQ(manager->optBtnVisibleState("recent"), OptionButtonManager::kDoNotHide);
}

TEST_F(OptionButtonManagerTest, HasVsibleState_ExistingScheme_ReturnsTrue)
{
    QString scheme = "file";
    manager->setOptBtnVisibleState(scheme, OptionButtonManager::kHideListViewBtn);

    EXPECT_TRUE(manager->hasVsibleState(scheme));
}

TEST_F(OptionButtonManagerTest, HasVsibleState_NonExistingScheme_ReturnsFalse)
{
    QString scheme = "nonexistent";
    EXPECT_FALSE(manager->hasVsibleState(scheme));
}

TEST_F(OptionButtonManagerTest, HasVsibleState_EmptyScheme_ReturnsFalse)
{
    QString scheme = "";
    EXPECT_FALSE(manager->hasVsibleState(scheme));
}

TEST_F(OptionButtonManagerTest, HasVsibleState_AfterSetAndRemove_ReturnsFalse)
{
    QString scheme = "file";
    manager->setOptBtnVisibleState(scheme, OptionButtonManager::kHideListViewBtn);
    EXPECT_TRUE(manager->hasVsibleState(scheme));

    manager->stateMap.remove(scheme);
    EXPECT_FALSE(manager->hasVsibleState(scheme));
}

TEST_F(OptionButtonManagerTest, HasVsibleState_MultipleSchemes_ReturnsCorrectResults)
{
    manager->setOptBtnVisibleState("file", OptionButtonManager::kHideListViewBtn);
    manager->setOptBtnVisibleState("trash", OptionButtonManager::kHideIconViewBtn);

    EXPECT_TRUE(manager->hasVsibleState("file"));
    EXPECT_TRUE(manager->hasVsibleState("trash"));
    EXPECT_FALSE(manager->hasVsibleState("recent"));
    EXPECT_FALSE(manager->hasVsibleState("computer"));
}

TEST_F(OptionButtonManagerTest, SetOptBtnVisibleState_SameSchemeMultipleTimes_OverwritesPrevious)
{
    QString scheme = "file";

    manager->setOptBtnVisibleState(scheme, OptionButtonManager::kHideListViewBtn);
    EXPECT_EQ(manager->optBtnVisibleState(scheme), OptionButtonManager::kHideListViewBtn);

    manager->setOptBtnVisibleState(scheme, OptionButtonManager::kHideIconViewBtn);
    EXPECT_EQ(manager->optBtnVisibleState(scheme), OptionButtonManager::kHideIconViewBtn);

    manager->setOptBtnVisibleState(scheme, OptionButtonManager::kHideAllBtn);
    EXPECT_EQ(manager->optBtnVisibleState(scheme), OptionButtonManager::kHideAllBtn);
}

TEST_F(OptionButtonManagerTest, StateMap_InitialState_IsEmpty)
{
    // After SetUp clears the map
    EXPECT_TRUE(manager->stateMap.isEmpty());
    EXPECT_EQ(manager->stateMap.size(), 0);
}

TEST_F(OptionButtonManagerTest, StateMap_AfterMultipleInserts_ContainsAllEntries)
{
    manager->setOptBtnVisibleState("file", OptionButtonManager::kHideListViewBtn);
    manager->setOptBtnVisibleState("trash", OptionButtonManager::kHideIconViewBtn);
    manager->setOptBtnVisibleState("recent", OptionButtonManager::kHideDetailSpaceBtn);
    manager->setOptBtnVisibleState("computer", OptionButtonManager::kHideTreeViewBtn);

    EXPECT_EQ(manager->stateMap.size(), 4);
    EXPECT_TRUE(manager->stateMap.contains("file"));
    EXPECT_TRUE(manager->stateMap.contains("trash"));
    EXPECT_TRUE(manager->stateMap.contains("recent"));
    EXPECT_TRUE(manager->stateMap.contains("computer"));
}

TEST_F(OptionButtonManagerTest, EnumValues_CheckBitmaskValues_CorrectValues)
{
    EXPECT_EQ(OptionButtonManager::kDoNotHide, 0x00);
    EXPECT_EQ(OptionButtonManager::kHideListViewBtn, 0x01);
    EXPECT_EQ(OptionButtonManager::kHideIconViewBtn, 0x02);
    EXPECT_EQ(OptionButtonManager::kHideDetailSpaceBtn, 0x04);
    EXPECT_EQ(OptionButtonManager::kHideTreeViewBtn, 0x08);
    EXPECT_EQ(OptionButtonManager::kHideListHeightOpt, 0x10);
}

TEST_F(OptionButtonManagerTest, EnumValues_kHideAllBtn_IsCombination)
{
    int expectedValue = OptionButtonManager::kHideListViewBtn |
                        OptionButtonManager::kHideIconViewBtn |
                        OptionButtonManager::kHideDetailSpaceBtn |
                        OptionButtonManager::kHideTreeViewBtn;

    EXPECT_EQ(OptionButtonManager::kHideAllBtn, expectedValue);
    EXPECT_EQ(OptionButtonManager::kHideAllBtn, 0x0F);
}

TEST_F(OptionButtonManagerTest, SetOptBtnVisibleState_SpecialSchemeNames_HandlesCorrectly)
{
    // Test with various scheme names
    manager->setOptBtnVisibleState("file:///", OptionButtonManager::kHideListViewBtn);
    manager->setOptBtnVisibleState("smb://", OptionButtonManager::kHideIconViewBtn);
    manager->setOptBtnVisibleState("ftp://", OptionButtonManager::kHideDetailSpaceBtn);
    manager->setOptBtnVisibleState("recent:///", OptionButtonManager::kHideTreeViewBtn);

    EXPECT_TRUE(manager->hasVsibleState("file:///"));
    EXPECT_TRUE(manager->hasVsibleState("smb://"));
    EXPECT_TRUE(manager->hasVsibleState("ftp://"));
    EXPECT_TRUE(manager->hasVsibleState("recent:///"));
}

TEST_F(OptionButtonManagerTest, SetOptBtnVisibleState_EmptyScheme_StoresCorrectly)
{
    QString emptyScheme = "";
    manager->setOptBtnVisibleState(emptyScheme, OptionButtonManager::kHideListViewBtn);

    EXPECT_TRUE(manager->hasVsibleState(emptyScheme));
    EXPECT_EQ(manager->optBtnVisibleState(emptyScheme), OptionButtonManager::kHideListViewBtn);
}

TEST_F(OptionButtonManagerTest, SetOptBtnVisibleState_LongSchemeName_HandlesCorrectly)
{
    QString longScheme = "verylongschemename_with_underscores_and_numbers_12345";
    manager->setOptBtnVisibleState(longScheme, OptionButtonManager::kHideAllBtn);

    EXPECT_TRUE(manager->hasVsibleState(longScheme));
    EXPECT_EQ(manager->optBtnVisibleState(longScheme), OptionButtonManager::kHideAllBtn);
}

TEST_F(OptionButtonManagerTest, Constructor_CreatesValidObject_IsQObject)
{
    EXPECT_NE(manager, nullptr);
    EXPECT_TRUE(qobject_cast<QObject*>(manager) != nullptr);
}

TEST_F(OptionButtonManagerTest, OptBtnVisibleState_CombinedStates_CanCheckIndividualFlags)
{
    QString scheme = "file";
    OptionButtonManager::OptBtnVisibleState combinedState =
        static_cast<OptionButtonManager::OptBtnVisibleState>(
            OptionButtonManager::kHideListViewBtn | OptionButtonManager::kHideIconViewBtn);

    manager->setOptBtnVisibleState(scheme, combinedState);
    OptionButtonManager::OptBtnVisibleState retrievedState = manager->optBtnVisibleState(scheme);

    // Check if individual flags are set
    EXPECT_TRUE(retrievedState & OptionButtonManager::kHideListViewBtn);
    EXPECT_TRUE(retrievedState & OptionButtonManager::kHideIconViewBtn);
    EXPECT_FALSE(retrievedState & OptionButtonManager::kHideDetailSpaceBtn);
    EXPECT_FALSE(retrievedState & OptionButtonManager::kHideTreeViewBtn);
}

TEST_F(OptionButtonManagerTest, OptBtnVisibleState_ZeroState_NotSetToAnyFlags)
{
    QString scheme = "file";
    manager->setOptBtnVisibleState(scheme, OptionButtonManager::kDoNotHide);
    OptionButtonManager::OptBtnVisibleState state = manager->optBtnVisibleState(scheme);

    EXPECT_FALSE(state & OptionButtonManager::kHideListViewBtn);
    EXPECT_FALSE(state & OptionButtonManager::kHideIconViewBtn);
    EXPECT_FALSE(state & OptionButtonManager::kHideDetailSpaceBtn);
    EXPECT_FALSE(state & OptionButtonManager::kHideTreeViewBtn);
    EXPECT_FALSE(state & OptionButtonManager::kHideListHeightOpt);
}

TEST_F(OptionButtonManagerTest, StateMap_DirectAccess_WorksCorrectly)
{
    // Test direct access to stateMap (via public member due to compiler flags)
    QString scheme = "file";
    OptionButtonManager::OptBtnVisibleState state = OptionButtonManager::kHideListViewBtn;

    manager->stateMap.insert(scheme, state);

    EXPECT_TRUE(manager->hasVsibleState(scheme));
    EXPECT_EQ(manager->optBtnVisibleState(scheme), state);
}

TEST_F(OptionButtonManagerTest, SetOptBtnVisibleState_StressTest_HandlesMultipleOperations)
{
    // Add many schemes
    for (int i = 0; i < 100; ++i) {
        QString scheme = QString("scheme_%1").arg(i);
        OptionButtonManager::OptBtnVisibleState state =
            static_cast<OptionButtonManager::OptBtnVisibleState>(i % 6);
        manager->setOptBtnVisibleState(scheme, state);
    }

    EXPECT_EQ(manager->stateMap.size(), 100);

    // Verify some entries
    EXPECT_TRUE(manager->hasVsibleState("scheme_0"));
    EXPECT_TRUE(manager->hasVsibleState("scheme_50"));
    EXPECT_TRUE(manager->hasVsibleState("scheme_99"));
    EXPECT_FALSE(manager->hasVsibleState("scheme_100"));
}
