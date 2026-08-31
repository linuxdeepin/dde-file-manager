// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticaleventreceiver_1.cpp
 * @brief Unit tests for OpticalEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/opticaleventreceiver.h"

#include <QTest>

using namespace dfmplugin_optical;

class OpticalEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalEventReceiverTest, OpticalEventReceiver)
{
    // Test constructor: OpticalEventReceiver((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OpticalEventReceiverTest, detailViewIcon)
{
    // Test method: bool detailViewIcon((const QUrl &url, QString *iconName))
    QUrl _arg0{};
    auto result = obj->detailViewIcon(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(OpticalEventReceiverTest, handleBlockShortcutPaste)
{
    // Test method: bool handleBlockShortcutPaste((quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->handleBlockShortcutPaste(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(OpticalEventReceiverTest, handleCutFilesShortcut)
{
    // Test method: bool handleCutFilesShortcut((quint64 winId, const QList<QUrl> &urls, const QUrl &rootUrl))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->handleCutFilesShortcut(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(OpticalEventReceiverTest, handleDropFiles)
{
    // Test method: bool handleDropFiles((const QList<QUrl> &fromUrls, const QUrl &toUrl))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->handleDropFiles(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(OpticalEventReceiverTest, handleMoveToTrashShortcut)
{
    // Test method: bool handleMoveToTrashShortcut((quint64 winId, const QList<QUrl> &urls, const QUrl &rootUrl))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->handleMoveToTrashShortcut(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(OpticalEventReceiverTest, handlePasteFilesShortcut)
{
    // Test method: bool handlePasteFilesShortcut((quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->handlePasteFilesShortcut(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(OpticalEventReceiverTest, handleTabCloseable)
{
    // Test method: bool handleTabCloseable((const QUrl &currentUrl, const QUrl &rootUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->handleTabCloseable(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(OpticalEventReceiverTest, instance)
{
    // Test getter: DFMBASE_USE_NAMESPACE instance()
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });
}

TEST_F(OpticalEventReceiverTest, isContainPWSubDirFile)
{
    // Test method: bool isContainPWSubDirFile((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->isContainPWSubDirFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OpticalEventReceiverTest, sepateTitlebarCrumb)
{
    // Test method: bool sepateTitlebarCrumb((const QUrl &url, QList<QVariantMap> *mapGroup))
    QUrl _arg0{};
    auto result = obj->sepateTitlebarCrumb(_arg0, nullptr);
    EXPECT_FALSE(result);

}
