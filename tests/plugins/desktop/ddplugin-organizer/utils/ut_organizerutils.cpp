/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
