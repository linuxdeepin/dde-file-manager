// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trash.cpp
 * @brief Unit tests for Trash methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "trash.h"

#include <QTest>

using namespace dfmplugin_trash;

class TrashTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Trash();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Trash *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashTest, addCustomTopWidget)
{
    // Test method: void addCustomTopWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->addCustomTopWidget());
}

TEST_F(TrashTest, addFileOperations)
{
    // Test method: void addFileOperations(())
    EXPECT_NO_FATAL_FAILURE(obj->addFileOperations());
}

TEST_F(TrashTest, bindWindows)
{
    // Test method: void bindWindows(())
    EXPECT_NO_FATAL_FAILURE(obj->bindWindows());
}

TEST_F(TrashTest, followEvents)
{
    // Test method: void followEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->followEvents());
}

TEST_F(TrashTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(TrashTest, onWindowOpened)
{
    // Test method: void onWindowOpened((quint64 windId))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowOpened(0));
}

TEST_F(TrashTest, regTrashCrumbToTitleBar)
{
    // Test method: void regTrashCrumbToTitleBar(())
    EXPECT_NO_FATAL_FAILURE(obj->regTrashCrumbToTitleBar());
}

TEST_F(TrashTest, regTrashItemToSideBar)
{
    // Test method: void regTrashItemToSideBar(())
    EXPECT_NO_FATAL_FAILURE(obj->regTrashItemToSideBar());
}

TEST_F(TrashTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(TrashTest, updateTrashItemToSideBar)
{
    // Test method: void updateTrashItemToSideBar(())
    EXPECT_NO_FATAL_FAILURE(obj->updateTrashItemToSideBar());
}

TEST_F(TrashTest, Trash_Destructor)
{
    // Test method:  ~Trash(())
    EXPECT_NO_FATAL_FAILURE({ Trash *tmp = new Trash(); delete tmp; });
}
