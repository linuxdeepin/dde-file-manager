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
