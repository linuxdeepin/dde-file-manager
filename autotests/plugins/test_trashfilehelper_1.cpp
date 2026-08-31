// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashfilehelper_1.cpp
 * @brief Unit tests for TrashFileHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/trashfilehelper.h"

#include <QTest>

using namespace dfmplugin_trash;

class TrashFileHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashFileHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashFileHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashFileHelperTest, TrashFileHelper)
{
    // Test constructor: TrashFileHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TrashFileHelperTest, blockPaste)
{
    // Test method: bool blockPaste((quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->blockPaste(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(TrashFileHelperTest, copyFile)
{
    // Test method: bool copyFile((const quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags))
    auto result = obj->copyFile(0, QList<QUrl>(), QUrl(), DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(TrashFileHelperTest, cutFile)
{
    // Test method: bool cutFile((const quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags))
    auto result = obj->cutFile(0, QList<QUrl>(), QUrl(), DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(TrashFileHelperTest, disableOpenWidgetWidget)
{
    // Test method: bool disableOpenWidgetWidget((const QUrl &url, bool *result))
    QUrl _arg0{};
    auto result = obj->disableOpenWidgetWidget(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TrashFileHelperTest, handleCanTag)
{
    // Test method: bool handleCanTag((const QUrl &url, bool *canTag))
    QUrl _arg0{};
    auto result = obj->handleCanTag(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TrashFileHelperTest, handleIsSubFile)
{
    // Test method: bool handleIsSubFile((const QUrl &parent, const QUrl &sub))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->handleIsSubFile(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TrashFileHelperTest, handleNotCdComputer)
{
    // Test method: bool handleNotCdComputer((const QUrl &url, QUrl *cdUrl))
    QUrl _arg0{};
    auto result = obj->handleNotCdComputer(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TrashFileHelperTest, hookCopyFilePath)
{
    // Test method: bool hookCopyFilePath((quint64 windowId, const QList<QUrl> &urlList, const QUrl &rootUrl))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->hookCopyFilePath(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(TrashFileHelperTest, moveToTrash)
{
    // Test method: bool moveToTrash((const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags))
    auto result = obj->moveToTrash(0, QList<QUrl>(), DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}
