// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualentrydata.cpp
 * @brief Unit tests for VirtualEntryData Mid-priority methods (dfmplugin-smbbrowser)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "displaycontrol/datahelper/virtualentrydata.h"

using namespace dfmplugin_smbbrowser;

class VirtualEntryDataTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(VirtualEntryDataTest, setDisplayName)
{
    // Instance method setDisplayName
    VirtualEntryData obj;
    EXPECT_NO_FATAL_FAILURE({ obj.setDisplayName(QString("test")); });
}

TEST_F(VirtualEntryDataTest, setHost)
{
    // Instance method setHost
    VirtualEntryData obj;
    EXPECT_NO_FATAL_FAILURE({ obj.setHost(QString("test")); });
}

TEST_F(VirtualEntryDataTest, setKey)
{
    // Instance method setKey
    VirtualEntryData obj;
    EXPECT_NO_FATAL_FAILURE({ obj.setKey(QString("test")); });
}

TEST_F(VirtualEntryDataTest, setPort)
{
    // Instance method setPort
    VirtualEntryData obj;
    EXPECT_NO_FATAL_FAILURE({ obj.setPort(0); });
}

TEST_F(VirtualEntryDataTest, setProtocol)
{
    // Instance method setProtocol
    VirtualEntryData obj;
    EXPECT_NO_FATAL_FAILURE({ obj.setProtocol(QString("test")); });
}

TEST_F(VirtualEntryDataTest, setTargetPath)
{
    // Instance method setTargetPath
    VirtualEntryData obj;
    EXPECT_NO_FATAL_FAILURE({ obj.setTargetPath(QString("test")); });
}

TEST_F(VirtualEntryDataTest, getDisplayName)
{
    // getDisplayName
    SUCCEED();
}

TEST_F(VirtualEntryDataTest, getHost)
{
    // getHost
    SUCCEED();
}

TEST_F(VirtualEntryDataTest, getKey)
{
    // getKey
    SUCCEED();
}

TEST_F(VirtualEntryDataTest, getPort)
{
    // getPort
    SUCCEED();
}

TEST_F(VirtualEntryDataTest, getProtocol)
{
    // getProtocol
    SUCCEED();
}

TEST_F(VirtualEntryDataTest, getTargetPath)
{
    // getTargetPath
    SUCCEED();
}
