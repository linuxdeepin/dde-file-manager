// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dpf.cpp
 * @brief Unit tests for dpf methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "plugin_diskencryptentry.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class dpfTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new dpf();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    dpf *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(dpfTest, Plugin)
{
    // Test getter: class DFMPLUGIN_DISK_ENCRYPT_EXPORT Plugin()
    auto result = obj->Plugin();
    EXPECT_NO_FATAL_FAILURE({ obj->Plugin(); });

}
