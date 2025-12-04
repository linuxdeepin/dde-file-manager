// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QIcon>
#include <QVariant>
#include <QRegularExpression>

#include "stubext.h"

#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/file/entry/private/entryfileinfo_p.h>
#include <dfm-base/interfaces/abstractentryfileentity.h>
#include <dfm-base/dfm_global_defines.h>

DFMBASE_USE_NAMESPACE

// Mock implementation of AbstractEntryFileEntity for testing EntryFileInfo
class TestEntryEntity : public AbstractEntryFileEntity
{
public:
    explicit TestEntryEntity(const QUrl &url)
        : AbstractEntryFileEntity(url)
    {
        mockExists = true;
        mockAccessable = true;
        mockRenamable = false;
        mockShowProgress = false;
        mockShowTotalSize = true;
        mockShowUsageSize = true;
        mockSizeTotal = 1000000;
        mockSizeUsage = 600000;
    }

    QString displayName() const override { return "TestEntityName"; }
    QIcon icon() const override { return QIcon(); }
    bool exists() const override { return mockExists; }
    bool showProgress() const override { return mockShowProgress; }
    bool showTotalSize() const override { return mockShowTotalSize; }
    bool showUsageSize() const override { return mockShowUsageSize; }
    EntryOrder order() const override { return EntryOrder::kOrderSysDisks; }

    quint64 sizeTotal() const override { return mockSizeTotal; }
    quint64 sizeUsage() const override { return mockSizeUsage; }
    QString description() const override { return "Test Description"; }
    QUrl targetUrl() const override { return QUrl("file:///test/target"); }
    bool isAccessable() const override { return mockAccessable; }
    bool renamable() const override { return mockRenamable; }
    QString editDisplayText() const override { return "EditTest"; }

    // Public members for controlling mock behavior
    bool mockExists;
    bool mockAccessable;
    bool mockRenamable;
    bool mockShowProgress;
    bool mockShowTotalSize;
    bool mockShowUsageSize;
    quint64 mockSizeTotal;
    quint64 mockSizeUsage;
};

class TestEntryFileInfo : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        // Register the test entity
        EntryEntityFactor::registCreator<TestEntryEntity>("testentry");
        EntryEntityFactor::registCreator<TestEntryEntity>("_common_");
    }

    void SetUp() override
    {
        testUrl = QUrl("entry:///test.testentry");
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    QUrl testUrl;
};

// ========== Constructor and Destructor Tests ==========

TEST_F(TestEntryFileInfo, Constructor_ValidUrl)
{
    EntryFileInfo info(testUrl);
    EXPECT_NE(info.entity(), nullptr);
}

TEST_F(TestEntryFileInfo, Entity_ReturnsValidEntity)
{
    EntryFileInfo info(testUrl);
    AbstractEntryFileEntity *entity = info.entity();
    EXPECT_NE(entity, nullptr);
    EXPECT_EQ(entity->displayName(), "TestEntityName");
}

// ========== Order Tests ==========

TEST_F(TestEntryFileInfo, Order_ReturnsEntityOrder)
{
    EntryFileInfo info(testUrl);
    AbstractEntryFileEntity::EntryOrder order = info.order();
    EXPECT_EQ(order, AbstractEntryFileEntity::EntryOrder::kOrderSysDisks);
}

TEST_F(TestEntryFileInfo, Order_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    AbstractEntryFileEntity::EntryOrder order = info.order();
    EXPECT_EQ(order, AbstractEntryFileEntity::EntryOrder::kOrderCustom);
}

// ========== TargetUrl Tests ==========

TEST_F(TestEntryFileInfo, TargetUrl_ReturnsEntityTargetUrl)
{
    EntryFileInfo info(testUrl);
    QUrl target = info.targetUrl();
    EXPECT_EQ(target, QUrl("file:///test/target"));
}

TEST_F(TestEntryFileInfo, TargetUrl_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    QUrl target = info.targetUrl();
    EXPECT_TRUE(target.isEmpty());
}

// ========== IsAccessable Tests ==========

TEST_F(TestEntryFileInfo, IsAccessable_ReturnsTrue)
{
    EntryFileInfo info(testUrl);
    bool result = info.isAccessable();
    EXPECT_TRUE(result);
}

TEST_F(TestEntryFileInfo, IsAccessable_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    bool result = info.isAccessable();
    EXPECT_FALSE(result);
}

// ========== Description Tests ==========

TEST_F(TestEntryFileInfo, Description_ReturnsEntityDescription)
{
    EntryFileInfo info(testUrl);
    QString desc = info.description();
    EXPECT_EQ(desc, "Test Description");
}

TEST_F(TestEntryFileInfo, Description_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    QString desc = info.description();
    EXPECT_TRUE(desc.isEmpty());
}

// ========== ExtraProperty Tests ==========

TEST_F(TestEntryFileInfo, ExtraProperty_ExistingProperty)
{
    EntryFileInfo info(testUrl);
    info.setExtraProperty("testKey", QVariant("testValue"));

    QVariant value = info.extraProperty("testKey");
    EXPECT_EQ(value.toString(), "testValue");
}

TEST_F(TestEntryFileInfo, ExtraProperty_NonExistingProperty)
{
    EntryFileInfo info(testUrl);
    QVariant value = info.extraProperty("nonexistent");
    EXPECT_FALSE(value.isValid());
}

TEST_F(TestEntryFileInfo, SetExtraProperty_Success)
{
    EntryFileInfo info(testUrl);
    info.setExtraProperty("intKey", QVariant(42));

    QVariantHash properties = info.extraProperties();
    EXPECT_TRUE(properties.contains("intKey"));
    EXPECT_EQ(properties["intKey"].toInt(), 42);
}

// ========== Renamable Tests ==========

TEST_F(TestEntryFileInfo, Renamable_ReturnsFalse)
{
    EntryFileInfo info(testUrl);
    bool result = info.renamable();
    EXPECT_FALSE(result);
}

TEST_F(TestEntryFileInfo, Renamable_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    bool result = info.renamable();
    EXPECT_FALSE(result);
}

// ========== DisplayName Tests ==========

TEST_F(TestEntryFileInfo, DisplayName_ReturnsEntityDisplayName)
{
    EntryFileInfo info(testUrl);
    QString name = info.displayName();
    EXPECT_EQ(name, "TestEntityName");
}

TEST_F(TestEntryFileInfo, DisplayName_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    QString name = info.displayName();
    EXPECT_TRUE(name.isEmpty());
}

// ========== EditDisplayText Tests ==========

TEST_F(TestEntryFileInfo, EditDisplayText_ReturnsEntityText)
{
    EntryFileInfo info(testUrl);
    QString text = info.editDisplayText();
    EXPECT_EQ(text, "EditTest");
}

TEST_F(TestEntryFileInfo, EditDisplayText_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    QString text = info.editDisplayText();
    EXPECT_TRUE(text.isEmpty());
}

// ========== Size Tests ==========

TEST_F(TestEntryFileInfo, SizeTotal_ReturnsCorrectValue)
{
    EntryFileInfo info(testUrl);
    quint64 size = info.sizeTotal();
    EXPECT_EQ(size, 1000000);
}

TEST_F(TestEntryFileInfo, SizeTotal_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    quint64 size = info.sizeTotal();
    EXPECT_EQ(size, 0);
}

TEST_F(TestEntryFileInfo, SizeUsage_ReturnsCorrectValue)
{
    EntryFileInfo info(testUrl);
    quint64 size = info.sizeUsage();
    EXPECT_EQ(size, 600000);
}

TEST_F(TestEntryFileInfo, SizeUsage_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    quint64 size = info.sizeUsage();
    EXPECT_EQ(size, 0);
}

TEST_F(TestEntryFileInfo, SizeFree_CalculatesCorrectly)
{
    EntryFileInfo info(testUrl);
    quint64 sizeFree = info.sizeFree();
    EXPECT_EQ(sizeFree, 400000);   // 1000000 - 600000
}

TEST_F(TestEntryFileInfo, SizeFree_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    quint64 sizeFree = info.sizeFree();
    EXPECT_EQ(sizeFree, 0);
}

// ========== Show Size Tests ==========

TEST_F(TestEntryFileInfo, ShowTotalSize_ReturnsTrue)
{
    EntryFileInfo info(testUrl);
    bool result = info.showTotalSize();
    EXPECT_TRUE(result);
}

TEST_F(TestEntryFileInfo, ShowTotalSize_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    bool result = info.showTotalSize();
    EXPECT_FALSE(result);
}

TEST_F(TestEntryFileInfo, ShowUsedSize_ReturnsTrue)
{
    EntryFileInfo info(testUrl);
    bool result = info.showUsedSize();
    EXPECT_TRUE(result);
}

TEST_F(TestEntryFileInfo, ShowUsedSize_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    bool result = info.showUsedSize();
    EXPECT_FALSE(result);
}

// ========== ShowProgress Tests ==========

TEST_F(TestEntryFileInfo, ShowProgress_ReturnsFalse)
{
    EntryFileInfo info(testUrl);
    bool result = info.showProgress();
    EXPECT_FALSE(result);
}

TEST_F(TestEntryFileInfo, ShowProgress_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    bool result = info.showProgress();
    EXPECT_FALSE(result);
}

// ========== Exists Tests ==========

TEST_F(TestEntryFileInfo, Exists_ReturnsTrue)
{
    EntryFileInfo info(testUrl);
    bool result = info.exists();
    EXPECT_TRUE(result);
}

TEST_F(TestEntryFileInfo, Exists_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    bool result = info.exists();
    EXPECT_FALSE(result);
}

// ========== NameOf Tests ==========

TEST_F(TestEntryFileInfo, NameOf_BaseName)
{
    EntryFileInfo info(testUrl);
    QString name = info.nameOf(FileInfo::FileNameInfoType::kBaseName);
    EXPECT_TRUE(name.isEmpty());
}

TEST_F(TestEntryFileInfo, NameOf_Suffix)
{
    EntryFileInfo info(testUrl);
    QString suffix = info.nameOf(FileInfo::FileNameInfoType::kSuffix);
    EXPECT_EQ(suffix, "testentry");
}

TEST_F(TestEntryFileInfo, NameOf_FileName)
{
    EntryFileInfo info(testUrl);
    QString fileName = info.nameOf(FileInfo::FileNameInfoType::kFileName);
    EXPECT_EQ(fileName, "test.testentry");
}

// ========== DisplayOf Tests ==========

TEST_F(TestEntryFileInfo, DisplayOf_FileDisplayName)
{
    EntryFileInfo info(testUrl);
    QString display = info.displayOf(FileInfo::DisplayInfoType::kFileDisplayName);
    EXPECT_EQ(display, "TestEntityName");
}

// ========== PathOf Tests ==========

TEST_F(TestEntryFileInfo, PathOf_Path)
{
    EntryFileInfo info(testUrl);
    QString path = info.pathOf(FileInfo::FilePathInfoType::kPath);
    EXPECT_EQ(path, "/test.testentry");
}

TEST_F(TestEntryFileInfo, PathOf_FilePath)
{
    EntryFileInfo info(testUrl);
    QString path = info.pathOf(FileInfo::FilePathInfoType::kFilePath);
    EXPECT_EQ(path, "/test.testentry");
}

// ========== FileIcon Tests ==========

TEST_F(TestEntryFileInfo, FileIcon_ReturnsIcon)
{
    EntryFileInfo info(testUrl);
    QIcon icon = info.fileIcon();
    EXPECT_TRUE(icon.isNull());
}

TEST_F(TestEntryFileInfo, FileIcon_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    QIcon icon = info.fileIcon();
    EXPECT_TRUE(icon.isNull());
}

// ========== Refresh Tests ==========

TEST_F(TestEntryFileInfo, Refresh_CanBeCalled)
{
    EntryFileInfo info(testUrl);
    info.refresh();
    // No assertion needed, just verify it doesn't crash
}

TEST_F(TestEntryFileInfo, Refresh_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    info.refresh();
    // No assertion needed, just verify it doesn't crash
}

// ========== ExtraProperties Tests ==========

TEST_F(TestEntryFileInfo, ExtraProperties_ReturnsHash)
{
    EntryFileInfo info(testUrl);
    info.setExtraProperty("key1", QVariant(10));
    info.setExtraProperty("key2", QVariant("value"));

    QVariantHash properties = info.extraProperties();
    EXPECT_EQ(properties.size(), 2);
    EXPECT_TRUE(properties.contains("key1"));
    EXPECT_TRUE(properties.contains("key2"));
}

TEST_F(TestEntryFileInfo, ExtraProperties_NullEntity)
{
    EntryFileInfo info(testUrl);
    stub.set_lamda(VADDR(EntryFileInfo, entity), [](EntryFileInfo *) -> AbstractEntryFileEntity * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    QVariantHash properties = info.extraProperties();
    EXPECT_TRUE(properties.isEmpty());
}
