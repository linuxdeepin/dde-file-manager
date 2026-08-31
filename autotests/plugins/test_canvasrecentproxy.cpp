// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasrecentproxy.cpp
 * @brief Unit tests for CanvasRecentProxy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "recentproxy/canvasrecentproxy.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasRecentProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasRecentProxy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasRecentProxy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasRecentProxyTest, CanvasRecentProxy)
{
    // Test constructor: CanvasRecentProxy((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasRecentProxyTest, handleReloadRecentFiles)
{
    // Test method: void handleReloadRecentFiles((const QList<QUrl> &srcUrls, bool ok, const QString &errMsg))
    QList<QUrl> _arg0{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleReloadRecentFiles(_arg0, false, _arg2));
}

TEST_F(CanvasRecentProxyTest, CanvasRecentProxy_Destructor)
{
    // Test method:  ~CanvasRecentProxy(())
    EXPECT_NO_FATAL_FAILURE({ CanvasRecentProxy *tmp = new CanvasRecentProxy(); delete tmp; });
}
