// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_viewdefines.cpp - ViewDefines class unit tests
// Using stub_ext for function stubbing

#include <gtest/gtest.h>
#include <QTest>
#include <QVariantList>
#include <QList>

// Include stub_ext
#include "stubext.h"

// Include test target classes
#include <dfm-base/utils/viewdefines.h>

using namespace dfmbase;

/**
 * @brief ViewDefines class unit tests
 *
 * Test scope:
 * 1. Constructor and initialization
 * 2. Icon size list operations
 * 3. Icon grid density operations
 * 4. List height operations
 * 5. Utility function operations
 * 6. Boundary conditions and error handling
 * 7. Private function testing
 */
class ViewDefinesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        stub.clear();
        viewDefines = new ViewDefines();
    }

    void TearDown() override
    {
        // Clean up test environment
        stub.clear();
        delete viewDefines;
        viewDefines = nullptr;
    }

    // Test stubbing utility
    stub_ext::StubExt stub;
    ViewDefines *viewDefines = nullptr;
    
    // Helper function to create a test ViewDefines instance
    ViewDefines* createTestViewDefines() {
        return new ViewDefines();
    }
};

/**
 * @brief Test constructor and initialization
 * Verify that ViewDefines is properly initialized
 */
TEST_F(ViewDefinesTest, Constructor_Basic)
{
    // Create new instance
    ViewDefines *testViewDefines = createTestViewDefines();
    EXPECT_NE(testViewDefines, nullptr);
    
    // Verify initialization - all lists should be populated
    EXPECT_GT(testViewDefines->iconSizeCount(), 0);
    EXPECT_GT(testViewDefines->iconGridDensityCount(), 0);
    EXPECT_GT(testViewDefines->listHeightCount(), 0);
    
    delete testViewDefines;
}

/**
 * @brief Test icon size count functionality
 * Verify iconSizeCount returns correct number
 */
TEST_F(ViewDefinesTest, IconSizeCount_Correct)
{
    int count = viewDefines->iconSizeCount();
    EXPECT_GT(count, 0);
    
    // Verify the count matches expected size based on initialization logic
    // Min=24, Middle=192, NormalStep=8: (192-24)/8 + 1 = 22
    // Middle=192, Max=512, LargeStep=16: (512-192)/16 = 20
    // Total should be 22 + 20 = 42
    EXPECT_EQ(count, 42);
}

/**
 * @brief Test icon size retrieval by index
 * Verify iconSize returns correct values
 */
TEST_F(ViewDefinesTest, IconSize_ByIndex)
{
    // Test first element
    int firstSize = viewDefines->iconSize(0);
    EXPECT_EQ(firstSize, kIconSizeMin);
    
    // Test some intermediate values
    EXPECT_EQ(viewDefines->iconSize(1), kIconSizeMin + kIconSizeNormalStep);
    EXPECT_EQ(viewDefines->iconSize(10), kIconSizeMin + (10 * kIconSizeNormalStep));
    
    // Test the transition point around kIconSizeMiddle
    int middleIndex = viewDefines->indexOfIconSize(kIconSizeMiddle);
    EXPECT_NE(middleIndex, -1);
    EXPECT_EQ(viewDefines->iconSize(middleIndex), kIconSizeMiddle);
    
    // Test last element
    int lastIndex = viewDefines->iconSizeCount() - 1;
    int lastSize = viewDefines->iconSize(lastIndex);
    EXPECT_EQ(lastSize, kIconSizeMax);
}

/**
 * @brief Test icon size index lookup
 * Verify indexOfIconSize returns correct indices
 */
TEST_F(ViewDefinesTest, IndexOfIconSize_Lookup)
{
    // Test valid sizes
    EXPECT_EQ(viewDefines->indexOfIconSize(kIconSizeMin), 0);
    EXPECT_EQ(viewDefines->indexOfIconSize(kIconSizeMiddle), viewDefines->iconSizeCount() - 21); // Middle is at position 21 (0-based)
    EXPECT_EQ(viewDefines->indexOfIconSize(kIconSizeMax), viewDefines->iconSizeCount() - 1);
    
    // Test invalid size
    EXPECT_EQ(viewDefines->indexOfIconSize(-1), -1);
    EXPECT_EQ(viewDefines->indexOfIconSize(0), -1);
    EXPECT_EQ(viewDefines->indexOfIconSize(kIconSizeMax + 1), -1);
}

/**
 * @brief Test icon size list conversion
 * Verify getIconSizeList returns proper QVariantList
 */
TEST_F(ViewDefinesTest, GetIconSizeList_Conversion)
{
    QVariantList sizeList = viewDefines->getIconSizeList();
    EXPECT_EQ(sizeList.size(), viewDefines->iconSizeCount());
    
    // Verify all elements are strings representing numbers
    for (const QVariant &variant : sizeList) {
        EXPECT_TRUE(variant.canConvert<QString>());
        QString str = variant.toString();
        EXPECT_FALSE(str.isEmpty());
        EXPECT_TRUE(str.toInt() > 0);
    }
    
    // Verify first and last elements
    EXPECT_EQ(sizeList.first().toString(), QString::number(kIconSizeMin));
    EXPECT_EQ(sizeList.last().toString(), QString::number(kIconSizeMax));
}

/**
 * @brief Test icon grid density count functionality
 * Verify iconGridDensityCount returns correct number
 */
TEST_F(ViewDefinesTest, IconGridDensityCount_Correct)
{
    int count = viewDefines->iconGridDensityCount();
    EXPECT_GT(count, 0);
    
    // Calculate expected count: (Max - Min) / Step + 1
    // (198 - 60) / 6 + 1 = 138 / 6 + 1 = 23 + 1 = 24
    int expectedCount = (kIconGridDensityMax - kIconGridDensityMin) / kIconGridDensityStep + 1;
    EXPECT_EQ(count, expectedCount);
}

/**
 * @brief Test icon grid density retrieval by index
 * Verify iconGridDensity returns correct values
 */
TEST_F(ViewDefinesTest, IconGridDensity_ByIndex)
{
    // Test first element
    int firstDensity = viewDefines->iconGridDensity(0);
    EXPECT_EQ(firstDensity, kIconGridDensityMin);
    
    // Test some intermediate values
    EXPECT_EQ(viewDefines->iconGridDensity(1), kIconGridDensityMin + kIconGridDensityStep);
    EXPECT_EQ(viewDefines->iconGridDensity(10), kIconGridDensityMin + (10 * kIconGridDensityStep));
    
    // Test last element
    int lastIndex = viewDefines->iconGridDensityCount() - 1;
    int lastDensity = viewDefines->iconGridDensity(lastIndex);
    EXPECT_EQ(lastDensity, kIconGridDensityMax);
}

/**
 * @brief Test icon grid density index lookup
 * Verify indexOfIconGridDensity returns correct indices
 */
TEST_F(ViewDefinesTest, IndexOfIconGridDensity_Lookup)
{
    // Test valid densities
    EXPECT_EQ(viewDefines->indexOfIconGridDensity(kIconGridDensityMin), 0);
    EXPECT_EQ(viewDefines->indexOfIconGridDensity(kIconGridDensityMax), viewDefines->iconGridDensityCount() - 1);
    
    // Test some intermediate values
    EXPECT_EQ(viewDefines->indexOfIconGridDensity(kIconGridDensityMin + kIconGridDensityStep), 1);
    
    // Test invalid density
    EXPECT_EQ(viewDefines->indexOfIconGridDensity(-1), -1);
    EXPECT_EQ(viewDefines->indexOfIconGridDensity(kIconGridDensityMin - 1), -1);
    EXPECT_EQ(viewDefines->indexOfIconGridDensity(kIconGridDensityMax + 1), -1);
}

/**
 * @brief Test icon grid density list conversion
 * Verify getIconGridDensityList returns proper QVariantList
 */
TEST_F(ViewDefinesTest, GetIconGridDensityList_Conversion)
{
    QVariantList densityList = viewDefines->getIconGridDensityList();
    EXPECT_EQ(densityList.size(), viewDefines->iconGridDensityCount());
    
    // Verify all elements are strings representing numbers
    for (const QVariant &variant : densityList) {
        EXPECT_TRUE(variant.canConvert<QString>());
        QString str = variant.toString();
        EXPECT_FALSE(str.isEmpty());
        EXPECT_TRUE(str.toInt() > 0);
    }
    
    // Verify first and last elements
    EXPECT_EQ(densityList.first().toString(), QString::number(kIconGridDensityMin));
    EXPECT_EQ(densityList.last().toString(), QString::number(kIconGridDensityMax));
}

/**
 * @brief Test list height count functionality
 * Verify listHeightCount returns correct number
 */
TEST_F(ViewDefinesTest, ListHeightCount_Correct)
{
    int count = viewDefines->listHeightCount();
    EXPECT_EQ(count, 3); // Should be exactly 3 as per initialization: {24, 32, 48}
}

/**
 * @brief Test list height retrieval by index
 * Verify listHeight returns correct values
 */
TEST_F(ViewDefinesTest, ListHeight_ByIndex)
{
    // Test all three values based on initialization {24, 32, 48}
    EXPECT_EQ(viewDefines->listHeight(0), 24);
    EXPECT_EQ(viewDefines->listHeight(1), 32);
    EXPECT_EQ(viewDefines->listHeight(2), 48);
}

/**
 * @brief Test list height index lookup
 * Verify indexOfListHeight returns correct indices
 */
TEST_F(ViewDefinesTest, IndexOfListHeight_Lookup)
{
    // Test valid heights
    EXPECT_EQ(viewDefines->indexOfListHeight(24), 0);
    EXPECT_EQ(viewDefines->indexOfListHeight(32), 1);
    EXPECT_EQ(viewDefines->indexOfListHeight(48), 2);
    
    // Test invalid heights
    EXPECT_EQ(viewDefines->indexOfListHeight(-1), -1);
    EXPECT_EQ(viewDefines->indexOfListHeight(0), -1);
    EXPECT_EQ(viewDefines->indexOfListHeight(100), -1);
}

/**
 * @brief Test list height list conversion
 * Verify getListHeightList returns proper QVariantList
 */
TEST_F(ViewDefinesTest, GetListHeightList_Conversion)
{
    QVariantList heightList = viewDefines->getListHeightList();
    EXPECT_EQ(heightList.size(), viewDefines->listHeightCount());
    EXPECT_EQ(heightList.size(), 3);
    
    // Verify all elements are strings representing numbers
    for (const QVariant &variant : heightList) {
        EXPECT_TRUE(variant.canConvert<QString>());
        QString str = variant.toString();
        EXPECT_FALSE(str.isEmpty());
        EXPECT_TRUE(str.toInt() > 0);
    }
    
    // Verify the three specific values
    EXPECT_EQ(heightList.at(0).toString(), "24");
    EXPECT_EQ(heightList.at(1).toString(), "32");
    EXPECT_EQ(heightList.at(2).toString(), "48");
}

/**
 * @brief Test boundary conditions for icon size
 * Verify behavior at boundary values
 */
TEST_F(ViewDefinesTest, IconSize_BoundaryConditions)
{
    // Test minimum valid index
    EXPECT_NO_THROW(viewDefines->iconSize(0));
    EXPECT_EQ(viewDefines->iconSize(0), kIconSizeMin);
    
    // Test maximum valid index
    int maxIndex = viewDefines->iconSizeCount() - 1;
    EXPECT_NO_THROW(viewDefines->iconSize(maxIndex));
    EXPECT_EQ(viewDefines->iconSize(maxIndex), kIconSizeMax);
}

/**
 * @brief Test boundary conditions for icon grid density
 * Verify behavior at boundary values
 */
TEST_F(ViewDefinesTest, IconGridDensity_BoundaryConditions)
{
    // Test minimum valid index
    EXPECT_NO_THROW(viewDefines->iconGridDensity(0));
    EXPECT_EQ(viewDefines->iconGridDensity(0), kIconGridDensityMin);
    
    // Test maximum valid index
    int maxIndex = viewDefines->iconGridDensityCount() - 1;
    EXPECT_NO_THROW(viewDefines->iconGridDensity(maxIndex));
    EXPECT_EQ(viewDefines->iconGridDensity(maxIndex), kIconGridDensityMax);
}

/**
 * @brief Test boundary conditions for list height
 * Verify behavior at boundary values
 */
TEST_F(ViewDefinesTest, ListHeight_BoundaryConditions)
{
    // Test minimum valid index
    EXPECT_NO_THROW(viewDefines->listHeight(0));
    EXPECT_EQ(viewDefines->listHeight(0), 24);
    
    // Test maximum valid index (should be 2 since there are 3 elements)
    EXPECT_NO_THROW(viewDefines->listHeight(2));
    EXPECT_EQ(viewDefines->listHeight(2), 48);
}

/**
 * @brief Test private initDefines function indirectly
 * Verify that initialization creates expected values
 */
TEST_F(ViewDefinesTest, InitDefines_IndirectVerification)
{
    // Create new instance to test initialization
    ViewDefines *newViewDefines = createTestViewDefines();
    
    // Verify icon size initialization pattern
    // Check first few values (normal step pattern)
    EXPECT_EQ(newViewDefines->iconSize(0), 24);
    EXPECT_EQ(newViewDefines->iconSize(1), 32);  // 24 + 8
    EXPECT_EQ(newViewDefines->iconSize(2), 40);  // 32 + 8
    
    // Check values around the middle transition
    int middleIndex = newViewDefines->indexOfIconSize(kIconSizeMiddle);
    if (middleIndex > 0 && middleIndex < newViewDefines->iconSizeCount() - 1) {
        int afterMiddle = newViewDefines->iconSize(middleIndex + 1);
        EXPECT_EQ(afterMiddle, kIconSizeMiddle + kIconSizeLargeStep); // Should use large step after middle
    }
    
    // Verify icon grid density initialization pattern
    EXPECT_EQ(newViewDefines->iconGridDensity(0), kIconGridDensityMin);
    EXPECT_EQ(newViewDefines->iconGridDensity(1), kIconGridDensityMin + kIconGridDensityStep);
    
    // Verify list height initialization
    EXPECT_EQ(newViewDefines->listHeight(0), 24);
    EXPECT_EQ(newViewDefines->listHeight(1), 32);
    EXPECT_EQ(newViewDefines->listHeight(2), 48);
    
    delete newViewDefines;
}

/**
 * @brief Test transToVariantList utility function
 * Verify the conversion from QList<int> to QVariantList works correctly
 */
TEST_F(ViewDefinesTest, TransToVariantList_UtilityFunction)
{
    // We can't directly test the private function, but we can test its behavior
    // through the public functions that use it
    
    QVariantList iconSizes = viewDefines->getIconSizeList();
    QVariantList densities = viewDefines->getIconGridDensityList();
    QVariantList heights = viewDefines->getListHeightList();
    
    // Verify all lists are properly converted to string representations
    for (const QVariantList &list : {iconSizes, densities, heights}) {
        EXPECT_FALSE(list.isEmpty());
        for (const QVariant &variant : list) {
            EXPECT_TRUE(variant.canConvert<QString>());
            QString str = variant.toString();
            EXPECT_FALSE(str.isEmpty());
            bool ok;
            int num = str.toInt(&ok);
            EXPECT_TRUE(ok);
            EXPECT_GT(num, 0);
        }
    }
}

/**
 * @brief Test empty and invalid parameters
 * Verify graceful handling of edge cases
 */
TEST_F(ViewDefinesTest, EdgeCase_Handling)
{
    // Test lookup of non-existent values
    EXPECT_EQ(viewDefines->indexOfIconSize(-100), -1);
    EXPECT_EQ(viewDefines->indexOfIconSize(1000), -1);
    EXPECT_EQ(viewDefines->indexOfIconGridDensity(-100), -1);
    EXPECT_EQ(viewDefines->indexOfIconGridDensity(1000), -1);
    EXPECT_EQ(viewDefines->indexOfListHeight(-100), -1);
    EXPECT_EQ(viewDefines->indexOfListHeight(1000), -1);
    
    // Test that count functions never return negative
    EXPECT_GE(viewDefines->iconSizeCount(), 0);
    EXPECT_GE(viewDefines->iconGridDensityCount(), 0);
    EXPECT_GE(viewDefines->listHeightCount(), 0);
}

/**
 * @brief Test consistency between different functions
 * Verify that related functions return consistent results
 */
TEST_F(ViewDefinesTest, FunctionConsistency)
{
    // Test consistency between count and index functions
    int iconSizeCount = viewDefines->iconSizeCount();
    for (int i = 0; i < iconSizeCount; ++i) {
        int size = viewDefines->iconSize(i);
        int index = viewDefines->indexOfIconSize(size);
        EXPECT_EQ(index, i);
    }
    
    // Test consistency for icon grid density
    int densityCount = viewDefines->iconGridDensityCount();
    for (int i = 0; i < densityCount; ++i) {
        int density = viewDefines->iconGridDensity(i);
        int index = viewDefines->indexOfIconGridDensity(density);
        EXPECT_EQ(index, i);
    }
    
    // Test consistency for list height
    int heightCount = viewDefines->listHeightCount();
    for (int i = 0; i < heightCount; ++i) {
        int height = viewDefines->listHeight(i);
        int index = viewDefines->indexOfListHeight(height);
        EXPECT_EQ(index, i);
    }
}

/**
 * @brief Test multiple instances consistency
 * Verify that multiple instances behave identically
 */
TEST_F(ViewDefinesTest, MultipleInstances_Consistency)
{
    ViewDefines *instance1 = createTestViewDefines();
    ViewDefines *instance2 = createTestViewDefines();
    
    // Test that both instances have same counts
    EXPECT_EQ(instance1->iconSizeCount(), instance2->iconSizeCount());
    EXPECT_EQ(instance1->iconGridDensityCount(), instance2->iconGridDensityCount());
    EXPECT_EQ(instance1->listHeightCount(), instance2->listHeightCount());
    
    // Test that both instances have same values
    for (int i = 0; i < instance1->iconSizeCount(); ++i) {
        EXPECT_EQ(instance1->iconSize(i), instance2->iconSize(i));
    }
    
    for (int i = 0; i < instance1->iconGridDensityCount(); ++i) {
        EXPECT_EQ(instance1->iconGridDensity(i), instance2->iconGridDensity(i));
    }
    
    for (int i = 0; i < instance1->listHeightCount(); ++i) {
        EXPECT_EQ(instance1->listHeight(i), instance2->listHeight(i));
    }
    
    delete instance1;
    delete instance2;
}
