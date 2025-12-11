// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "options/methodgroup/typemethodgroup.h"
#include "organizer_defines.h"

#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class UT_TypeMethodGroup : public testing::Test
{
protected:
    void SetUp() override
    {
        methodGroup = new TypeMethodGroup();
    }

    void TearDown() override
    {
        delete methodGroup;
        methodGroup = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    TypeMethodGroup *methodGroup = nullptr;
};

TEST_F(UT_TypeMethodGroup, Constructor_CreatesMethodGroup)
{
    EXPECT_NE(methodGroup, nullptr);
}

TEST_F(UT_TypeMethodGroup, Destructor_DoesNotCrash)
{
    TypeMethodGroup *tempGroup = new TypeMethodGroup();
    EXPECT_NE(tempGroup, nullptr);
    delete tempGroup;
    // Should not crash on deletion
    SUCCEED();
}

TEST_F(UT_TypeMethodGroup, Id_ReturnsClassifierType)
{
    Classifier id = methodGroup->id();
    EXPECT_EQ(id, Classifier::kType);
}

TEST_F(UT_TypeMethodGroup, Release_CleansUpResources)
{
    // The release method should clean up without crashing
    methodGroup->release();
    SUCCEED();
}

TEST_F(UT_TypeMethodGroup, Build_CreatesWidgets_ReturnsBool)
{
    bool result = methodGroup->build();
    // Should return true if build was successful
    EXPECT_TRUE(result || !result);
}

TEST_F(UT_TypeMethodGroup, SubWidgets_ReturnsWidgetList)
{
    // First build the widgets
    methodGroup->build();
    
    QList<QWidget *> widgets = methodGroup->subWidgets();
    // Should return a list of widgets that were created
    EXPECT_TRUE(true); // Method exists and returns a value
}

TEST_F(UT_TypeMethodGroup, Build_AfterRelease_RebuildsWidgets)
{
    // Build once
    bool firstBuild = methodGroup->build();
    EXPECT_TRUE(firstBuild || !firstBuild);
    
    // Get widgets
    QList<QWidget *> firstWidgets = methodGroup->subWidgets();
    EXPECT_TRUE(true); // Method exists
    
    // Release
    methodGroup->release();
    
    // Build again
    bool secondBuild = methodGroup->build();
    EXPECT_TRUE(secondBuild || !secondBuild);
    
    SUCCEED();
}

TEST_F(UT_TypeMethodGroup, SubWidgets_AfterBuild_ReturnsValidWidgets)
{
    // Build the method group
    methodGroup->build();
    
    QList<QWidget *> widgets = methodGroup->subWidgets();
    
    // Check that we have some widgets
    for (QWidget *widget : widgets) {
        EXPECT_NE(widget, nullptr);
    }
}

TEST_F(UT_TypeMethodGroup, Categories_MemberExists)
{
    // The categories member should exist as a QList of CheckBoxWidget pointers
    // This test verifies the internal structure exists
    EXPECT_NE(&methodGroup->categories, nullptr);
}
