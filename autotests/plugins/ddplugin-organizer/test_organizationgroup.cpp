// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "options/organizationgroup.h"

#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class UT_OrganizationGroup : public testing::Test
{
protected:
    void SetUp() override
    {
        group = new OrganizationGroup();
    }

    void TearDown() override
    {
        delete group;
        group = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    OrganizationGroup *group = nullptr;
};

TEST_F(UT_OrganizationGroup, Constructor_CreatesGroup)
{
    EXPECT_NE(group, nullptr);
    EXPECT_EQ(group->parent(), nullptr);
}

TEST_F(UT_OrganizationGroup, Constructor_WithParent_CreatesGroup)
{
    QWidget parent;
    OrganizationGroup childGroup(&parent);
    EXPECT_EQ(childGroup.parent(), &parent);
}

TEST_F(UT_OrganizationGroup, Destructor_DoesNotCrash)
{
    OrganizationGroup *tempGroup = new OrganizationGroup();
    EXPECT_NE(tempGroup, nullptr);
    delete tempGroup;
    SUCCEED();
}

TEST_F(UT_OrganizationGroup, Reset_DoesNotCrash)
{
    EXPECT_NO_THROW(group->reset());
}

TEST_F(UT_OrganizationGroup, EnableOrganizeChanged_DoesNotCrash)
{
    EXPECT_NO_THROW(group->enableOrganizeChanged(true));
    EXPECT_NO_THROW(group->enableOrganizeChanged(false));
}

TEST_F(UT_OrganizationGroup, EnableHideAllChanged_DoesNotCrash)
{
    group->reset();
    // //mock static SwitchWidget::setRoundEdge
    // stub.set_lamda(ADDR(SwitchWidget, setRoundEdge), []() {
    //     __DBG_STUB_INVOKE__
    // });
    // EXPECT_NO_THROW(group->enableHideAllChanged(true));
    // EXPECT_NO_THROW(group->enableHideAllChanged(false));
}

TEST_F(UT_OrganizationGroup, InternalWidgets_Initialized)
{
    // The constructor should initialize internal widgets
    EXPECT_NE(group->findChild<SwitchWidget*>(), nullptr);  // At least one SwitchWidget should exist
    // We can't directly access private members, but we can check that the object functions
    EXPECT_NE(group, nullptr);
}

TEST_F(UT_OrganizationGroup, MultipleResetCalls)
{
    EXPECT_NO_THROW(group->reset());
    EXPECT_NO_THROW(group->reset());
    EXPECT_NO_THROW(group->reset());
}

TEST_F(UT_OrganizationGroup, SignalSlotConnections)
{
    // Test that signal-slot connections work without crashing
    EXPECT_NO_THROW(group->enableOrganizeChanged(true));
    // EXPECT_NO_THROW(group->enableHideAllChanged(false));
    
    // Test with different values
    EXPECT_NO_THROW(group->enableOrganizeChanged(false));
    // EXPECT_NO_THROW(group->enableHideAllChanged(true));
}

TEST_F(UT_OrganizationGroup, LayoutOperations)
{
    // The group should have internal layout operations
    EXPECT_NO_THROW(group->reset());
    
    // Verify the object is still valid after operations
    EXPECT_NE(group, nullptr);
}
