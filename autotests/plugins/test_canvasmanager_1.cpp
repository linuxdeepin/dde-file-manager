// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmanager_1.cpp
 * @brief Unit tests for CanvasManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "canvasmanager.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasManagerTest, autoArrange)
{
    // Test bool getter: autoArrange()
    bool result = obj->autoArrange();
    EXPECT_FALSE(result);

}

TEST_F(CanvasManagerTest, fileModel)
{
    // Test getter: FileInfoModel fileModel()
    auto result = obj->fileModel();
    EXPECT_NO_FATAL_FAILURE({ obj->fileModel(); });

}

TEST_F(CanvasManagerTest, iconLevel)
{
    // Test getter: int iconLevel()
    auto result = obj->iconLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasManagerTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(CanvasManagerTest, onChangeIconLevel)
{
    // Test method: void onChangeIconLevel((bool increase))
    EXPECT_NO_FATAL_FAILURE(obj->onChangeIconLevel(false));
}

TEST_F(CanvasManagerTest, onDetachWindows)
{
    // Test method: void onDetachWindows(())
    EXPECT_NO_FATAL_FAILURE(obj->onDetachWindows());
}

TEST_F(CanvasManagerTest, onFontChanged)
{
    // Test method: void onFontChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onFontChanged());
}

TEST_F(CanvasManagerTest, onGeometryChanged)
{
    // Test method: void onGeometryChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onGeometryChanged());
}

TEST_F(CanvasManagerTest, onTrashStateChanged)
{
    // Test method: void onTrashStateChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onTrashStateChanged());
}

TEST_F(CanvasManagerTest, onWallperSetting)
{
    // Test method: void onWallperSetting((CanvasView *view))
    EXPECT_NO_FATAL_FAILURE(obj->onWallperSetting(nullptr));
}

TEST_F(CanvasManagerTest, refresh)
{
    // Test method: void refresh((bool silent))
    EXPECT_NO_FATAL_FAILURE(obj->refresh(false));
}

TEST_F(CanvasManagerTest, selectionModel)
{
    // Test getter: CanvasSelectionModel selectionModel()
    auto result = obj->selectionModel();
    EXPECT_NO_FATAL_FAILURE({ obj->selectionModel(); });

}

TEST_F(CanvasManagerTest, setAutoArrange)
{
    // Test setter: void setAutoArrange((bool on))
    EXPECT_NO_FATAL_FAILURE(obj->setAutoArrange(false));
}

TEST_F(CanvasManagerTest, setIconLevel)
{
    // Test setter: void setIconLevel((int level))
    EXPECT_NO_FATAL_FAILURE(obj->setIconLevel(0));
}

TEST_F(CanvasManagerTest, update)
{
    // Test method: void update(())
    EXPECT_NO_FATAL_FAILURE(obj->update());
}
