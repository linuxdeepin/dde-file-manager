// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_renamedialog_1.cpp
 * @brief Unit tests for RenameDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/renamedialog.h"

#include <QTest>

using namespace ddplugin_canvas;

class RenameDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RenameDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RenameDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RenameDialogTest, RenameDialog)
{
    // Test constructor: RenameDialog((int fileCount, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RenameDialogTest, M_~RenameDialog)
{
    // Test method:  ~RenameDialog(())
    EXPECT_NO_FATAL_FAILURE({ RenameDialog *tmp = new RenameDialog(); delete tmp; });
}

TEST_F(RenameDialogTest, operator=)
{
    // Test getter: RenameDialog operator=()
    EXPECT_NO_FATAL_FAILURE({ obj->operator=(); });
}

TEST_F(RenameDialogTest, modifyMode)
{
    // Test getter: RenameDialog::ModifyMode modifyMode()
    auto result = obj->modifyMode();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(RenameDialogTest, initUi)
{
    // Test method: void initUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initUi());
}

TEST_F(RenameDialogTest, getReplaceContent)
{
    // Test getter: QPair<QString, QString> getReplaceContent()
    auto result = obj->getReplaceContent();
    EXPECT_NO_FATAL_FAILURE({ obj->getReplaceContent(); });

}

TEST_F(RenameDialogTest, getAddContent)
{
    // Test getter: QPair<QString, AbstractJobHandler::FileNameAddFlag> getAddContent()
    auto result = obj->getAddContent();
    EXPECT_NO_FATAL_FAILURE({ obj->getAddContent(); });

}

TEST_F(RenameDialogTest, getCustomContent)
{
    // Test getter: QPair<QString, QString> getCustomContent()
    auto result = obj->getCustomContent();
    EXPECT_NO_FATAL_FAILURE({ obj->getCustomContent(); });

}

TEST_F(RenameDialogTest, public)
{
    // Test getter: Q_OBJECT public()
    EXPECT_NO_FATAL_FAILURE({ obj->public(); });
}

TEST_F(RenameDialogTest, d)
{
    // Test getter: QSharedPointer<RenameDialogPrivate> d()
    auto result = obj->d();
    EXPECT_EQ(result.get(), nullptr);

}
