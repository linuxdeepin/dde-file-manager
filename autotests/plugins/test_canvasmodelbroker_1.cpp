// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmodelbroker_1.cpp
 * @brief Unit tests for CanvasModelBroker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "broker/canvasmodelbroker.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasModelBrokerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasModelBroker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasModelBroker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasModelBrokerTest, CanvasModelBroker)
{
    // Test constructor: CanvasModelBroker((CanvasProxyModel *m, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasModelBrokerTest, data)
{
    // Test method: QVariant data((const QUrl &url, int itemRole))
    QUrl _arg0{};
    auto result = obj->data(_arg0, 0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasModelBrokerTest, fetch)
{
    // Test method: bool fetch((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->fetch(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelBrokerTest, fileUrl)
{
    // Test method: QUrl fileUrl((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->fileUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasModelBrokerTest, files)
{
    // Test getter: QList<QUrl> files()
    auto result = obj->files();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasModelBrokerTest, init)
{
    // Test bool getter: init()
    bool result = obj->init();
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelBrokerTest, refresh)
{
    // Test method: void refresh((bool global, int ms, bool updateFile))
    EXPECT_NO_FATAL_FAILURE(obj->refresh(false, 0, false));
}

TEST_F(CanvasModelBrokerTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(CanvasModelBrokerTest, rowCount)
{
    // Test getter: int rowCount()
    auto result = obj->rowCount();
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasModelBrokerTest, setShowHiddenFiles)
{
    // Test setter: void setShowHiddenFiles((bool show))
    EXPECT_NO_FATAL_FAILURE(obj->setShowHiddenFiles(false));
}

TEST_F(CanvasModelBrokerTest, setSortOrder)
{
    // Test setter: void setSortOrder((int order))
    EXPECT_NO_FATAL_FAILURE(obj->setSortOrder(0));
}

TEST_F(CanvasModelBrokerTest, setSortRole)
{
    // Test setter: void setSortRole((int role, int order))
    EXPECT_NO_FATAL_FAILURE(obj->setSortRole(0, 0));
}

TEST_F(CanvasModelBrokerTest, showHiddenFiles)
{
    // Test bool getter: showHiddenFiles()
    bool result = obj->showHiddenFiles();
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelBrokerTest, sort)
{
    // Test method: void sort(())
    EXPECT_NO_FATAL_FAILURE(obj->sort());
}

TEST_F(CanvasModelBrokerTest, sortOrder)
{
    // Test getter: int sortOrder()
    auto result = obj->sortOrder();
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasModelBrokerTest, sortRole)
{
    // Test getter: int sortRole()
    auto result = obj->sortRole();
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasModelBrokerTest, take)
{
    // Test method: bool take((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->take(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelBrokerTest, urlIndex)
{
    // Test method: QModelIndex urlIndex((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->urlIndex(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasModelBrokerTest, CanvasModelBroker_Destructor)
{
    // Test method:  ~CanvasModelBroker(())
    EXPECT_NO_FATAL_FAILURE({ CanvasModelBroker *tmp = new CanvasModelBroker(); delete tmp; });
}
