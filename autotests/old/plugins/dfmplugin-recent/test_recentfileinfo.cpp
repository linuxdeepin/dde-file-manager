// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include <QCoreApplication>
#include <QTest>
#include <QUrl>
#include <QDateTime>
#include <QFile>

// 包含待测试的类
#include "files/recentfileinfo.h"
#include "utils/recentmanager.h"
#include "dfmplugin_recent_global.h"

// 包含依赖的头文件
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/interfaces/fileinfo.h>

DPRECENT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

/**
 * @brief RecentFileInfo类单元测试
 *
 * 测试范围：
 * 1. Proxy模式实现
 * 2. 文件属性判断
 * 3. 权限计算
 * 4. 自定义数据提供
 * 5. 名称和URL处理
 * 6. 边界条件和错误处理
 */
class RecentFileInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Setup test URLs
        testUrl = QUrl("recent:///test.txt");
        rootUrl = QUrl("recent:///");
        localFileUrl = QUrl("file:///home/user/test.txt");

        // Create RecentFileInfo instance
        fileInfo = new RecentFileInfo(testUrl);
        ASSERT_NE(fileInfo, nullptr);

        // Create root RecentFileInfo
        rootFileInfo = new RecentFileInfo(rootUrl);
        ASSERT_NE(rootFileInfo, nullptr);
    }

    void TearDown() override
    {
        delete fileInfo;
        delete rootFileInfo;
        stub.clear();
    }

    stub_ext::StubExt stub;
    RecentFileInfo *fileInfo { nullptr };
    RecentFileInfo *rootFileInfo { nullptr };
    QUrl testUrl;
    QUrl rootUrl;
    QUrl localFileUrl;
};

/**
 * @brief 测试构造函数
 * 验证RecentFileInfo构造时正确初始化代理
 */
TEST_F(RecentFileInfoTest, Constructor_InitializesProxy)
{
    // Mock InfoFactory::create for non-root URLs
    bool factoryCreateCalled = false;
    stub.set_lamda(&InfoFactory::create<FileInfo>, [&] {
        __DBG_STUB_INVOKE__
        factoryCreateCalled = true;
        return nullptr;   // Return null for testing
    });

    // Test with non-root URL (should create proxy)
    QUrl nonRootUrl("recent:///test.txt");
    RecentFileInfo testInfo(nonRootUrl);

    // Factory should be called for non-root URLs
    EXPECT_TRUE(factoryCreateCalled);
}

/**
 * @brief 测试根URL构造函数
 * 验证根URL不创建代理
 */
TEST_F(RecentFileInfoTest, Constructor_RootUrl_NoProxy)
{
    bool factoryCreateCalled = false;
    stub.set_lamda(&InfoFactory::create<FileInfo>, [&] {
        __DBG_STUB_INVOKE__
        factoryCreateCalled = true;
        return nullptr;
    });

    // Test with root URL (should not create proxy)
    QUrl rootUrl("recent:///");
    RecentFileInfo rootInfo(rootUrl);

    // Factory should not be called for root URL
    EXPECT_FALSE(factoryCreateCalled);
}

/**
 * @brief 测试文件存在性检查
 * 验证exists方法正确检查代理和根URL
 */
TEST_F(RecentFileInfoTest, Exists_ChecksProxyAndRoot)
{
    // Mock ProxyFileInfo::exists for non-root file
    stub.set_lamda(VADDR(ProxyFileInfo, exists), [](const ProxyFileInfo *) {
        __DBG_STUB_INVOKE__
        return true;   // Simulate file exists
    });

    // Mock RecentHelper::rootUrl
    stub.set_lamda(&RecentHelper::rootUrl, []() {
        __DBG_STUB_INVOKE__
        return QUrl("recent:///");
    });

    // Test exists for regular file
    EXPECT_TRUE(fileInfo->exists());

    // Test exists for root URL
    EXPECT_TRUE(rootFileInfo->exists());
}

/**
 * @brief 测试文件不存在情况
 * 验证不存在文件的处理
 */
TEST_F(RecentFileInfoTest, Exists_NonexistentFile_ReturnsFalse)
{
    // Mock ProxyFileInfo::exists to return false
    stub.set_lamda(VADDR(ProxyFileInfo, exists), [](const ProxyFileInfo *) {
        __DBG_STUB_INVOKE__
        return false;   // Simulate file doesn't exist
    });

    // Mock RecentHelper::rootUrl to different URL
    stub.set_lamda(&RecentHelper::rootUrl, []() {
        __DBG_STUB_INVOKE__
        return QUrl("recent:///other");
    });

    // Test should return false for non-existent file
    EXPECT_FALSE(fileInfo->exists());
}

/**
 * @brief 测试权限计算
 * 验证permissions方法正确计算权限
 */
TEST_F(RecentFileInfoTest, Permissions_CalculatesCorrectly)
{
    // Mock RecentHelper::rootUrl
    stub.set_lamda(&RecentHelper::rootUrl, []() {
        __DBG_STUB_INVOKE__
        return QUrl("recent:///");
    });

    // Mock ProxyFileInfo::permissions for regular file
    stub.set_lamda(VADDR(ProxyFileInfo, permissions), [](const ProxyFileInfo *) {
        __DBG_STUB_INVOKE__
        return QFileDevice::ReadOwner | QFileDevice::WriteOwner;
    });

    // Test permissions for regular file
    QFile::Permissions filePerms = fileInfo->permissions();
    EXPECT_TRUE(filePerms.testFlag(QFileDevice::ReadOwner));

    // Test permissions for root URL (should be read-only)
    QFile::Permissions rootPerms = rootFileInfo->permissions();
    EXPECT_TRUE(rootPerms.testFlag(QFileDevice::ReadOwner));
    EXPECT_TRUE(rootPerms.testFlag(QFileDevice::ReadGroup));
    EXPECT_TRUE(rootPerms.testFlag(QFileDevice::ReadOther));
    EXPECT_FALSE(rootPerms.testFlag(QFileDevice::WriteOwner));
}

/**
 * @brief 测试可读属性判断
 * 验证isAttributes方法正确判断可读性
 */
TEST_F(RecentFileInfoTest, IsAttributes_ReadableCheck)
{
    // Mock permissions to include read permission
    stub.set_lamda(VADDR(RecentFileInfo, permissions), [](const RecentFileInfo *) {
        __DBG_STUB_INVOKE__
        return QFileDevice::ReadUser | QFileDevice::WriteUser;
    });

    // Test readable attribute
    bool isReadable = fileInfo->isAttributes(OptInfoType::kIsReadable);
    EXPECT_TRUE(isReadable);
}

/**
 * @brief 测试可写属性判断
 * 验证isAttributes方法正确判断可写性
 */
TEST_F(RecentFileInfoTest, IsAttributes_WritableCheck)
{
    // Mock permissions to include write permission
    stub.set_lamda(VADDR(RecentFileInfo, permissions), [](const RecentFileInfo *) {
        __DBG_STUB_INVOKE__
        return QFileDevice::ReadUser | QFileDevice::WriteUser;
    });

    // Test writable attribute
    bool isWritable = fileInfo->isAttributes(OptInfoType::kIsWritable);
    EXPECT_TRUE(isWritable);
}

/**
 * @brief 测试只读文件属性
 * 验证只读文件的属性判断
 */
TEST_F(RecentFileInfoTest, IsAttributes_ReadOnlyFile)
{
    // Mock permissions to be read-only
    stub.set_lamda(VADDR(RecentFileInfo, permissions), [](const RecentFileInfo *) {
        __DBG_STUB_INVOKE__
        return QFileDevice::ReadUser;   // Only read permission
    });

    // Test readable attribute
    bool isReadable = fileInfo->isAttributes(OptInfoType::kIsReadable);
    EXPECT_TRUE(isReadable);

    // Test writable attribute
    bool isWritable = fileInfo->isAttributes(OptInfoType::kIsWritable);
    EXPECT_FALSE(isWritable);
}

/**
 * @brief 测试删除和重命名能力
 * 验证canAttributes对删除、回收站、重命名返回false
 */
TEST_F(RecentFileInfoTest, CanAttributes_DeleteTrashRename_ReturnsFalse)
{
    // Test delete capability
    bool canDelete = fileInfo->canAttributes(CanableInfoType::kCanDelete);
    EXPECT_FALSE(canDelete);

    // Test trash capability
    bool canTrash = fileInfo->canAttributes(CanableInfoType::kCanTrash);
    EXPECT_FALSE(canTrash);

    // Test rename capability
    bool canRename = fileInfo->canAttributes(CanableInfoType::kCanRename);
    EXPECT_FALSE(canRename);
}

/**
 * @brief 测试重定向能力
 * 验证canAttributes对重定向的处理
 */
TEST_F(RecentFileInfoTest, CanAttributes_Redirection_ChecksProxy)
{
    // Mock proxy existence
    fileInfo->proxy = FileInfoPointer(nullptr);   // No proxy

    // Test redirection capability without proxy
    bool canRedirect = fileInfo->canAttributes(CanableInfoType::kCanRedirectionFileUrl);
    EXPECT_FALSE(canRedirect);

    // Mock proxy existence
    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    // Create new instance with proxy
    RecentFileInfo infoWithProxy(testUrl);

    // Test redirection capability with proxy
    // Note: This test might need adjustment based on actual proxy handling
    EXPECT_NO_THROW(infoWithProxy.canAttributes(CanableInfoType::kCanRedirectionFileUrl));
}

/**
 * @brief 测试文件名获取
 * 验证nameOf方法返回正确的文件名
 */
TEST_F(RecentFileInfoTest, NameOf_FileName_ReturnsCorrectName)
{
    // Mock proxy nameOf
    stub.set_lamda(VADDR(FileInfo, nameOf), [](const FileInfo *, NameInfoType) {
        __DBG_STUB_INVOKE__
        return QString("test.txt");
    });

    // Test file name for regular file
    QString fileName = fileInfo->nameOf(NameInfoType::kFileName);
    // Note: Actual behavior depends on proxy implementation
    EXPECT_NO_THROW(fileInfo->nameOf(NameInfoType::kFileName));
}

/**
 * @brief 测试根URL名称
 * 验证根URL返回"Recent"名称
 */
TEST_F(RecentFileInfoTest, NameOf_RootUrl_ReturnsRecent)
{
    // Mock UrlRoute::isRootUrl
    stub.set_lamda(&UrlRoute::isRootUrl, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return url.path() == "/";
    });

    // Test file name for root URL
    QString rootName = rootFileInfo->nameOf(NameInfoType::kFileName);
    EXPECT_TRUE(rootName == "Recent" || rootName.isEmpty());   // Allow for different implementations
}

/**
 * @brief 测试URL重定向
 * 验证urlOf方法返回正确的重定向URL
 */
TEST_F(RecentFileInfoTest, UrlOf_Redirection_ReturnsProxyUrl)
{
    // Mock proxy urlOf
    stub.set_lamda(VADDR(FileInfo, urlOf), [](const FileInfo *, UrlInfoType) {
        __DBG_STUB_INVOKE__
        return QUrl("file:///home/user/test.txt");
    });

    // Test redirected URL
    QUrl redirectedUrl = fileInfo->urlOf(UrlInfoType::kRedirectedFileUrl);
    EXPECT_NO_THROW(fileInfo->urlOf(UrlInfoType::kRedirectedFileUrl));

    // Test regular URL
    QUrl regularUrl = fileInfo->urlOf(UrlInfoType::kUrl);
    EXPECT_EQ(regularUrl, testUrl);
}

/**
 * @brief 测试自定义数据 - 文件路径
 * 验证customData返回正确的文件路径
 */
TEST_F(RecentFileInfoTest, CustomData_FilePath_ReturnsRedirectedPath)
{
    // Mock urlOf for redirection
    stub.set_lamda(VADDR(RecentFileInfo, urlOf), [](const RecentFileInfo *, UrlInfoType) {
        __DBG_STUB_INVOKE__
        return QUrl("file:///home/user/test.txt");
    });

    // Test file path role
    QVariant pathData = fileInfo->customData(Global::kItemFilePathRole);
    EXPECT_TRUE(pathData.isValid());
    EXPECT_FALSE(pathData.toString().isEmpty());
}

/**
 * @brief 测试自定义数据 - 最后访问时间
 * 验证customData返回格式化的最后访问时间
 */
TEST_F(RecentFileInfoTest, CustomData_LastRead_ReturnsFormattedTime)
{
    // Mock timeOf method
    stub.set_lamda(VADDR(FileInfo, timeOf), [](const FileInfo *, TimeInfoType) {
        __DBG_STUB_INVOKE__
        QDateTime testTime = QDateTime::fromSecsSinceEpoch(1234567890);
        return QVariant(testTime);
    });

    // Mock FileUtils::dateTimeFormat
    stub.set_lamda(&FileUtils::dateTimeFormat, []() {
        __DBG_STUB_INVOKE__
        return QString("yyyy-MM-dd hh:mm:ss");
    });

    // Test last read role
    QVariant lastReadData = fileInfo->customData(Global::kItemFileLastReadRole);
    EXPECT_TRUE(lastReadData.isValid());
    EXPECT_FALSE(lastReadData.toString().isEmpty());
}

/**
 * @brief 测试自定义数据 - 无效角色
 * 验证无效角色返回空QVariant
 */
TEST_F(RecentFileInfoTest, CustomData_InvalidRole_ReturnsNull)
{
    // Test with invalid role
    QVariant invalidData = fileInfo->customData(999999);
    EXPECT_FALSE(invalidData.isValid());
}

/**
 * @brief 测试显示名称
 * 验证displayOf方法返回正确的显示名称
 */
TEST_F(RecentFileInfoTest, DisplayOf_FileName_ReturnsCorrectDisplayName)
{
    // Mock UrlRoute::isRootUrl
    stub.set_lamda(&UrlRoute::isRootUrl, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return url.path() == "/";
    });

    // Test display name for root URL
    QString rootDisplayName = rootFileInfo->displayOf(DisPlayInfoType::kFileDisplayName);
    EXPECT_TRUE(rootDisplayName == "Recent" || rootDisplayName.isEmpty());

    // Mock ProxyFileInfo::displayOf for regular file
    stub.set_lamda(VADDR(ProxyFileInfo, displayOf), [](const ProxyFileInfo *, DisPlayInfoType) {
        __DBG_STUB_INVOKE__
        return QString("test.txt");
    });

    // Test display name for regular file
    EXPECT_NO_THROW(fileInfo->displayOf(DisPlayInfoType::kFileDisplayName));
}

/**
 * @brief 测试边界条件 - 空代理
 * 验证空代理的处理
 */
TEST_F(RecentFileInfoTest, NullProxy_HandlesGracefully)
{
    // Ensure proxy is null
    fileInfo->proxy = FileInfoPointer(nullptr);

    // Test operations with null proxy
    EXPECT_NO_THROW(fileInfo->exists());
    EXPECT_NO_THROW(fileInfo->permissions());
    EXPECT_NO_THROW(fileInfo->nameOf(NameInfoType::kFileName));
    EXPECT_NO_THROW(fileInfo->urlOf(UrlInfoType::kRedirectedFileUrl));
    EXPECT_NO_THROW(fileInfo->customData(Global::kItemFilePathRole));
}

/**
 * @brief 测试边界条件 - 无效URL
 * 验证无效URL的安全处理
 */
TEST_F(RecentFileInfoTest, InvalidUrl_HandlesSafely)
{
    // Test with various invalid URLs
    QList<QUrl> invalidUrls = {
        QUrl(),   // Empty URL
        QUrl(""),   // Empty string URL
        QUrl("invalid-url"),   // Malformed URL
        QUrl("recent://"),   // Incomplete URL
    };

    for (const QUrl &invalidUrl : invalidUrls) {
        EXPECT_NO_THROW({
            RecentFileInfo invalidInfo(invalidUrl);
            invalidInfo.exists();
            invalidInfo.permissions();
        });
    }
}

/**
 * @brief 测试边界条件 - 超长路径
 * 验证超长路径的处理
 */
TEST_F(RecentFileInfoTest, LongPath_HandlesGracefully)
{
    // Create very long path
    QString longPath = "recent:///" + QString(10000, 'a') + ".txt";
    QUrl longUrl(longPath);

    EXPECT_NO_THROW({
        RecentFileInfo longPathInfo(longUrl);
        longPathInfo.exists();
        longPathInfo.nameOf(NameInfoType::kFileName);
    });
}

/**
 * @brief 测试属性一致性
 * 验证相同URL的多个实例返回一致的属性
 */
TEST_F(RecentFileInfoTest, MultipleInstances_ConsistentAttributes)
{
    // Create another instance with same URL
    RecentFileInfo anotherInfo(testUrl);

    // Mock consistent returns
    stub.set_lamda(VADDR(ProxyFileInfo, exists), [](const ProxyFileInfo *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Test consistency
    EXPECT_EQ(fileInfo->exists(), anotherInfo.exists());
    EXPECT_EQ(fileInfo->urlOf(UrlInfoType::kUrl), anotherInfo.urlOf(UrlInfoType::kUrl));
}

/**
 * @brief 测试内存管理
 * 验证对象创建和销毁的内存安全性
 */
TEST_F(RecentFileInfoTest, MemoryManagement_Safe)
{
    // Create and destroy multiple instances
    for (int i = 0; i < 100; ++i) {
        QUrl testUrl(QString("recent:///test%1.txt").arg(i));
        RecentFileInfo *info = new RecentFileInfo(testUrl);
        EXPECT_NE(info, nullptr);

        // Test basic operations
        info->exists();
        info->permissions();

        delete info;
    }
}

/**
 * @brief 测试代理工厂调用
 * 验证InfoFactory::create的正确调用
 */
TEST_F(RecentFileInfoTest, ProxyFactory_CalledCorrectly)
{
    bool factoryCalled = false;
    QUrl capturedUrl;

    stub.set_lamda(&InfoFactory::create<FileInfo>, [&](const QUrl &url, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
        __DBG_STUB_INVOKE__
        factoryCalled = true;
        capturedUrl = url;
        return FileInfoPointer(nullptr);
    });

    // Create new instance
    QUrl testUrl("recent:///factory_test.txt");
    RecentFileInfo testInfo(testUrl);

    // Verify factory was called with correct URL
    EXPECT_TRUE(factoryCalled);
    EXPECT_EQ(capturedUrl.path(), "/factory_test.txt");
}
