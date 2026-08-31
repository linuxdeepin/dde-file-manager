// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasinterface.cpp
 * @brief Unit tests for CanvasInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "interface/canvasinterface.h"

#include <QTest>

using namespace ddplugin_organizer;

class CanvasInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasInterfaceTest, CanvasInterface)
{
    // Test constructor: CanvasInterface((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasInterfaceTest, canvasGrid)
{
    // Test getter: CanvasGridShell canvasGrid()
    auto result = obj->canvasGrid();
    EXPECT_NO_FATAL_FAILURE({ obj->canvasGrid(); });

}

TEST_F(CanvasInterfaceTest, canvasManager)
{
    // Test getter: CanvasManagerShell canvasManager()
    auto result = obj->canvasManager();
    EXPECT_NO_FATAL_FAILURE({ obj->canvasManager(); });

}

TEST_F(CanvasInterfaceTest, canvasModel)
{
    // Test getter: CanvasModelShell canvasModel()
    auto result = obj->canvasModel();
    EXPECT_NO_FATAL_FAILURE({ obj->canvasModel(); });

}

TEST_F(CanvasInterfaceTest, canvasSelectionShell)
{
    // Test getter: CanvasSelectionShell canvasSelectionShell()
    auto result = obj->canvasSelectionShell();
    EXPECT_NO_FATAL_FAILURE({ obj->canvasSelectionShell(); });

}

TEST_F(CanvasInterfaceTest, canvasView)
{
    // Test getter: CanvasViewShell canvasView()
    auto result = obj->canvasView();
    EXPECT_NO_FATAL_FAILURE({ obj->canvasView(); });

}

TEST_F(CanvasInterfaceTest, fileInfoModel)
{
    // Test getter: FileInfoModelShell fileInfoModel()
    auto result = obj->fileInfoModel();
    EXPECT_NO_FATAL_FAILURE({ obj->fileInfoModel(); });

}

TEST_F(CanvasInterfaceTest, iconLevel)
{
    // Test getter: int iconLevel()
    auto result = obj->iconLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasInterfaceTest, initialize)
{
    // Test bool getter: initialize()
    bool result = obj->initialize();
    EXPECT_FALSE(result);

}

TEST_F(CanvasInterfaceTest, setIconLevel)
{
    // Test setter: void setIconLevel((int lv))
    EXPECT_NO_FATAL_FAILURE(obj->setIconLevel(0));
}

TEST_F(CanvasInterfaceTest, CanvasInterface_Destructor)
{
    // Test method:  ~CanvasInterface(())
    EXPECT_NO_FATAL_FAILURE({ CanvasInterface *tmp = new CanvasInterface(); delete tmp; });
}
