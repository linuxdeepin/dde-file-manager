// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/beans/virtualentrydata.h"
#include "units/smbvirtualentryupgradeunit.h"

#include "stubext.h"

#include <gtest/gtest.h>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDateTime>

using namespace dfm_upgrade;

// Add tests for VirtualEntryData copy constructor and assignment operator
class TestVirtualEntryData : public testing::Test {
};

TEST_F(TestVirtualEntryData, copyConstructor)
{
    dfm_upgrade::VirtualEntryData original;
    original.setKey("test_key");
    original.setHost("test_host");
    original.setProtocol("smb");
    original.setPort(45);
    original.setDisplayName("Test Entry");
    
    dfm_upgrade::VirtualEntryData copied(original);
    
    EXPECT_EQ(copied.getKey(), original.getKey());
    EXPECT_EQ(copied.getHost(), original.getHost());
    EXPECT_EQ(copied.getProtocol(), original.getProtocol());
    EXPECT_EQ(copied.getPort(), original.getPort());
    EXPECT_EQ(copied.getDisplayName(), original.getDisplayName());
}

TEST_F(TestVirtualEntryData, assignmentOperator)
{
    dfm_upgrade::VirtualEntryData original;
    original.setKey("test_key");
    original.setHost("test_host");
    original.setProtocol("smb");
    original.setPort(445);
    original.setDisplayName("Test Entry");
    
    dfm_upgrade::VirtualEntryData assigned;
    assigned = original;
    
    EXPECT_EQ(assigned.getKey(), original.getKey());
    EXPECT_EQ(assigned.getHost(), original.getHost());
    EXPECT_EQ(assigned.getProtocol(), original.getProtocol());
    EXPECT_EQ(assigned.getPort(), original.getPort());
    EXPECT_EQ(assigned.getDisplayName(), original.getDisplayName());
}