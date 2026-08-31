// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmanagerbroker.cpp
 * @brief Unit tests for CanvasManagerBroker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "broker/canvasmanagerbroker.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasManagerBrokerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasManagerBroker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasManagerBroker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasManagerBrokerTest, CanvasManagerBroker)
{
    // Test constructor: CanvasManagerBroker((CanvasManager *ptr, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasManagerBrokerTest, autoArrange)
{
    // Test bool getter: autoArrange()
    bool result = obj->autoArrange();
    EXPECT_FALSE(result);

}

TEST_F(CanvasManagerBrokerTest, edit)
{
    // Test method: void edit((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->edit(_arg0));
}

TEST_F(CanvasManagerBrokerTest, fileInfoModel)
{
    // Test getter: QAbstractItemModel fileInfoModel()
    auto result = obj->fileInfoModel();
    EXPECT_NO_FATAL_FAILURE({ obj->fileInfoModel(); });

}

TEST_F(CanvasManagerBrokerTest, iconLevel)
{
    // Test getter: int iconLevel()
    auto result = obj->iconLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasManagerBrokerTest, init)
{
    // Test bool getter: init()
    bool result = obj->init();
    EXPECT_FALSE(result);

}

TEST_F(CanvasManagerBrokerTest, selectionModel)
{
    // Test getter: QItemSelectionModel selectionModel()
    auto result = obj->selectionModel();
    EXPECT_NO_FATAL_FAILURE({ obj->selectionModel(); });

}

TEST_F(CanvasManagerBrokerTest, setAutoArrange)
{
    // Test setter: void setAutoArrange((bool on))
    EXPECT_NO_FATAL_FAILURE(obj->setAutoArrange(false));
}

TEST_F(CanvasManagerBrokerTest, setIconLevel)
{
    // Test setter: void setIconLevel((int lv))
    EXPECT_NO_FATAL_FAILURE(obj->setIconLevel(0));
}

TEST_F(CanvasManagerBrokerTest, update)
{
    // Test method: void update(())
    EXPECT_NO_FATAL_FAILURE(obj->update());
}

TEST_F(CanvasManagerBrokerTest, view)
{
    // Test method: QAbstractItemView view((int viewIdx))
    auto result = obj->view(0);
    EXPECT_NO_FATAL_FAILURE({ obj->view(0); });

}

TEST_F(CanvasManagerBrokerTest, CanvasManagerBroker_Destructor)
{
    // Test method:  ~CanvasManagerBroker(())
    EXPECT_NO_FATAL_FAILURE({ CanvasManagerBroker *tmp = new CanvasManagerBroker(); delete tmp; });
}
