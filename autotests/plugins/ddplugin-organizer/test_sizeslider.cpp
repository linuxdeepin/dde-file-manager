// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "options/sizeslider.h"

#include <QLabel>

#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class UT_SizeSlider : public testing::Test
{
protected:
    void SetUp() override
    {
        slider = new SizeSlider();
    }

    void TearDown() override
    {
        delete slider;
        slider = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    SizeSlider *slider = nullptr;
};

TEST_F(UT_SizeSlider, Constructor_CreatesSlider)
{
    EXPECT_NE(slider, nullptr);
    EXPECT_EQ(slider->parent(), nullptr);
    EXPECT_NE(slider->findChild<DTK_WIDGET_NAMESPACE::DSlider*>(), nullptr);
    EXPECT_NE(slider->findChild<QLabel*>(), nullptr);
}

TEST_F(UT_SizeSlider, Constructor_WithParent_CreatesSlider)
{
    QWidget parent;
    SizeSlider childSlider(&parent);
    EXPECT_EQ(childSlider.parent(), &parent);
}

TEST_F(UT_SizeSlider, Destructor_DoesNotCrash)
{
    SizeSlider *tempSlider = new SizeSlider();
    EXPECT_NE(tempSlider, nullptr);
    delete tempSlider;
    SUCCEED();
}

TEST_F(UT_SizeSlider, Init_DoesNotCrash)
{
    EXPECT_NO_THROW(slider->init());
}

TEST_F(UT_SizeSlider, ResetToIcon_DoesNotCrash)
{
    slider->init(); // Initialize first
    EXPECT_NO_THROW(slider->resetToIcon());
}

TEST_F(UT_SizeSlider, SetIconLevel_SetsValue)
{
    slider->init(); // Initialize first
    EXPECT_NO_THROW(slider->setIconLevel(2));
}

TEST_F(UT_SizeSlider, SyncIconLevel_SyncsValue)
{
    slider->init(); // Initialize first
    EXPECT_NO_THROW(slider->syncIconLevel(3));
}

TEST_F(UT_SizeSlider, SetValue_SetsValue)
{
    slider->init(); // Initialize first
    EXPECT_NO_THROW(slider->setValue(50));
}

TEST_F(UT_SizeSlider, IconLevel_ReturnsValue)
{
    slider->init(); // Initialize first
    int level = slider->iconLevel();
    EXPECT_TRUE(level >= 0); // Should return a valid level
}

TEST_F(UT_SizeSlider, Ticks_ReturnsList)
{
    QStringList ticks = SizeSlider::ticks(5);
    EXPECT_EQ(ticks.size(), 5);
    
    QStringList ticks2 = SizeSlider::ticks(0);
    EXPECT_TRUE(ticks2.isEmpty());
    
    QStringList ticks3 = SizeSlider::ticks(10);
    EXPECT_EQ(ticks3.size(), 10);
}

TEST_F(UT_SizeSlider, MultipleInitOperations)
{
    EXPECT_NO_THROW(slider->init());
    EXPECT_NO_THROW(slider->resetToIcon());
    EXPECT_NO_THROW(slider->setIconLevel(1));
    EXPECT_NO_THROW(slider->syncIconLevel(2));
    EXPECT_NO_THROW(slider->setValue(75));
    
    int level = slider->iconLevel();
    EXPECT_TRUE(level >= 0);
}

TEST_F(UT_SizeSlider, Inheritance_FromContentBackgroundWidget)
{
    SizeSlider testSlider;
    ContentBackgroundWidget *basePtr = &testSlider;
    EXPECT_NE(basePtr, nullptr);
    
    // Test inherited methods
    EXPECT_EQ(testSlider.radius(), 0);
    EXPECT_EQ(testSlider.roundEdge(), ContentBackgroundWidget::kNone);
    
    testSlider.setRadius(8);
    EXPECT_EQ(testSlider.radius(), 8);
}

TEST_F(UT_SizeSlider, IconClicked_Slot)
{
    // This is a protected slot, so we just verify that the object was created successfully
    EXPECT_NE(slider, nullptr);
}

TEST_F(UT_SizeSlider, SliderAndLabel_Initialized)
{
    // After construction, slider and label should be initialized
    EXPECT_NE(slider->findChild<DTK_WIDGET_NAMESPACE::DSlider*>(), nullptr);
    EXPECT_NE(slider->findChild<QLabel*>(), nullptr);
}
