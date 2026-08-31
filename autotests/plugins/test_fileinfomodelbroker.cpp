// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileinfomodelbroker.cpp
 * @brief Unit tests for FileInfoModelBroker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "broker/fileinfomodelbroker.h"

#include <QTest>

using namespace ddplugin_canvas;

class FileInfoModelBrokerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileInfoModelBroker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileInfoModelBroker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileInfoModelBrokerTest, files)
{
    // Test getter: QList<QUrl> files()
    auto result = obj->files();
    EXPECT_TRUE(result.isEmpty());

}
