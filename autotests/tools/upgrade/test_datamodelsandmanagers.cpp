// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/beans/tagproperty.h"
#include "units/beans/virtualentrydata.h"
#include "units/bookmarkupgrade/defaultitemmanager.h"
#include "units/bookmarkupgradeunit.h"
#include "units/dconfigupgradeunit.h"

#include <gtest/gtest.h>
#include <QObject>
#include <QString>
#include <QUrl>

using namespace dfm_upgrade;

// Test for TagProperty
TEST(TagProperty, getter_setter)
{
    TagProperty prop;
    
    // Test tagIndex
    prop.setTagIndex(10);
    EXPECT_EQ(prop.getTagIndex(), 10);
    
    // Test tagName
    prop.setTagName("test_tag");
    EXPECT_EQ(prop.getTagName(), QString("test_tag"));
    
    // Test tagColor
    prop.setTagColor("#FF0000");
    EXPECT_EQ(prop.getTagColor(), QString("#FF0000"));
    
    // Test ambiguity
    prop.setAmbiguity(1);
    EXPECT_EQ(prop.getAmbiguity(), 1);
    
    // Test future
    prop.setFuture("future_value");
    EXPECT_EQ(prop.getFuture(), QString("future_value"));
}

// Test for VirtualEntryData
TEST(VirtualEntryData, getter_setter)
{
    VirtualEntryData data;
    
    // Test key
    data.setKey("test_key");
    EXPECT_EQ(data.getKey(), QString("test_key"));
    
    // Test protocol
    data.setProtocol("smb");
    EXPECT_EQ(data.getProtocol(), QString("smb"));
    
    // Test host
    data.setHost("example.com");
    EXPECT_EQ(data.getHost(), QString("example.com"));
    
    // Test port
    data.setPort(445);
    EXPECT_EQ(data.getPort(), 445);
    
    // Test displayName
    data.setDisplayName("Test Display Name");
    EXPECT_EQ(data.getDisplayName(), QString("Test Display Name"));
}

// Test for VirtualEntryData constructor with SMB path
TEST(VirtualEntryData, constructor_with_smb_path)
{
    QString smbPath = "smb://example.com/share";
    VirtualEntryData data(smbPath);
    
    EXPECT_EQ(data.getKey(), smbPath);
    EXPECT_EQ(data.getProtocol(), QString("smb"));
    EXPECT_EQ(data.getHost(), QString("example.com"));
    EXPECT_EQ(data.getPort(), -1); // Default port for SMB
}

// Test for DefaultItemManager
TEST(DefaultItemManager, instance)
{
    auto instance1 = DefaultItemManager::instance();
    auto instance2 = DefaultItemManager::instance();
    
    // Should be singleton
    EXPECT_EQ(instance1, instance2);
}

// Test for BookmarkData struct
TEST(BookmarkData, serialize)
{
    BookmarkData data;
    
    // Set some values
    data.name = "test_bookmark";
    data.url = QUrl("file:///home/user/test");
    data.isDefaultItem = true;
    data.index = 5;
    
    // Test serialize
    QVariantMap serialized = data.serialize();
    EXPECT_FALSE(serialized.isEmpty());
    EXPECT_EQ(serialized.value("name").toString(), QString("test_bookmark"));
    EXPECT_EQ(serialized.value("isDefaultItem").toBool(), false); // 默认值为false
    EXPECT_EQ(serialized.value("index").toInt(), 5);
}

// Test for BookMarkUpgradeUnit
TEST(BookMarkUpgradeUnit, name)
{
    BookMarkUpgradeUnit unit;
    EXPECT_EQ(unit.name(), QString("BookMarkUpgradeUnit"));
}

// Test for DConfigUpgradeUnit
TEST(DConfigUpgradeUnit, name)
{
    DConfigUpgradeUnit unit;
    EXPECT_EQ(unit.name(), QString("DConfigUpgradeUnit"));
}