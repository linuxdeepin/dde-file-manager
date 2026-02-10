// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/desktop/ddplugin-canvas/utils/fileutil.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/desktopfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <QUrl>
#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

class UT_DesktopFileCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = DesktopFileCreator::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    DesktopFileCreator *creator = nullptr;
};

TEST_F(UT_DesktopFileCreator, instance)
{
    // Test singleton pattern
    EXPECT_NE(creator, nullptr);
    
    // Should return the same instance
    DesktopFileCreator *creator2 = DesktopFileCreator::instance();
    EXPECT_EQ(creator, creator2);
}

TEST_F(UT_DesktopFileCreator, createFileInfo_ValidUrl)
{
    bool infoFactoryCreateCalled = false;
    QUrl capturedUrl;
    dfmbase::Global::CreateFileInfoType capturedCacheType;
    
    // Stub InfoFactory::create to avoid complex file system operations
    stub.set_lamda(static_cast<FileInfoPointer(*)(const QUrl&, dfmbase::Global::CreateFileInfoType, QString*)>(&InfoFactory::create<FileInfo>), 
                   [&infoFactoryCreateCalled, &capturedUrl, &capturedCacheType](const QUrl &url, dfmbase::Global::CreateFileInfoType cache, QString *errString) -> FileInfoPointer {
        __DBG_STUB_INVOKE__
        infoFactoryCreateCalled = true;
        capturedUrl = url;
        capturedCacheType = cache;
        if (errString) *errString = "";
        
        // Return a mock FileInfo to simulate successful creation
        // Use SyncFileInfo to avoid potential recursion with DesktopFileInfo
        return QSharedPointer<FileInfo>(new SyncFileInfo(url));
    });
    
    QUrl testUrl("file:///tmp/test.txt");
    dfmbase::Global::CreateFileInfoType cacheType = dfmbase::Global::CreateFileInfoType::kCreateFileInfoAuto;
    
    FileInfoPointer result = creator->createFileInfo(testUrl, cacheType);
    
    EXPECT_TRUE(infoFactoryCreateCalled);
    EXPECT_EQ(capturedUrl, testUrl);
    EXPECT_EQ(capturedCacheType, cacheType);
    EXPECT_NE(result, nullptr);
}

TEST_F(UT_DesktopFileCreator, createFileInfo_InfoFactoryFails)
{
    bool infoFactoryCreateCalled = false;
    QString testErrorString = "Mock creation failed";
    
    // Stub InfoFactory::create to return nullptr (failure case)
    stub.set_lamda(static_cast<FileInfoPointer(*)(const QUrl&, dfmbase::Global::CreateFileInfoType, QString*)>(&InfoFactory::create<FileInfo>), 
                   [&infoFactoryCreateCalled, &testErrorString](const QUrl &url, dfmbase::Global::CreateFileInfoType cache, QString *errString) -> FileInfoPointer {
        __DBG_STUB_INVOKE__
        infoFactoryCreateCalled = true;
        if (errString) *errString = testErrorString;
        return nullptr; // Simulate failure
    });
    
    QUrl testUrl("file:///invalid/path");
    
    FileInfoPointer result = creator->createFileInfo(testUrl);
    
    EXPECT_TRUE(infoFactoryCreateCalled);
    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_DesktopFileCreator, createFileInfo_DefaultCacheType)
{
    bool infoFactoryCreateCalled = false;
    dfmbase::Global::CreateFileInfoType capturedCacheType;
    
    // Stub InfoFactory::create
    stub.set_lamda(static_cast<FileInfoPointer(*)(const QUrl&, dfmbase::Global::CreateFileInfoType, QString*)>(&InfoFactory::create<FileInfo>), 
                   [&infoFactoryCreateCalled, &capturedCacheType](const QUrl &url, dfmbase::Global::CreateFileInfoType cache, QString *errString) -> FileInfoPointer {
        __DBG_STUB_INVOKE__
        infoFactoryCreateCalled = true;
        capturedCacheType = cache;
        if (errString) *errString = "";
        return QSharedPointer<FileInfo>(new SyncFileInfo(url));
    });
    
    QUrl testUrl("file:///tmp/test.txt");
    
    // Test with default cache type parameter
    FileInfoPointer result = creator->createFileInfo(testUrl);
    
    EXPECT_TRUE(infoFactoryCreateCalled);
    EXPECT_EQ(capturedCacheType, dfmbase::Global::CreateFileInfoType::kCreateFileInfoAuto);
    EXPECT_NE(result, nullptr);
}

TEST_F(UT_DesktopFileCreator, createFileInfo_DifferentCacheTypes)
{
    bool infoFactoryCreateCalled = false;
    dfmbase::Global::CreateFileInfoType capturedCacheType;
    
    // Stub InfoFactory::create
    stub.set_lamda(static_cast<FileInfoPointer(*)(const QUrl&, dfmbase::Global::CreateFileInfoType, QString*)>(&InfoFactory::create<FileInfo>), 
                   [&infoFactoryCreateCalled, &capturedCacheType](const QUrl &url, dfmbase::Global::CreateFileInfoType cache, QString *errString) -> FileInfoPointer {
        __DBG_STUB_INVOKE__
        infoFactoryCreateCalled = true;
        capturedCacheType = cache;
        if (errString) *errString = "";
        return QSharedPointer<FileInfo>(new SyncFileInfo(url));
    });
    
    QUrl testUrl("file:///tmp/test.txt");
    
    // Test with specific cache type
    FileInfoPointer result = creator->createFileInfo(testUrl, dfmbase::Global::CreateFileInfoType::kCreateFileInfoSync);
    
    EXPECT_TRUE(infoFactoryCreateCalled);
    EXPECT_EQ(capturedCacheType, dfmbase::Global::CreateFileInfoType::kCreateFileInfoSync);
    EXPECT_NE(result, nullptr);
}

TEST_F(UT_DesktopFileCreator, createFileInfo_DifferentUrlTypes)
{
    bool infoFactoryCreateCalled = false;
    QUrl capturedUrl;
    
    // Stub InfoFactory::create
    stub.set_lamda(static_cast<FileInfoPointer(*)(const QUrl&, dfmbase::Global::CreateFileInfoType, QString*)>(&InfoFactory::create<FileInfo>), 
                   [&infoFactoryCreateCalled, &capturedUrl](const QUrl &url, dfmbase::Global::CreateFileInfoType cache, QString *errString) -> FileInfoPointer {
        __DBG_STUB_INVOKE__
        infoFactoryCreateCalled = true;
        capturedUrl = url;
        if (errString) *errString = "";
        return QSharedPointer<FileInfo>(new SyncFileInfo(url));
    });
    
    // Test with different URL types
    QUrl desktopUrl("file:///home/user/Desktop/test.desktop");
    QUrl documentUrl("file:///home/user/Documents/test.txt");
    QUrl trashUrl("trash:///test.txt");
    
    // Test desktop file
    FileInfoPointer result1 = creator->createFileInfo(desktopUrl);
    EXPECT_TRUE(infoFactoryCreateCalled);
    EXPECT_EQ(capturedUrl, desktopUrl);
    EXPECT_NE(result1, nullptr);
    
    // Reset flag
    infoFactoryCreateCalled = false;
    
    // Test document file
    FileInfoPointer result2 = creator->createFileInfo(documentUrl);
    EXPECT_TRUE(infoFactoryCreateCalled);
    EXPECT_EQ(capturedUrl, documentUrl);
    EXPECT_NE(result2, nullptr);
    
    // Reset flag
    infoFactoryCreateCalled = false;
    
    // Test trash file
    FileInfoPointer result3 = creator->createFileInfo(trashUrl);
    EXPECT_TRUE(infoFactoryCreateCalled);
    EXPECT_EQ(capturedUrl, trashUrl);
    EXPECT_NE(result3, nullptr);
}