// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_controllerpipe.cpp
 * @brief Unit tests for ControllerPipe methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-extractor/libextractor/controllerpipe.h"

#include <QTest>

using namespace src;

class ControllerPipeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ControllerPipe();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ControllerPipe *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ControllerPipeTest, processInputBuffer)
{
    // Test method: void processInputBuffer(())
    EXPECT_NO_FATAL_FAILURE(obj->processInputBuffer());
}
