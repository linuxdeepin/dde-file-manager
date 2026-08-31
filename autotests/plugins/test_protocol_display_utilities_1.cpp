// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_protocol_display_utilities_1.cpp
 * @brief Unit tests for protocol_display_utilities methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displaycontrol/utilities/protocoldisplayutilities.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class protocol_display_utilitiesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new protocol_display_utilities();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    protocol_display_utilities *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(protocol_display_utilitiesTest, getDisplayNameOf)
{
    // Test method: QString getDisplayNameOf((const QUrl &entryUrl))
    QUrl _arg0{};
    auto result = obj->getDisplayNameOf(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
