// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "models/completerviewmodel.h"

#include "stubext.h"

#include <gtest/gtest.h>

DPTITLEBAR_USE_NAMESPACE

TEST(CompleterViewModelTest, ut_setStringList)
{
    CompleterViewModel model;
    model.setStringList(QStringList() << "1"
                                      << "");
    EXPECT_EQ(1, model.rowCount());
}
