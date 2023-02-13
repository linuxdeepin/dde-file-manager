// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "options/organizationgroup.h"
#include "config/configpresenter.h"
#include "options/methodgroup/typemethodgroup.h"

#include "stubext.h"

#include <gtest/gtest.h>

DDP_ORGANIZER_USE_NAMESPACE

TEST(OrganizationGroup, construct)
{
    OrganizationGroup og;
    ASSERT_NE(og.contentLayout, nullptr);
    EXPECT_EQ(og.contentLayout->sizeConstraint(), QLayout::SetFixedSize);
}

TEST(OrganizationGroup, reset_on)
{
    OrganizationGroup og;
    stub_ext::StubExt stub;
    stub.set_lamda(&ConfigPresenter::isEnable, [](){
        return true;
    });
    stub.set_lamda(&ConfigPresenter::mode, [](){
        return kNormalized;
    });

    stub.set_lamda(&ConfigPresenter::classification, [](){
        return kType;
    });

    stub.set_lamda(VADDR(TypeMethodGroup, build), [](){
        return true;
    });

    og.reset();

    ASSERT_NE(og.organizationSwitch, nullptr);
    EXPECT_TRUE(og.organizationSwitch->checked());

    ASSERT_NE(og.methodCombox, nullptr);
    ASSERT_NE(og.currentClass, nullptr);
    EXPECT_EQ(og.currentClass->id(), kType);
}

TEST(OrganizationGroup, reset_off)
{
    OrganizationGroup og;
    stub_ext::StubExt stub;
    stub.set_lamda(&ConfigPresenter::isEnable, [](){
        return false;
    });
    stub.set_lamda(&ConfigPresenter::mode, [](){
        return kNormalized;
    });

    stub.set_lamda(&ConfigPresenter::classification, [](){
        return kType;
    });

    stub.set_lamda(VADDR(TypeMethodGroup, build), [](){
        return true;
    });

    og.reset();

    ASSERT_NE(og.organizationSwitch, nullptr);
    EXPECT_FALSE(og.organizationSwitch->checked());

    EXPECT_EQ(og.methodCombox, nullptr);
    EXPECT_EQ(og.currentClass, nullptr);
}
