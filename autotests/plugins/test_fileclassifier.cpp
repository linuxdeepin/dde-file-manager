// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileclassifier.cpp
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

TEST_F(FileClassifierTest, append)
{
    // Test method: QString append((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->append(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileClassifierTest, baseData)
{
    // Test method: CollectionBaseDataPtr baseData((const QString &key))
    QString _arg0{};
    auto result = obj->baseData(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileClassifierTest, change)
{
    // Test method: QString change((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->change(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileClassifierTest, insert)
{
    // Test method: void insert((const QUrl &, const QString &, const int))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->insert(_arg0, _arg1, {}));
}

TEST_F(FileClassifierTest, prepend)
{
    // Test method: QString prepend((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->prepend(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileClassifierTest, remove)
{
    // Test method: QString remove((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->remove(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileClassifierTest, reset)
{
    // Test method: void reset((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->reset(_arg0));
}

TEST_F(FileClassifierTest, FileClassifier)
{
    // Test constructor: FileClassifier((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
