// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "app/filemanagerdaemon.h"

namespace {
class TestFileManagerDaemon : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestFileManagerDaemon";
        daemon = new FileManagerDaemon;
    }

    void TearDown() override
    {
        std::cout << "end TestFileManagerDaemon";
        delete daemon;
        daemon = nullptr;
    }

public:
    FileManagerDaemon *daemon = nullptr;
};
} // namespace

TEST_F(TestFileManagerDaemon, coverageTest)
{
    daemon->initControllers();
    daemon->initConnect();
}
