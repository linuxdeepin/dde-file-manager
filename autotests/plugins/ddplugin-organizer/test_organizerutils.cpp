// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/organizerutils.h"
#include "organizer_defines.h"

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

class UT_OrganizerUtils : public testing::Test
{
protected:
    void SetUp() override
    {
        // No special setup needed for static utility class
    }

    void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_OrganizerUtils, TestIsAllItemCategory)
{
    // Test with kCatAll flag
    EXPECT_TRUE(OrganizerUtils::isAllItemCategory(kCatAll));
    
    // Test with other categories
    EXPECT_FALSE(OrganizerUtils::isAllItemCategory(kCatDefault));
    EXPECT_FALSE(OrganizerUtils::isAllItemCategory(kCatApplication));
    EXPECT_FALSE(OrganizerUtils::isAllItemCategory(kCatDocument));
    EXPECT_FALSE(OrganizerUtils::isAllItemCategory(kCatVideo));
    EXPECT_FALSE(OrganizerUtils::isAllItemCategory(kCatMusic));
    EXPECT_FALSE(OrganizerUtils::isAllItemCategory(kCatPicture));
    EXPECT_FALSE(OrganizerUtils::isAllItemCategory(kCatOther));
    
    // Test with combined flags (assuming kCatAll is not a combination)
    ItemCategories combined = static_cast<ItemCategories>(kCatApplication | kCatDocument);
    EXPECT_FALSE(OrganizerUtils::isAllItemCategory(combined));
    
    // Test with zero
    EXPECT_FALSE(OrganizerUtils::isAllItemCategory(static_cast<ItemCategories>(0)));
    
    // Test with negative value
    EXPECT_FALSE(OrganizerUtils::isAllItemCategory(static_cast<ItemCategories>(-1)));
}

TEST_F(UT_OrganizerUtils, TestBuildBitwiseEnabledCategory)
{
    // Test with kCatDefault flag
    ItemCategories result = OrganizerUtils::buildBitwiseEnabledCategory(kCatDefault);
    ItemCategories expected = kCatAll;
    expected &= ~kCatOther;
    expected &= ~kCatApplication;
    EXPECT_EQ(result, expected);
    
    // Test with negative value
    ItemCategories negativeValue = static_cast<ItemCategories>(-1);
    result = OrganizerUtils::buildBitwiseEnabledCategory(negativeValue);
    expected &= ~kCatOther;
    expected &= ~kCatApplication;
    EXPECT_EQ(result, expected);
    
    // Test with zero (might be treated as 0)
    result = OrganizerUtils::buildBitwiseEnabledCategory(static_cast<ItemCategories>(0));
    expected &= ~kCatOther;
    expected &= ~kCatApplication;
    EXPECT_EQ(result, expected);
    
    // Test with valid individual categories
    result = OrganizerUtils::buildBitwiseEnabledCategory(kCatApplication);
    EXPECT_EQ(result, kCatApplication);
    
    result = OrganizerUtils::buildBitwiseEnabledCategory(kCatDocument);
    EXPECT_EQ(result, kCatDocument);
    
    result = OrganizerUtils::buildBitwiseEnabledCategory(kCatVideo);
    EXPECT_EQ(result, kCatVideo);
    
    result = OrganizerUtils::buildBitwiseEnabledCategory(kCatMusic);
    EXPECT_EQ(result, kCatMusic);
    
    result = OrganizerUtils::buildBitwiseEnabledCategory(kCatPicture);
    EXPECT_EQ(result, kCatPicture);
    
    result = OrganizerUtils::buildBitwiseEnabledCategory(kCatOther);
    EXPECT_EQ(result, kCatOther);
    
    // Test with kCatAll (should remain unchanged)
    result = OrganizerUtils::buildBitwiseEnabledCategory(kCatAll);
    EXPECT_EQ(result, kCatAll);
    
    // Test with combined categories
    ItemCategories combined = static_cast<ItemCategories>(kCatApplication | kCatDocument | kCatVideo);
    result = OrganizerUtils::buildBitwiseEnabledCategory(combined);
    EXPECT_EQ(result, combined);
}

TEST_F(UT_OrganizerUtils, TestEdgeCases)
{
    // Test with large positive values
    ItemCategories largeValue = static_cast<ItemCategories>(0x7FFFFFFF);
    ItemCategories result = OrganizerUtils::buildBitwiseEnabledCategory(largeValue);
    EXPECT_EQ(result, largeValue);
    EXPECT_TRUE(OrganizerUtils::isAllItemCategory(largeValue));  // Unless kCatAll is defined differently
    
    // Test with boundary values
    ItemCategories expected = kCatAll;
    result = OrganizerUtils::buildBitwiseEnabledCategory(static_cast<ItemCategories>(-100));
    expected &= ~kCatOther;
    expected &= ~kCatApplication;
    EXPECT_EQ(result, expected);
    
    // Test with various negative values
    for (int i = -10; i < 0; ++i) {
        result = OrganizerUtils::buildBitwiseEnabledCategory(static_cast<ItemCategories>(i));
        EXPECT_EQ(result, expected);
    }
}

TEST_F(UT_OrganizerUtils, TestBitwiseOperations)
{
    // Test that bitwise operations work correctly
    ItemCategories base = OrganizerUtils::buildBitwiseEnabledCategory(kCatDefault);
    EXPECT_NE(base, 0);  // Should not be zero after operations
    
    // Test that kCatOther and kCatApplication are excluded from default
    EXPECT_FALSE(base & kCatOther);
    EXPECT_FALSE(base & kCatApplication);
    
    // Test that other categories are included
    EXPECT_TRUE(base & kCatDocument);
    EXPECT_TRUE(base & kCatVideo);
    EXPECT_TRUE(base & kCatMusic);
    EXPECT_TRUE(base & kCatPicture);
}

TEST_F(UT_OrganizerUtils, TestConsistency)
{
    // Test that the same input produces the same output
    ItemCategories result1 = OrganizerUtils::buildBitwiseEnabledCategory(kCatDefault);
    ItemCategories result2 = OrganizerUtils::buildBitwiseEnabledCategory(kCatDefault);
    EXPECT_EQ(result1, result2);
    
    ItemCategories result3 = OrganizerUtils::buildBitwiseEnabledCategory(kCatApplication);
    ItemCategories result4 = OrganizerUtils::buildBitwiseEnabledCategory(kCatApplication);
    EXPECT_EQ(result3, result4);
    
    // Test idempotence of isAllItemCategory
    bool allCheck1 = OrganizerUtils::isAllItemCategory(kCatAll);
    bool allCheck2 = OrganizerUtils::isAllItemCategory(kCatAll);
    EXPECT_EQ(allCheck1, allCheck2);
    EXPECT_TRUE(allCheck1);
}

TEST_F(UT_OrganizerUtils, TestMultipleCategories)
{
    // Test building with multiple categories
    ItemCategories multiCat = static_cast<ItemCategories>(kCatDocument | kCatVideo | kCatMusic);
    ItemCategories result = OrganizerUtils::buildBitwiseEnabledCategory(multiCat);
    EXPECT_EQ(result, multiCat);
    
    // Test adding kCatOther to multi-category
    multiCat |= kCatOther;
    result = OrganizerUtils::buildBitwiseEnabledCategory(multiCat);
    EXPECT_EQ(result, multiCat);
    
    // Test that isAllItemCategory returns false for multi-category (unless it matches kCatAll)
    EXPECT_FALSE(OrganizerUtils::isAllItemCategory(multiCat));
}

TEST_F(UT_OrganizerUtils, TestDefaultBehavior)
{
    // The function should handle various input scenarios gracefully
    std::vector<ItemCategories> testValues = {
        kCatDefault,
        static_cast<ItemCategories>(-1),
        static_cast<ItemCategories>(0),
        kCatApplication,
        kCatDocument,
        kCatVideo,
        kCatMusic,
        kCatPicture,
        kCatOther,
        kCatAll,
        static_cast<ItemCategories>(kCatApplication | kCatDocument),
        static_cast<ItemCategories>(kCatVideo | kCatMusic | kCatPicture),
        static_cast<ItemCategories>(0xFFFFFFFF)  // All bits set
    };
    
    for (ItemCategories value : testValues) {
        // Should not crash for any of these values
        EXPECT_NO_THROW(OrganizerUtils::isAllItemCategory(value));
        EXPECT_NO_THROW(OrganizerUtils::buildBitwiseEnabledCategory(value));
        
        // The result should be a valid ItemCategories value
        ItemCategories result = OrganizerUtils::buildBitwiseEnabledCategory(value);
        EXPECT_GE(result, 0) << "Failed for value: " << value;
    }
}
