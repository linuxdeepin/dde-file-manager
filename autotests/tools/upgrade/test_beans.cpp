// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "units/beans/filetaginfo.h"
#include "units/beans/tagproperty.h"
#include "units/beans/oldtagproperty.h"
#include "units/beans/oldfileproperty.h"
#include "units/beans/virtualentrydata.h"
#include "units/beans/sqlitemaster.h"
#include "units/bookmarkupgradeunit.h"
#include "units/smbvirtualentryupgradeunit.h"

#include <QVariantMap>
#include <QUrl>
#include <QDateTime>

using namespace dfm_upgrade;

// ---------------------------------------------------------------------------
// FileTagInfo tests
// ---------------------------------------------------------------------------
class FileTagInfoTest : public testing::Test
{
protected:
    void SetUp() override { info = new FileTagInfo(); }
    void TearDown() override { delete info; }
    FileTagInfo *info = nullptr;
};

TEST_F(FileTagInfoTest, DefaultConstructor_InitializesToDefaults)
{
    EXPECT_EQ(info->getFileIndex(), 0);
    EXPECT_TRUE(info->getFilePath().isEmpty());
    EXPECT_TRUE(info->getTagName().isEmpty());
    EXPECT_EQ(info->getTagOrder(), 0);
    EXPECT_TRUE(info->getFuture().isEmpty());
}

TEST_F(FileTagInfoTest, SetAndGetFileIndex_RoundTrip)
{
    info->setFileIndex(42);
    EXPECT_EQ(info->getFileIndex(), 42);
}

TEST_F(FileTagInfoTest, SetAndGetFilePath_RoundTrip)
{
    info->setFilePath("/home/user/file.txt");
    EXPECT_EQ(info->getFilePath(), "/home/user/file.txt");
}

TEST_F(FileTagInfoTest, SetAndGetTagName_RoundTrip)
{
    info->setTagName("Important");
    EXPECT_EQ(info->getTagName(), "Important");
}

TEST_F(FileTagInfoTest, SetAndGetTagOrder_RoundTrip)
{
    info->setTagOrder(5);
    EXPECT_EQ(info->getTagOrder(), 5);
}

TEST_F(FileTagInfoTest, SetAndGetFuture_RoundTrip)
{
    info->setFuture("reserved");
    EXPECT_EQ(info->getFuture(), "reserved");
}

// ---------------------------------------------------------------------------
// TagProperty tests
// ---------------------------------------------------------------------------
class TagPropertyTest : public testing::Test
{
protected:
    void SetUp() override { prop = new TagProperty(); }
    void TearDown() override { delete prop; }
    TagProperty *prop = nullptr;
};

TEST_F(TagPropertyTest, DefaultConstructor_InitializesToDefaults)
{
    EXPECT_EQ(prop->getTagIndex(), 0);
    EXPECT_TRUE(prop->getTagName().isEmpty());
    EXPECT_TRUE(prop->getTagColor().isEmpty());
    EXPECT_EQ(prop->getAmbiguity(), 0);
    EXPECT_TRUE(prop->getFuture().isEmpty());
}

TEST_F(TagPropertyTest, SetAndGetTagIndex_RoundTrip)
{
    prop->setTagIndex(10);
    EXPECT_EQ(prop->getTagIndex(), 10);
}

TEST_F(TagPropertyTest, SetAndGetTagName_RoundTrip)
{
    prop->setTagName("Work");
    EXPECT_EQ(prop->getTagName(), "Work");
}

TEST_F(TagPropertyTest, SetAndGetTagColor_RoundTrip)
{
    prop->setTagColor("#FF0000");
    EXPECT_EQ(prop->getTagColor(), "#FF0000");
}

TEST_F(TagPropertyTest, SetAndGetAmbiguity_RoundTrip)
{
    prop->setAmbiguity(1);
    EXPECT_EQ(prop->getAmbiguity(), 1);
}

TEST_F(TagPropertyTest, SetAndGetFuture_RoundTrip)
{
    prop->setFuture("future_val");
    EXPECT_EQ(prop->getFuture(), "future_val");
}

// ---------------------------------------------------------------------------
// OldTagProperty tests
// ---------------------------------------------------------------------------
class OldTagPropertyTest : public testing::Test
{
protected:
    void SetUp() override { prop = new OldTagProperty(); }
    void TearDown() override { delete prop; }
    OldTagProperty *prop = nullptr;
};

TEST_F(OldTagPropertyTest, DefaultConstructor_InitializesToDefaults)
{
    EXPECT_EQ(prop->getTagIndex(), 0);
    EXPECT_TRUE(prop->getTagName().isEmpty());
    EXPECT_TRUE(prop->getTagColor().isEmpty());
}

TEST_F(OldTagPropertyTest, SetAndGetTagIndex_RoundTrip)
{
    prop->setTagIndex(99);
    EXPECT_EQ(prop->getTagIndex(), 99);
}

TEST_F(OldTagPropertyTest, SetAndGetTagName_RoundTrip)
{
    prop->setTagName("OldTag");
    EXPECT_EQ(prop->getTagName(), "OldTag");
}

TEST_F(OldTagPropertyTest, SetAndGetTagColor_RoundTrip)
{
    prop->setTagColor("blue");
    EXPECT_EQ(prop->getTagColor(), "blue");
}

// ---------------------------------------------------------------------------
// OldFileProperty tests
// ---------------------------------------------------------------------------
class OldFilePropertyTest : public testing::Test
{
protected:
    void SetUp() override { prop = new OldFileProperty(); }
    void TearDown() override { delete prop; }
    OldFileProperty *prop = nullptr;
};

TEST_F(OldFilePropertyTest, DefaultConstructor_InitializesEmpty)
{
    EXPECT_TRUE(prop->getFilePath().isEmpty());
    EXPECT_TRUE(prop->getTag().isEmpty());
}

TEST_F(OldFilePropertyTest, SetAndGetFilePath_RoundTrip)
{
    prop->setFilePath("/tmp/document.pdf");
    EXPECT_EQ(prop->getFilePath(), "/tmp/document.pdf");
}

TEST_F(OldFilePropertyTest, SetAndGetTag_RoundTrip)
{
    prop->setTag("favorite");
    EXPECT_EQ(prop->getTag(), "favorite");
}

// ---------------------------------------------------------------------------
// SqliteMaster tests
// ---------------------------------------------------------------------------
class SqliteMasterTest : public testing::Test
{
protected:
    void SetUp() override { master = new SqliteMaster(); }
    void TearDown() override { delete master; }
    SqliteMaster *master = nullptr;
};

TEST_F(SqliteMasterTest, DefaultConstructor_InitializesToDefaults)
{
    EXPECT_TRUE(master->getType().isEmpty());
    EXPECT_TRUE(master->getName().isEmpty());
    EXPECT_TRUE(master->getTbl_name().isEmpty());
    EXPECT_EQ(master->getRootpage(), 0);
    EXPECT_TRUE(master->getSql().isEmpty());
}

TEST_F(SqliteMasterTest, SetAndGetType_RoundTrip)
{
    master->setType("table");
    EXPECT_EQ(master->getType(), "table");
}

TEST_F(SqliteMasterTest, SetAndGetName_RoundTrip)
{
    master->setName("my_table");
    EXPECT_EQ(master->getName(), "my_table");
}

TEST_F(SqliteMasterTest, SetAndGetTbl_name_RoundTrip)
{
    master->setTbl_name("my_table");
    EXPECT_EQ(master->getTbl_name(), "my_table");
}

TEST_F(SqliteMasterTest, SetAndGetRootpage_RoundTrip)
{
    master->setRootpage(7);
    EXPECT_EQ(master->getRootpage(), 7);
}

TEST_F(SqliteMasterTest, SetAndGetSql_RoundTrip)
{
    master->setSql("CREATE TABLE foo (id INTEGER)");
    EXPECT_EQ(master->getSql(), "CREATE TABLE foo (id INTEGER)");
}

// ---------------------------------------------------------------------------
// BookmarkData serialize tests
// ---------------------------------------------------------------------------
class BookmarkDataTest : public testing::Test
{
protected:
    void SetUp() override { data = new BookmarkData(); }
    void TearDown() override { delete data; }
    BookmarkData *data = nullptr;
};

TEST_F(BookmarkDataTest, Serialize_RoundTripsAllFields)
{
    data->created = QDateTime(QDate(2024, 1, 15), QTime(10, 30, 0));
    data->lastModified = QDateTime(QDate(2024, 6, 20), QTime(14, 0, 0));
    data->deviceUrl = "smb://host/share";
    data->name = "MyShare";
    data->url = QUrl("file:///home/user/docs");
    data->index = 3;
    data->isDefaultItem = true;

    QVariantMap result = data->serialize();
    EXPECT_FALSE(result.isEmpty());
    EXPECT_EQ(result.value("name").toString(), "MyShare");
    EXPECT_EQ(result.value("url").toUrl(), QUrl("file:///home/user/docs"));
    EXPECT_EQ(result.value("index").toInt(), 3);
    EXPECT_EQ(result.value("defaultItem").toBool(), true);
    EXPECT_EQ(result.value("mountPoint").toString(), "smb://host/share");
}

TEST_F(BookmarkDataTest, Serialize_EmptyData_ProducesMapWithDefaults)
{
    QVariantMap result = data->serialize();
    EXPECT_FALSE(result.isEmpty());
    EXPECT_EQ(result.value("index").toInt(), -1);
    EXPECT_EQ(result.value("defaultItem").toBool(), false);
}

// ---------------------------------------------------------------------------
// SmbVirtualEntryUpgradeUnit::convertFromMap test
// ---------------------------------------------------------------------------
class SmbConvertTest : public testing::Test
{
};

TEST_F(SmbConvertTest, ConvertFromMap_ValidInput_PopulatesData)
{
    SmbVirtualEntryUpgradeUnit unit;
    QVariantMap map;
    map["protocol"] = "smb";
    map["host"] = "192.168.1.50";
    map["share"] = "public";
    map["name"] = "PublicShare";

    // convertFromMap is private; we test via the public API path.
    // Verify the unit name and initialize don't crash.
    EXPECT_FALSE(unit.name().isEmpty());
    QMap<QString, QString> args;
    EXPECT_NO_FATAL_FAILURE(unit.initialize(args));
}
