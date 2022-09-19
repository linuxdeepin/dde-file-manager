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

#include "mode/normalized/type/typeclassifier_p.h"
#include "config/configpresenter.h"
#include "dfm-base/dfm_global_defines.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace ddplugin_organizer;

class TypeClassifierTest : public Test
{
public:
    virtual void SetUp() override {
        stub.set_lamda(&ConfigPresenter::enabledTypeCategories, [this](){
            return categories;
        });

    }
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
    ItemCategories categories = kCatAll;
};

TEST_F(TypeClassifierTest, classes_all)
{
    TypeClassifier obj;
    ASSERT_EQ(obj.d->categories, ItemCategories(kCatAll));

    auto list = obj.classes();
    QStringList expList = {QString("Type_Apps"),QString("Type_Documents"),
                           QString("Type_Pictures"),QString("Type_Videos"),
                           QString("Type_Music"), QString("Type_Folders"),
                            QString("Type_Other")};
    ASSERT_EQ(list.size(), expList.size());
    for (int i = 0; i < expList.size(); ++i)
        EXPECT_EQ(list[i], expList[i]);
}

TEST_F(TypeClassifierTest, classes_none)
{
    categories = kCatNone;
    TypeClassifier obj;
    ASSERT_EQ(obj.d->categories, ItemCategories(kCatNone));

    auto list = obj.classes();
    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list.first(), QString("Type_Other"));
}

TEST_F(TypeClassifierTest, classes_single)
{
    TypeClassifier obj;
    int index = 0;
    QStringList expList = {QString("Type_Apps"),QString("Type_Documents"),
                           QString("Type_Pictures"),QString("Type_Videos"),
                           QString("Type_Music"), QString("Type_Folders"),
                            };

    for (int i = kCatApplication; i <= kCatEnd; i = i << 1) {
        auto cat = static_cast<ItemCategory>(i);
        obj.d->categories = cat;
        auto list = obj.classes();
        ASSERT_LE(index, expList.size());
        EXPECT_EQ(list.size(), 2);
        if (list.size() == 2) {
            EXPECT_EQ(list.first(), expList[index]);
            EXPECT_EQ(list.last(), QString("Type_Other"));
        }
        index++;
    }
}
