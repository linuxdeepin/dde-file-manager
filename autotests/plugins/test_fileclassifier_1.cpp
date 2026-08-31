// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileclassifier_1.cpp
 * @brief Unit tests for FileClassifier methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/normalized/fileclassifier.h"

#include <QTest>

using namespace ddplugin_organizer;

class FileClassifierTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileClassifier();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileClassifier *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileClassifierTest, acceptInsert)
{
    // Test method: bool acceptInsert((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->acceptInsert(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileClassifierTest, acceptRename)
{
    // Test method: bool acceptRename((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->acceptRename(_arg0, _arg1);
    EXPECT_FALSE(result);

}
