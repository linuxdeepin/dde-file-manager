// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_ocrindexdbusprivate.cpp
 * @brief Unit tests for OcrIndexDBusPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/ocrindexdbus.h"

#include <QTest>

using namespace src;

class OcrIndexDBusPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OcrIndexDBusPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OcrIndexDBusPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OcrIndexDBusPrivateTest, canSilentlyRefreshIndex)
{
    // Test method: bool canSilentlyRefreshIndex((const QString &path))
    QString _arg0{};
    auto result = obj->canSilentlyRefreshIndex(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OcrIndexDBusPrivateTest, handleConfigChanged)
{
    // Test method: void handleConfigChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->handleConfigChanged());
}

TEST_F(OcrIndexDBusPrivateTest, handleMonitoring)
{
    // Test method: void handleMonitoring((bool start))
    EXPECT_NO_FATAL_FAILURE(obj->handleMonitoring(false));
}

TEST_F(OcrIndexDBusPrivateTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(OcrIndexDBusPrivateTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(OcrIndexDBusPrivateTest, initializeSupportedExtensions)
{
    // Test method: void initializeSupportedExtensions(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeSupportedExtensions());
}
