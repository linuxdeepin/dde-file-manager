// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileinfomodelbroker_1.cpp
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

TEST_F(FileInfoModelBrokerTest, FileInfoModelBroker)
{
    // Test constructor: FileInfoModelBroker((FileInfoModel *m, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileInfoModelBrokerTest, fileInfo)
{
    // Test method: FileInfoPointer fileInfo((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->fileInfo(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileInfoModelBrokerTest, indexUrl)
{
    // Test method: QUrl indexUrl((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->indexUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileInfoModelBrokerTest, init)
{
    // Test bool getter: init()
    bool result = obj->init();
    EXPECT_FALSE(result);

}

TEST_F(FileInfoModelBrokerTest, modelState)
{
    // Test getter: int modelState()
    auto result = obj->modelState();
    EXPECT_EQ(result, 0);

}

TEST_F(FileInfoModelBrokerTest, onDataReplaced)
{
    // Test method: void onDataReplaced((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onDataReplaced(_arg0, _arg1));
}

TEST_F(FileInfoModelBrokerTest, refresh)
{
    // Test method: void refresh((const QModelIndex &parent))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->refresh(_arg0));
}

TEST_F(FileInfoModelBrokerTest, rootIndex)
{
    // Test getter: QModelIndex rootIndex()
    auto result = obj->rootIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileInfoModelBrokerTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(FileInfoModelBrokerTest, updateFile)
{
    // Test method: void updateFile((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateFile(_arg0));
}

TEST_F(FileInfoModelBrokerTest, urlIndex)
{
    // Test method: QModelIndex urlIndex((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->urlIndex(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileInfoModelBrokerTest, FileInfoModelBroker_Destructor)
{
    // Test method:  ~FileInfoModelBroker(())
    EXPECT_NO_FATAL_FAILURE({ FileInfoModelBroker *tmp = new FileInfoModelBroker(); delete tmp; });
}
