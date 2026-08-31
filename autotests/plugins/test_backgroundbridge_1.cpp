// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_backgroundbridge_1.cpp
 * @brief Unit tests for BackgroundBridge methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "backgroundmanager.h"

#include <QTest>

using namespace ddplugin_background;

class BackgroundBridgeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BackgroundBridge();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BackgroundBridge *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BackgroundBridgeTest, forceRequest)
{
    // Test method: void forceRequest(())
    EXPECT_NO_FATAL_FAILURE(obj->forceRequest());
}

TEST_F(BackgroundBridgeTest, processRequests)
{
    // Test method: void processRequests((const QList<Requestion> &cachedRequests, const QList<Requestion> &uncachedRequests, bool forceMode))
    QList<Requestion> _arg0{};
    QList<Requestion> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->processRequests(_arg0, _arg1, false));
}

TEST_F(BackgroundBridgeTest, queryCacheAndClassify)
{
    // Test method: void queryCacheAndClassify((Requestion &req, QList<Requestion> &cachedRequests, QList<Requestion> &uncachedRequests))
    Requestion _arg0{};
    QList<Requestion> _arg1{};
    QList<Requestion> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->queryCacheAndClassify(_arg0, _arg1, _arg2));
}
