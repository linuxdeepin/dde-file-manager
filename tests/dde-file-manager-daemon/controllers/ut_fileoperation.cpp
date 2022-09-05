// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "controllers/fileoperation.h"

namespace {
class TestFileOperation : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestFileOperation";
    }

    void TearDown() override
    {
        std::cout << "end TestFileOperation";
    }

public:
};
} // namespace
