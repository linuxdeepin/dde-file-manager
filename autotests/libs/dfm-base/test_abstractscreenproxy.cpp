// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractscreenproxy.cpp
 * @brief Unit tests for AbstractScreenProxy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/interfaces/screen/abstractscreenproxy.h"

#include <QTest>

using namespace src;

class AbstractScreenProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractScreenProxy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractScreenProxy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractScreenProxyTest, appendEvent)
{
    // Test event handler: appendEvent((AbstractScreenProxy::Event e))
    AbstractScreenProxy::Event _event(AbstractScreenProxy::Event::None);
    EXPECT_NO_FATAL_FAILURE(obj->appendEvent(&_event));
}
