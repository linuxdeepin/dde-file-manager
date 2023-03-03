// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "organizer_defines.h"
#include "options/methodgroup/methodgrouphelper.h"
#include "options/methodgroup/typemethodgroup.h"

#include "stubext.h"

#include <gtest/gtest.h>

DDP_ORGANIZER_USE_NAMESPACE

TEST(MethodGroupHelper, create)
{
    auto obj = MethodGroupHelper::create(Classifier::kType);
    ASSERT_NE(obj, nullptr);

    auto cast = qobject_cast<TypeMethodGroup *>(obj);
    EXPECT_NE(cast, nullptr);
}
