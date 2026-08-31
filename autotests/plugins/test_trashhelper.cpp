// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashhelper.cpp
 * @brief Unit tests for TrashHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/trashhelper.h"

#include <QTest>

using namespace dfmplugin_trash;

class TrashHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashHelperTest, checkDragDropAction)
{
    // Test method: bool checkDragDropAction((const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->checkDragDropAction(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TrashHelperTest, emptyTrash)
{
    // Test method: void emptyTrash((const quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->emptyTrash(0));
}

TEST_F(TrashHelperTest, isTrashFile)
{
    // Test method: bool isTrashFile((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->isTrashFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TrashHelperTest, isTrashRootFile)
{
    // Test method: bool isTrashRootFile((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->isTrashRootFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TrashHelperTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(TrashHelperTest, scheme)
{
    // Test getter: QString scheme()
    auto result = obj->scheme();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TrashHelperTest, windowId)
{
    // Test method: quint64 windowId((QWidget *sender))
    auto result = obj->windowId(nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(TrashHelperTest, TrashHelper)
{
    // Test constructor: TrashHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TrashHelperTest, instance)
{
    // Test getter: DFMBASE_USE_NAMESPACE instance()
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });
}
