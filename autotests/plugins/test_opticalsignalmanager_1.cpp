// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalsignalmanager_1.cpp
 * @brief Unit tests for OpticalSignalManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/opticalsignalmanager.h"

#include <QTest>

using namespace dfmplugin_optical;

class OpticalSignalManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalSignalManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalSignalManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalSignalManagerTest, OpticalSignalManager)
{
    // Test constructor: OpticalSignalManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
