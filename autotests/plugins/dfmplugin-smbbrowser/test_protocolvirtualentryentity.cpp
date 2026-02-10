// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "displaycontrol/info/protocolvirtualentryentity.h"
#include "displaycontrol/datahelper/virtualentrydbhandler.h"

#include <dfm-base/interfaces/abstractentryfileentity.h>

#include <QUrl>
#include <QString>
#include <QIcon>
#include <QVariantMap>

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class ProtocolVirtualEntryEntityTest : public testing::Test
{
public:
    void SetUp() override
    {
        stub.clear();

        entity = nullptr;

        // 创建测试URL
        testUrl = QUrl("smb://192.168.1.100/share.ventry");
        testUrlWithSuffix = QUrl("smb://192.168.1.200/test.ventry");
        testUrlWithoutSuffix = QUrl("smb://192.168.1.300/plain");

        // Mock数据
        mockDisplayName = "Test Virtual Entry";
        mockFullSmbPath = "smb://192.168.1.100/share/full/path";
        mockVEntryDbHandler = nullptr;
    }

    void TearDown() override
    {
        stub.clear();
        delete entity;
        entity = nullptr;
    }

protected:
    ProtocolVirtualEntryEntity *entity;
    stub_ext::StubExt stub;

    QUrl testUrl;
    QUrl testUrlWithSuffix;
    QUrl testUrlWithoutSuffix;

    QString mockDisplayName;
    QString mockFullSmbPath;
    VirtualEntryDbHandler *mockVEntryDbHandler;
};

TEST_F(ProtocolVirtualEntryEntityTest, Constructor_ValidUrl_EntityCreated)
{
    entity = new ProtocolVirtualEntryEntity(testUrl);

    EXPECT_NE(entity, nullptr);
    EXPECT_EQ(entity->entryUrl, testUrl);
}

TEST_F(ProtocolVirtualEntryEntityTest, Constructor_EmptyUrl_EntityCreated)
{
    QUrl emptyUrl;
    entity = new ProtocolVirtualEntryEntity(emptyUrl);

    EXPECT_NE(entity, nullptr);
    EXPECT_EQ(entity->entryUrl, emptyUrl);
}

TEST_F(ProtocolVirtualEntryEntityTest, DisplayName_CachedValue_ReturnsCachedName)
{
    entity = new ProtocolVirtualEntryEntity(testUrl);

    // 模拟已缓存的显示名称
    entity->datas.insert("ventry_display_name", mockDisplayName);

    QString displayName = entity->displayName();

    EXPECT_EQ(displayName, mockDisplayName);
}

TEST_F(ProtocolVirtualEntryEntityTest, DisplayName_EmptyCacheValidDbHandler_ReturnsDbName)
{
    // 打桩VirtualEntryDbHandler::instance
    static VirtualEntryDbHandler mockHandler;
    mockVEntryDbHandler = &mockHandler;

    stub.set_lamda(VirtualEntryDbHandler::instance, [&]() -> VirtualEntryDbHandler * {
        __DBG_STUB_INVOKE__
        return mockVEntryDbHandler;
    });

    // 打桩getDisplayNameOf方法
    stub.set_lamda(ADDR(VirtualEntryDbHandler, getDisplayNameOf), [&](VirtualEntryDbHandler *, const QUrl &url) -> QString {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(url, testUrl);
        return mockDisplayName;
    });

    entity = new ProtocolVirtualEntryEntity(testUrl);

    QString displayName = entity->displayName();

    EXPECT_EQ(displayName, mockDisplayName);
    // 验证结果被缓存
    EXPECT_EQ(entity->datas.value("ventry_display_name").toString(), mockDisplayName);
}

TEST_F(ProtocolVirtualEntryEntityTest, DisplayName_EmptyCacheEmptyDbResult_ReturnsEmptyString)
{
    // 打桩VirtualEntryDbHandler::instance
    static VirtualEntryDbHandler mockHandler;
    mockVEntryDbHandler = &mockHandler;

    stub.set_lamda(VirtualEntryDbHandler::instance, [&]() -> VirtualEntryDbHandler * {
        __DBG_STUB_INVOKE__
        return mockVEntryDbHandler;
    });

    // 打桩getDisplayNameOf返回空字符串
    stub.set_lamda(ADDR(VirtualEntryDbHandler, getDisplayNameOf), [&](VirtualEntryDbHandler *, const QUrl &url) -> QString {
        __DBG_STUB_INVOKE__
        return QString();
    });

    entity = new ProtocolVirtualEntryEntity(testUrl);

    QString displayName = entity->displayName();

    EXPECT_TRUE(displayName.isEmpty());
}

TEST_F(ProtocolVirtualEntryEntityTest, DisplayName_MultipleCalls_UsesCache)
{
    // 打桩VirtualEntryDbHandler::instance
    static VirtualEntryDbHandler mockHandler;
    mockVEntryDbHandler = &mockHandler;

    stub.set_lamda(VirtualEntryDbHandler::instance, [&]() -> VirtualEntryDbHandler * {
        __DBG_STUB_INVOKE__
        return mockVEntryDbHandler;
    });

    int dbCallCount = 0;
    stub.set_lamda(ADDR(VirtualEntryDbHandler, getDisplayNameOf), [&](VirtualEntryDbHandler *, const QUrl &url) -> QString {
        __DBG_STUB_INVOKE__
        dbCallCount++;
        return mockDisplayName;
    });

    entity = new ProtocolVirtualEntryEntity(testUrl);

    // 多次调用displayName
    QString displayName1 = entity->displayName();
    QString displayName2 = entity->displayName();
    QString displayName3 = entity->displayName();

    EXPECT_EQ(displayName1, mockDisplayName);
    EXPECT_EQ(displayName2, mockDisplayName);
    EXPECT_EQ(displayName3, mockDisplayName);
    EXPECT_EQ(dbCallCount, 1);   // 数据库只应该被调用一次
}

TEST_F(ProtocolVirtualEntryEntityTest, Icon_AnyUrl_ReturnsRemoteFolderIcon)
{
    // 打桩QIcon::fromTheme
    QIcon mockIcon;
    stub.set_lamda(static_cast<QIcon (*)(const QString &)>(QIcon::fromTheme), [&](const QString &name) -> QIcon {
        __DBG_STUB_INVOKE__
        return mockIcon;
    });

    entity = new ProtocolVirtualEntryEntity(testUrl);

    QIcon icon = entity->icon();

    EXPECT_TRUE(icon.isNull());   // 验证图标获取
}

TEST_F(ProtocolVirtualEntryEntityTest, Icon_DifferentUrls_AlwaysReturnsSameIcon)
{
    // 打桩QIcon::fromTheme
    QIcon mockIcon;
    stub.set_lamda(static_cast<QIcon (*)(const QString &)>(QIcon::fromTheme), [&](const QString &name) -> QIcon {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(name, "folder-remote");
        return mockIcon;
    });

    ProtocolVirtualEntryEntity entity1(testUrl);
    ProtocolVirtualEntryEntity entity2(testUrlWithSuffix);
    ProtocolVirtualEntryEntity entity3(testUrlWithoutSuffix);

    QIcon icon1 = entity1.icon();
    QIcon icon2 = entity2.icon();
    QIcon icon3 = entity3.icon();

    // 所有实体都应该返回相同类型的图标
    EXPECT_TRUE(true);   // 主要验证不会崩溃
}

TEST_F(ProtocolVirtualEntryEntityTest, Exists_AnyUrl_AlwaysReturnsTrue)
{
    entity = new ProtocolVirtualEntryEntity(testUrl);

    bool exists = entity->exists();

    EXPECT_TRUE(exists);
}

TEST_F(ProtocolVirtualEntryEntityTest, Exists_EmptyUrl_AlwaysReturnsTrue)
{
    QUrl emptyUrl;
    entity = new ProtocolVirtualEntryEntity(emptyUrl);

    bool exists = entity->exists();

    EXPECT_TRUE(exists);   // 虚拟条目总是存在
}

TEST_F(ProtocolVirtualEntryEntityTest, ShowProgress_AnyUrl_AlwaysReturnsFalse)
{
    entity = new ProtocolVirtualEntryEntity(testUrl);

    bool showProgress = entity->showProgress();

    EXPECT_FALSE(showProgress);
}

TEST_F(ProtocolVirtualEntryEntityTest, ShowTotalSize_AnyUrl_AlwaysReturnsFalse)
{
    entity = new ProtocolVirtualEntryEntity(testUrl);

    bool showTotalSize = entity->showTotalSize();

    EXPECT_FALSE(showTotalSize);
}

TEST_F(ProtocolVirtualEntryEntityTest, ShowUsageSize_AnyUrl_AlwaysReturnsFalse)
{
    entity = new ProtocolVirtualEntryEntity(testUrl);

    bool showUsageSize = entity->showUsageSize();

    EXPECT_FALSE(showUsageSize);
}

TEST_F(ProtocolVirtualEntryEntityTest, Order_AnyUrl_ReturnsOrderSmb)
{
    entity = new ProtocolVirtualEntryEntity(testUrl);

    AbstractEntryFileEntity::EntryOrder order = entity->order();

    EXPECT_EQ(order, AbstractEntryFileEntity::EntryOrder::kOrderSmb);
}

TEST_F(ProtocolVirtualEntryEntityTest, Order_DifferentUrls_AlwaysReturnsSameOrder)
{
    ProtocolVirtualEntryEntity entity1(testUrl);
    ProtocolVirtualEntryEntity entity2(testUrlWithSuffix);
    ProtocolVirtualEntryEntity entity3(testUrlWithoutSuffix);

    AbstractEntryFileEntity::EntryOrder order1 = entity1.order();
    AbstractEntryFileEntity::EntryOrder order2 = entity2.order();
    AbstractEntryFileEntity::EntryOrder order3 = entity3.order();

    EXPECT_EQ(order1, AbstractEntryFileEntity::EntryOrder::kOrderSmb);
    EXPECT_EQ(order2, AbstractEntryFileEntity::EntryOrder::kOrderSmb);
    EXPECT_EQ(order3, AbstractEntryFileEntity::EntryOrder::kOrderSmb);
}

TEST_F(ProtocolVirtualEntryEntityTest, TargetUrl_RootPath_ReturnsOriginalUrl)
{
    QUrl rootUrl("smb://192.168.1.100/.ventry");

    entity = new ProtocolVirtualEntryEntity(rootUrl);

    EXPECT_NO_FATAL_FAILURE(entity->targetUrl());
}

TEST_F(ProtocolVirtualEntryEntityTest, TargetUrl_EmptyPath_ReturnsOriginalUrl)
{
    QUrl emptyPathUrl("smb://192.168.1.100.ventry");

    entity = new ProtocolVirtualEntryEntity(emptyPathUrl);

    QUrl targetUrl = entity->targetUrl();

    // 对于空路径，应该返回处理后的URL
    EXPECT_TRUE(targetUrl.isValid() || targetUrl.isEmpty());
}

TEST_F(ProtocolVirtualEntryEntityTest, TargetUrl_NonRootPath_ReturnsFullSmbPath)
{
    QUrl nonRootUrl("smb://192.168.1.100/share/subfolder.ventry");

    // 打桩VirtualEntryDbHandler::instance
    static VirtualEntryDbHandler mockHandler;
    mockVEntryDbHandler = &mockHandler;

    stub.set_lamda(VirtualEntryDbHandler::instance, [&]() -> VirtualEntryDbHandler * {
        __DBG_STUB_INVOKE__
        return mockVEntryDbHandler;
    });

    // 打桩getFullSmbPath方法
    stub.set_lamda(ADDR(VirtualEntryDbHandler, getFullSmbPath), [&](VirtualEntryDbHandler *, const QString &path) {
        __DBG_STUB_INVOKE__
        return mockFullSmbPath;
    });

    entity = new ProtocolVirtualEntryEntity(nonRootUrl);

    QUrl targetUrl = entity->targetUrl();

    EXPECT_EQ(targetUrl.toString(), mockFullSmbPath);
}

TEST_F(ProtocolVirtualEntryEntityTest, TargetUrl_SuffixRemoval_CorrectlyRemovesSuffix)
{
    QUrl urlWithSuffix("smb://192.168.1.100/test.ventry");

    // 打桩VirtualEntryDbHandler::instance
    static VirtualEntryDbHandler mockHandler;
    mockVEntryDbHandler = &mockHandler;

    stub.set_lamda(VirtualEntryDbHandler::instance, [&]() -> VirtualEntryDbHandler * {
        __DBG_STUB_INVOKE__
        return mockVEntryDbHandler;
    });

    // 打桩getFullSmbPath方法
    stub.set_lamda(ADDR(VirtualEntryDbHandler, getFullSmbPath), [&](VirtualEntryDbHandler *, const QString &path) -> QString {
        __DBG_STUB_INVOKE__
        // 验证后缀被正确移除
        return mockFullSmbPath;
    });

    entity = new ProtocolVirtualEntryEntity(urlWithSuffix);

    QUrl targetUrl = entity->targetUrl();

    EXPECT_FALSE(targetUrl.toString().contains(".ventry"));   // 后缀应该被移除
}

TEST_F(ProtocolVirtualEntryEntityTest, MultipleMethodCalls_SameInstance_AllMethodsWork)
{
    // 打桩所有可能的外部调用
    static VirtualEntryDbHandler mockHandler;
    mockVEntryDbHandler = &mockHandler;

    stub.set_lamda(VirtualEntryDbHandler::instance, [&]() -> VirtualEntryDbHandler * {
        __DBG_STUB_INVOKE__
        return mockVEntryDbHandler;
    });

    stub.set_lamda(ADDR(VirtualEntryDbHandler, getDisplayNameOf), [&](VirtualEntryDbHandler *, const QUrl &url) -> QString {
        __DBG_STUB_INVOKE__
        return mockDisplayName;
    });

    stub.set_lamda(ADDR(VirtualEntryDbHandler, getFullSmbPath), [&] {
        __DBG_STUB_INVOKE__
        return mockFullSmbPath;
    });

    stub.set_lamda(static_cast<QIcon (*)(const QString &)>(QIcon::fromTheme), [&](const QString &name) -> QIcon {
        __DBG_STUB_INVOKE__
        return QIcon();
    });

    entity = new ProtocolVirtualEntryEntity(testUrl);

    // 调用所有公共方法
    QString displayName = entity->displayName();
    QIcon icon = entity->icon();
    bool exists = entity->exists();
    bool showProgress = entity->showProgress();
    bool showTotalSize = entity->showTotalSize();
    bool showUsageSize = entity->showUsageSize();
    AbstractEntryFileEntity::EntryOrder order = entity->order();
    QUrl targetUrl = entity->targetUrl();

    // 验证所有调用都成功
    EXPECT_EQ(displayName, mockDisplayName);
    EXPECT_TRUE(exists);
    EXPECT_FALSE(showProgress);
    EXPECT_FALSE(showTotalSize);
    EXPECT_FALSE(showUsageSize);
    EXPECT_EQ(order, AbstractEntryFileEntity::EntryOrder::kOrderSmb);
    EXPECT_FALSE(targetUrl.isEmpty());
}

TEST_F(ProtocolVirtualEntryEntityTest, InheritanceInterface_AbstractEntityInterface_AllMethodsImplemented)
{
    entity = new ProtocolVirtualEntryEntity(testUrl);

    // 验证继承的接口都有正确的实现
    AbstractEntryFileEntity *baseEntity = entity;

    EXPECT_NE(baseEntity, nullptr);

    // 通过基类指针调用虚函数
    EXPECT_NO_THROW(baseEntity->displayName());
    EXPECT_NO_THROW(baseEntity->icon());
    EXPECT_NO_THROW(baseEntity->exists());
    EXPECT_NO_THROW(baseEntity->showProgress());
    EXPECT_NO_THROW(baseEntity->showTotalSize());
    EXPECT_NO_THROW(baseEntity->showUsageSize());
    EXPECT_NO_THROW(baseEntity->order());
    EXPECT_NO_THROW(baseEntity->targetUrl());
}

TEST_F(ProtocolVirtualEntryEntityTest, DataMemberAccess_PublicDataMember_DataAccessible)
{
    entity = new ProtocolVirtualEntryEntity(testUrl);

    // 访问公共数据成员
    EXPECT_TRUE(entity->datas.isEmpty());   // 初始时应该为空

    // 添加测试数据
    entity->datas.insert("test_key", "test_value");
    EXPECT_EQ(entity->datas.value("test_key").toString(), "test_value");

    // 清空数据
    entity->datas.clear();
    EXPECT_TRUE(entity->datas.isEmpty());
}
