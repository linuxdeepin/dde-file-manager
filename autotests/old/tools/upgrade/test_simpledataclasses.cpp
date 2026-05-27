// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/beans/filetaginfo.h"
#include "units/beans/oldfileproperty.h"
#include "units/beans/oldtagproperty.h"
#include "units/beans/sqlitemaster.h"

#include <gtest/gtest.h>
#include <QObject>
#include <QString>

using namespace dfm_upgrade;

TEST(FileTagInfo, getter_setter)
{
    FileTagInfo info;
    
    // Test fileIndex
    info.setFileIndex(10);
    EXPECT_EQ(info.getFileIndex(), 10);
    
    // Test filePath
    info.setFilePath("test/path");
    EXPECT_EQ(info.getFilePath(), QString("test/path"));
    
    // Test tagName
    info.setTagName("test_tag");
    EXPECT_EQ(info.getTagName(), QString("test_tag"));
    
    // Test tagOrder
    info.setTagOrder(5);
    EXPECT_EQ(info.getTagOrder(), 5);
    
    // Test future
    info.setFuture("future_value");
    EXPECT_EQ(info.getFuture(), QString("future_value"));
}

TEST(OldFileProperty, getter_setter)
{
    OldFileProperty prop;
    
    // Test filePath
    prop.setFilePath("old/file/path");
    EXPECT_EQ(prop.getFilePath(), QString("old/file/path"));
    
    // Test tag
    prop.setTag("old_tag");
    EXPECT_EQ(prop.getTag(), QString("old_tag"));
}

TEST(OldTagProperty, getter_setter)
{
    OldTagProperty prop;
    
    // Test tagIndex
    prop.setTagIndex(20);
    EXPECT_EQ(prop.getTagIndex(), 20);
    
    // Test tagName
    prop.setTagName("old_tag_name");
    EXPECT_EQ(prop.getTagName(), QString("old_tag_name"));
    
    // Test tagColor
    prop.setTagColor("#FF0000");
    EXPECT_EQ(prop.getTagColor(), QString("#FF0000"));
}

TEST(SqliteMaster, getter_setter)
{
    SqliteMaster master;
    
    // Test type
    master.setType("table");
    EXPECT_EQ(master.getType(), QString("table"));
    
    // Test name
    master.setName("test_table");
    EXPECT_EQ(master.getName(), QString("test_table"));
    
    // Test tbl_name
    master.setTbl_name("tbl_test");
    EXPECT_EQ(master.getTbl_name(), QString("tbl_test"));
    
    // Test rootpage
    master.setRootpage(100);
    EXPECT_EQ(master.getRootpage(), 100);
    
    // Test sql
    master.setSql("CREATE TABLE test");
    EXPECT_EQ(master.getSql(), QString("CREATE TABLE test"));
}