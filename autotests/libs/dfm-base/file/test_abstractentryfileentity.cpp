// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QIcon>
#include <QVariant>

#include "stubext.h"

#include <dfm-base/interfaces/abstractentryfileentity.h>

DFMBASE_USE_NAMESPACE

// Mock implementation of AbstractEntryFileEntity for testing
class MockEntryFileEntity : public AbstractEntryFileEntity
{
public:
    explicit MockEntryFileEntity(const QUrl &url)
        : AbstractEntryFileEntity(url)
    {
    }

    QString displayName() const override
    {
        return "MockEntity";
    }

    QIcon icon() const override
    {
        return QIcon();
    }

    bool exists() const override
    {
        return true;
    }

    bool showProgress() const override
    {
        return false;
    }

    bool showTotalSize() const override
    {
        return true;
    }

    bool showUsageSize() const override
    {
        return true;
    }

    EntryOrder order() const override
    {
        return EntryOrder::kOrderCustom;
    }

    quint64 sizeTotal() const override
    {
        return 1000000;
    }

    quint64 sizeUsage() const override
    {
        return 500000;
    }

    QString description() const override
    {
        return "Mock Description";
    }

    QUrl targetUrl() const override
    {
        return QUrl("file:///tmp/target");
    }

    bool isAccessable() const override
    {
        return true;
    }

    bool renamable() const override
    {
        return true;
    }

    QString editDisplayText() const override
    {
        return "Edit Mock";
    }
};

class TestAbstractEntryFileEntity : public testing::Test
{
public:
    void SetUp() override
    {
        testUrl = QUrl("entry:///test.mock");
        entity = new MockEntryFileEntity(testUrl);
    }

    void TearDown() override
    {
        stub.clear();
        delete entity;
        entity = nullptr;
    }

protected:
    stub_ext::StubExt stub;
    QUrl testUrl;
    MockEntryFileEntity *entity = nullptr;
};

// ========== Constructor and Destructor Tests ==========

TEST_F(TestAbstractEntryFileEntity, Constructor_ValidUrl)
{
    QUrl url("entry:///test.entity");
    MockEntryFileEntity testEntity(url);
    EXPECT_EQ(testEntity.entryUrl, url);
}

// ========== Pure Virtual Function Tests (through MockEntryFileEntity) ==========

TEST_F(TestAbstractEntryFileEntity, DisplayName_ReturnsCorrectValue)
{
    QString result = entity->displayName();
    EXPECT_EQ(result, "MockEntity");
}

TEST_F(TestAbstractEntryFileEntity, Icon_ReturnsIcon)
{
    QIcon result = entity->icon();
    EXPECT_TRUE(result.isNull());
}

TEST_F(TestAbstractEntryFileEntity, Exists_ReturnsTrue)
{
    bool result = entity->exists();
    EXPECT_TRUE(result);
}

TEST_F(TestAbstractEntryFileEntity, ShowProgress_ReturnsFalse)
{
    bool result = entity->showProgress();
    EXPECT_FALSE(result);
}

TEST_F(TestAbstractEntryFileEntity, ShowTotalSize_ReturnsTrue)
{
    bool result = entity->showTotalSize();
    EXPECT_TRUE(result);
}

TEST_F(TestAbstractEntryFileEntity, ShowUsageSize_ReturnsTrue)
{
    bool result = entity->showUsageSize();
    EXPECT_TRUE(result);
}

TEST_F(TestAbstractEntryFileEntity, Order_ReturnsCustomOrder)
{
    AbstractEntryFileEntity::EntryOrder result = entity->order();
    EXPECT_EQ(result, AbstractEntryFileEntity::EntryOrder::kOrderCustom);
}

// ========== Virtual Function with Default Implementation Tests ==========

TEST_F(TestAbstractEntryFileEntity, Refresh_CanBeCalled)
{
    entity->refresh();
    // No assertion needed, just verify it doesn't crash
}

TEST_F(TestAbstractEntryFileEntity, SizeTotal_ReturnsValue)
{
    quint64 result = entity->sizeTotal();
    EXPECT_EQ(result, 1000000);
}

TEST_F(TestAbstractEntryFileEntity, SizeUsage_ReturnsValue)
{
    quint64 result = entity->sizeUsage();
    EXPECT_EQ(result, 500000);
}

TEST_F(TestAbstractEntryFileEntity, Description_ReturnsString)
{
    QString result = entity->description();
    EXPECT_EQ(result, "Mock Description");
}

TEST_F(TestAbstractEntryFileEntity, TargetUrl_ReturnsUrl)
{
    QUrl result = entity->targetUrl();
    EXPECT_EQ(result, QUrl("file:///tmp/target"));
}

TEST_F(TestAbstractEntryFileEntity, IsAccessable_ReturnsTrue)
{
    bool result = entity->isAccessable();
    EXPECT_TRUE(result);
}

TEST_F(TestAbstractEntryFileEntity, Renamable_ReturnsTrue)
{
    bool result = entity->renamable();
    EXPECT_TRUE(result);
}

TEST_F(TestAbstractEntryFileEntity, EditDisplayText_ReturnsString)
{
    QString result = entity->editDisplayText();
    EXPECT_EQ(result, "Edit Mock");
}

// ========== Extra Properties Tests ==========

TEST_F(TestAbstractEntryFileEntity, ExtraProperties_InitiallyEmpty)
{
    QVariantHash properties = entity->extraProperties();
    EXPECT_TRUE(properties.isEmpty());
}

TEST_F(TestAbstractEntryFileEntity, SetExtraProperty_SingleProperty)
{
    entity->setExtraProperty("testKey", QVariant("testValue"));
    QVariantHash properties = entity->extraProperties();
    EXPECT_TRUE(properties.contains("testKey"));
    EXPECT_EQ(properties["testKey"].toString(), "testValue");
}

TEST_F(TestAbstractEntryFileEntity, SetExtraProperty_MultipleProperties)
{
    entity->setExtraProperty("key1", QVariant(123));
    entity->setExtraProperty("key2", QVariant("value2"));
    entity->setExtraProperty("key3", QVariant(true));

    QVariantHash properties = entity->extraProperties();
    EXPECT_EQ(properties.size(), 3);
    EXPECT_EQ(properties["key1"].toInt(), 123);
    EXPECT_EQ(properties["key2"].toString(), "value2");
    EXPECT_EQ(properties["key3"].toBool(), true);
}

TEST_F(TestAbstractEntryFileEntity, SetExtraProperty_OverwriteExisting)
{
    entity->setExtraProperty("key", QVariant("value1"));
    entity->setExtraProperty("key", QVariant("value2"));

    QVariantHash properties = entity->extraProperties();
    EXPECT_EQ(properties.size(), 1);
    EXPECT_EQ(properties["key"].toString(), "value2");
}

// ========== EntryEntityFactor Tests ==========

TEST_F(TestAbstractEntryFileEntity, EntryEntityFactor_RegistCreator_Success)
{
    bool result = EntryEntityFactor::registCreator<MockEntryFileEntity>("test_suffix");
    EXPECT_TRUE(result);

    AbstractEntryFileEntity *created = EntryEntityFactor::create("test_suffix", QUrl("entry:///test.test_suffix"));
    EXPECT_NE(created, nullptr);
    delete created;
}

TEST_F(TestAbstractEntryFileEntity, EntryEntityFactor_RegistCreator_DuplicateFails)
{
    EntryEntityFactor::registCreator<MockEntryFileEntity>("duplicate_suffix");
    bool result = EntryEntityFactor::registCreator<MockEntryFileEntity>("duplicate_suffix");
    EXPECT_FALSE(result);
}

TEST_F(TestAbstractEntryFileEntity, EntryEntityFactor_Create_UnregisteredSuffix)
{
    AbstractEntryFileEntity *created = EntryEntityFactor::create("nonexistent_suffix", QUrl("entry:///test.nonexistent"));
    EXPECT_EQ(created, nullptr);
}

TEST_F(TestAbstractEntryFileEntity, EntryEntityFactor_Create_ValidSuffix)
{
    EntryEntityFactor::registCreator<MockEntryFileEntity>("valid_suffix");
    AbstractEntryFileEntity *created = EntryEntityFactor::create("valid_suffix", QUrl("entry:///test.valid"));
    EXPECT_NE(created, nullptr);
    EXPECT_EQ(created->displayName(), "MockEntity");
    delete created;
}

// ========== EntryOrder Enum Tests ==========

TEST_F(TestAbstractEntryFileEntity, EntryOrder_AllValuesAccessible)
{
    EXPECT_EQ(static_cast<int>(AbstractEntryFileEntity::EntryOrder::kOrderUserDir), 0);
    EXPECT_GE(static_cast<int>(AbstractEntryFileEntity::EntryOrder::kOrderCustom), 0);
}
