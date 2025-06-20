// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    EXPECT_FALSE(type.categoryName.value(kCatFolder).isEmpty());
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

namespace {
inline const char kKeyCheckboxID[] = "CheckboxID";
}

TEST(TypeMethodGroup, build_all)
{
    TypeMethodGroup type;
    stub_ext::StubExt stub;
    ItemCategories enabled(kCatDefault);
    stub.set_lamda(&ConfigPresenter::enabledTypeCategories, [&enabled]() {
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
    EXPECT_TRUE(enabled.testFlag(kCatFolder));
}

TEST(TypeMethodGroup, build_allflags)
{
    TypeMethodGroup type;
    stub_ext::StubExt stub;
    ItemCategories enabled(kCatApplication
                           | kCatDocument | kCatPicture
                           | kCatVideo | kCatMusic | kCatFolder);
    stub.set_lamda(&ConfigPresenter::enabledTypeCategories, [&enabled]() {
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
    EXPECT_TRUE(enabled.testFlag(kCatFolder));
}

TEST(TypeMethodGroup, build_flag)
{
    TypeMethodGroup type;
    stub_ext::StubExt stub;
    ItemCategories enabled(kCatVideo);
    stub.set_lamda(&ConfigPresenter::enabledTypeCategories, [&enabled]() {
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
    EXPECT_FALSE(enabled.testFlag(kCatFolder));
}

TEST(TypeMethodGroup, subWidgets)
{
    CheckBoxWidget wid("");
    TypeMethodGroup type;
    type.categories << &wid;
    auto list = type.subWidgets();

    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list.first(), &wid);
    type.categories.clear();
}

TEST(TypeMethodGroup, onChenged)
{
    bool connect = false;
    stub_ext::StubExt stub;
    stub.set_lamda(&ConfigPresenter::switchToNormalized, [&connect](ConfigPresenter *self, int) {
        __DBG_STUB_INVOKE__
        connect = true;
    });
    TypeMethodGroup type;
    CheckBoxWidget *check = new CheckBoxWidget("temp_check");
    check->setProperty("CheckboxID", 1);
    QObject::connect(check, &CheckBoxWidget::changed, &type, &TypeMethodGroup::onChanged);
    check->changed(true);

    EXPECT_FALSE(connect);

    delete check;
}
