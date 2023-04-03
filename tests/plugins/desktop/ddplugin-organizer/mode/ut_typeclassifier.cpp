// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mode/normalized/type/typeclassifier_p.h"
#include "config/configpresenter.h"
#include <dfm-base/dfm_global_defines.h>

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
    ItemCategories categories = kCatDefault;
};

TEST_F(TypeClassifierTest, classes_all)
{
    TypeClassifier obj;
    ASSERT_EQ(obj.d->categories, ItemCategories(kCatDefault));

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
