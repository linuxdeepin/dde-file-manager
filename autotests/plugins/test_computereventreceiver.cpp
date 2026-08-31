// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computereventreceiver.cpp
 * @brief Unit tests for ComputerEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/computereventreceiver.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerEventReceiverTest, ComputerEventReceiver)
{
    // Test constructor: ComputerEventReceiver((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ComputerEventReceiverTest, parseDevMountCrumb)
{
    // Test method: bool parseDevMountCrumb((const QUrl &url, QList<QVariantMap> *mapGroup))
    QUrl _arg0{};
    auto result = obj->parseDevMountCrumb(_arg0, nullptr);
    EXPECT_FALSE(result);

}
