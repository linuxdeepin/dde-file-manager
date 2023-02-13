// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "organizer_defines.h"
#include "options/methodgroup/methodcombox.h"

#include "stubext.h"

#include <gtest/gtest.h>

DDP_ORGANIZER_USE_NAMESPACE

TEST(MethodComBox, construct)
{
    const QString title = "xxxx";
    MethodComBox box(title);
    ASSERT_NE(box.label, nullptr);
    ASSERT_NE(box.comboBox, nullptr);
    EXPECT_EQ(box.label->text(), title);
}
