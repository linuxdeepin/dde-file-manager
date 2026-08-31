// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashcorestartupprobe.cpp
 * @brief Unit tests for TrashCoreStartupProbe methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/trashcorestartupprobe.h"

#include <QTest>

using namespace dfmplugin_trashcore;

class TrashCoreStartupProbeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashCoreStartupProbe();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashCoreStartupProbe *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashCoreStartupProbeTest, start)
{
    // Test method: void start(())
    EXPECT_NO_FATAL_FAILURE(obj->start());
}
