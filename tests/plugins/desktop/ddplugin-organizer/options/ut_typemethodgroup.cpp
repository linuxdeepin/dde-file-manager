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

#include "organizer_defines.h"
#include "options/methodgroup/typemethodgroup.h"
#include "config/configpresenter.h"

#include "stubext.h"

#include <gtest/gtest.h>

DDP_ORGANIZER_USE_NAMESPACE

TEST(TypeMethodGroup, construct)
{
    TypeMethodGroup type;
    EXPECT_TRUE(type.categories.isEmpty());
    EXPECT_FALSE(type.categoryName.value(kCatApplication).isEmpty());
    EXPECT_FALSE(type.categoryName.value(kCatDocument).isEmpty());
    EXPECT_FALSE(type.categoryName.value(kCatPicture).isEmpty());
    EXPECT_FALSE(type.categoryName.value(kCatVideo).isEmpty());
    EXPECT_FALSE(type.categoryName.value(kCatMusic).isEmpty());
    EXPECT_FALSE(type.categoryName.value(kCatFloder).isEmpty());
}

TEST(TypeMethodGroup, id)
{
    TypeMethodGroup type;
    EXPECT_EQ(type.id(), kType);
}

TEST(TypeMethodGroup, build_none)
{
    CheckBoxWidget wid("");
    TypeMethodGroup type;
    type.categories << &wid;
    EXPECT_TRUE(type.build());

    ASSERT_EQ(type.categories.size(), 1);
    EXPECT_EQ(type.categories.first(), &wid);
    type.categories.clear();
}

namespace
{
    inline const char kKeyCheckboxID[] = "CheckboxID";
}

TEST(TypeMethodGroup, build_all)
{
    TypeMethodGroup type;
    stub_ext::StubExt stub;
    ItemCategories enabled(kCatAll);
    stub.set_lamda(&ConfigPresenter::enabledTypeCategories, [&enabled]()
    {
        return enabled;
    });

    type.build();
    EXPECT_EQ(type.categories.size(), 6);
    ItemCategories flags(kCatNone);

    for (CheckBoxWidget *box : type.categories) {
        EXPECT_TRUE(box->checked());
        int var = box->property(kKeyCheckboxID).toInt();
        auto flag = static_cast<ItemCategory>(var);
        flags |= flag;
    }
    EXPECT_TRUE(enabled.testFlag(kCatApplication));
    EXPECT_TRUE(enabled.testFlag(kCatDocument));
    EXPECT_TRUE(enabled.testFlag(kCatPicture));
    EXPECT_TRUE(enabled.testFlag(kCatVideo));
    EXPECT_TRUE(enabled.testFlag(kCatMusic));
    EXPECT_TRUE(enabled.testFlag(kCatFloder));
}

TEST(TypeMethodGroup, build_allflags)
{
    TypeMethodGroup type;
    stub_ext::StubExt stub;
    ItemCategories enabled(kCatApplication
                           | kCatDocument | kCatPicture
                           | kCatVideo | kCatMusic | kCatFloder
                           );
    stub.set_lamda(&ConfigPresenter::enabledTypeCategories, [&enabled]()
    {
        return enabled;
    });

    type.build();
    EXPECT_EQ(type.categories.size(), 6);
    ItemCategories flags(kCatNone);

    for (CheckBoxWidget *box : type.categories) {
        EXPECT_TRUE(box->checked());
        int var = box->property(kKeyCheckboxID).toInt();
        auto flag = static_cast<ItemCategory>(var);
        flags |= flag;
    }
    EXPECT_TRUE(enabled.testFlag(kCatApplication));
    EXPECT_TRUE(enabled.testFlag(kCatDocument));
    EXPECT_TRUE(enabled.testFlag(kCatPicture));
    EXPECT_TRUE(enabled.testFlag(kCatVideo));
    EXPECT_TRUE(enabled.testFlag(kCatMusic));
    EXPECT_TRUE(enabled.testFlag(kCatFloder));
}

TEST(TypeMethodGroup, build_flag)
{
    TypeMethodGroup type;
    stub_ext::StubExt stub;
    ItemCategories enabled(kCatVideo);
    stub.set_lamda(&ConfigPresenter::enabledTypeCategories, [&enabled]()
    {
        return enabled;
    });

    type.build();
    EXPECT_EQ(type.categories.size(), 6);
    ItemCategories flags(kCatNone);

    for (CheckBoxWidget *box : type.categories) {
        bool checked = box->checked();

        int var = box->property(kKeyCheckboxID).toInt();
        auto flag = static_cast<ItemCategory>(var);
        EXPECT_EQ(checked, flag == kCatVideo);
        if (checked)
            flags |= flag;
    }

    EXPECT_FALSE(enabled.testFlag(kCatApplication));
    EXPECT_FALSE(enabled.testFlag(kCatDocument));
    EXPECT_FALSE(enabled.testFlag(kCatPicture));
    EXPECT_TRUE(enabled.testFlag(kCatVideo));
    EXPECT_FALSE(enabled.testFlag(kCatMusic));
    EXPECT_FALSE(enabled.testFlag(kCatFloder));
}

TEST(TypeMethodGroup, subWidgets)
{
    CheckBoxWidget wid("");
    TypeMethodGroup type;
    type.categories << &wid;
    auto list =  type.subWidgets();

    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list.first(), &wid);
    type.categories.clear();
}
