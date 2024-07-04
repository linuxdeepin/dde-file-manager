// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/organizerutils.h"

#include "stubext.h"

#include <gtest/gtest.h>

DDP_ORGANIZER_USE_NAMESPACE

TEST(OrganizerUtils, isAllItemCategory)
{
    for (int i = kCatApplication; i <= kCatEnd; i = i << 1) {
        EXPECT_FALSE(OrganizerUtils::isAllItemCategory(static_cast<ItemCategory>(i)));
    }

    EXPECT_FALSE(OrganizerUtils::isAllItemCategory(kCatNone));
    EXPECT_TRUE(OrganizerUtils::isAllItemCategory(kCatDefault));
    auto all = ItemCategories(kCatApplication | kCatDocument | kCatPicture | kCatVideo | kCatMusic | kCatFolder);
    EXPECT_TRUE(OrganizerUtils::isAllItemCategory(all));
}
