// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/private/renamedialog_p.h"

#include "stubext.h"

#include <gtest/gtest.h>

DDP_CANVAS_USE_NAMESPACE

TEST(RenameDialog, construct)
{
    RenameDialog rd(998);
    ASSERT_NE(rd.d->titleLabel, nullptr);
    auto texts = rd.d->titleLabel->text().split(" ");
    ASSERT_GT(texts.size(), 2);
    EXPECT_EQ(texts.at(1), QString::number(998));
}



