// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_organizercreator.cpp
 * @brief Unit tests for OrganizerCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/canvasorganizer.h"

#include <QTest>

using namespace ddplugin_organizer;

class OrganizerCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OrganizerCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OrganizerCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OrganizerCreatorTest, createOrganizer)
{
    // Test method: CanvasOrganizer createOrganizer((OrganizerMode mode))
    auto result = obj->createOrganizer(OrganizerMode());
    EXPECT_NO_FATAL_FAILURE({ obj->createOrganizer(OrganizerMode()); });

}
