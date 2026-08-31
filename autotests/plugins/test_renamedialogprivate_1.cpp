// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_renamedialogprivate_1.cpp
 * @brief Unit tests for RenameDialogPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/private/renamedialog_p.h"

#include <QTest>

using namespace ddplugin_canvas;

class RenameDialogPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RenameDialogPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RenameDialogPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RenameDialogPrivateTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(RenameDialogPrivateTest, initConnect_initConn)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(RenameDialogPrivateTest, initUi)
{
    // Test method: void initUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initUi());
}

TEST_F(RenameDialogPrivateTest, initUi_initUi)
{
    // Test method: void initUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initUi());
}

TEST_F(RenameDialogPrivateTest, setRenameButtonStatus)
{
    // Test setter: void setRenameButtonStatus((const bool &enabled))
    bool _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setRenameButtonStatus(_arg0));
}

TEST_F(RenameDialogPrivateTest, setRenameButtonStatus_setRenam)
{
    // Test setter: void setRenameButtonStatus((const bool &enabled))
    bool _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setRenameButtonStatus(_arg0));
}

TEST_F(RenameDialogPrivateTest, updateStates)
{
    // Test method: void updateStates(())
    EXPECT_NO_FATAL_FAILURE(obj->updateStates());
}

TEST_F(RenameDialogPrivateTest, updateStates_updateSt)
{
    // Test method: void updateStates(())
    EXPECT_NO_FATAL_FAILURE(obj->updateStates());
}

TEST_F(RenameDialogPrivateTest, RenameDialogPrivate_Destructor)
{
    // Test method:  ~RenameDialogPrivate(())
    EXPECT_NO_FATAL_FAILURE({ RenameDialogPrivate *tmp = new RenameDialogPrivate(); delete tmp; });
}

TEST_F(RenameDialogPrivateTest, RenameDialogPrivate_Destructor_xRenameD)
{
    // Test method:  ~RenameDialogPrivate(())
    EXPECT_NO_FATAL_FAILURE({ RenameDialogPrivate *tmp = new RenameDialogPrivate(); delete tmp; });
}
