// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/organizerutils.h"

#include "stubext.h"

#include <gtest/gtest.h>

DDP_ORGANIZER_USE_NAMESPACE

TEST(OrganizerUtils, covertIconLevel)
{
    EXPECT_EQ(OrganizerUtils::covertIconLevel(0, true), (int)DisplaySize::kSmaller);
    EXPECT_EQ(OrganizerUtils::covertIconLevel(1, true), (int)DisplaySize::kSmaller);
    EXPECT_EQ(OrganizerUtils::covertIconLevel(2, true), (int)DisplaySize::kNormal);
    EXPECT_EQ(OrganizerUtils::covertIconLevel(3, true), (int)DisplaySize::kLarger);
    EXPECT_EQ(OrganizerUtils::covertIconLevel(4, true), (int)DisplaySize::kLarger);

    EXPECT_EQ(OrganizerUtils::covertIconLevel((int)DisplaySize::kSmaller, false), 1);
    EXPECT_EQ(OrganizerUtils::covertIconLevel((int)DisplaySize::kNormal, false), 2);
    EXPECT_EQ(OrganizerUtils::covertIconLevel((int)DisplaySize::kLarger, false), 3);
    EXPECT_EQ(OrganizerUtils::covertIconLevel(0, false), 1);
    EXPECT_EQ(OrganizerUtils::covertIconLevel(3, false), 1);
}

TEST(OrganizerUtils, isAllItemCategory)
{
    for (int i = kCatApplication; i <= kCatEnd; i = i << 1) {
        EXPECT_FALSE(OrganizerUtils::isAllItemCategory(static_cast<ItemCategory>(i)));
    }

    EXPECT_FALSE(OrganizerUtils::isAllItemCategory(kCatNone));
    EXPECT_TRUE(OrganizerUtils::isAllItemCategory(kCatDefault));
    auto all = ItemCategories(kCatApplication| kCatDocument | kCatPicture | kCatVideo | kCatMusic | kCatFloder);
    EXPECT_TRUE(OrganizerUtils::isAllItemCategory(all));
}
