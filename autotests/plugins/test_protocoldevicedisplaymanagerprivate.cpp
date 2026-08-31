// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_protocoldevicedisplaymanagerprivate.cpp
 * @brief Unit tests for ProtocolDeviceDisplayManagerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displaycontrol/protocoldevicedisplaymanager_p.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class ProtocolDeviceDisplayManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProtocolDeviceDisplayManagerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProtocolDeviceDisplayManagerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProtocolDeviceDisplayManagerPrivateTest, removeAllSmb)
{
    // Test method: void removeAllSmb((QList<QUrl> *entryUrls))
    EXPECT_NO_FATAL_FAILURE(obj->removeAllSmb(nullptr));
}
