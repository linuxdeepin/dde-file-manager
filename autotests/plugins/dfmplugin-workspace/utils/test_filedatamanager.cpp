// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "models/rootinfo.h"
#include "stubext.h"

#include "utils/filedatamanager.h"
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/device/deviceproxymanager.h>

#include <QUrl>
#include <QDebug>

using namespace dfmplugin_workspace;

class FileDataManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        stub.set_lamda(&Application::appAttribute, []() {
            return QVariant(true);
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(FileDataManagerTest, Instance_ReturnsSameInstance)
{
    // Test that instance() returns the same singleton instance
    FileDataManager *instance1 = FileDataManager::instance();
    FileDataManager *instance2 = FileDataManager::instance();
    
    EXPECT_EQ(instance1, instance2);
}

TEST_F(FileDataManagerTest, FetchRoot_ValidUrl_ReturnsRootInfo)
{
    // Test fetching root info for a valid URL
    FileDataManager *manager = FileDataManager::instance();
    QUrl testUrl("file:///tmp/test");
    
    // Mock the createRoot method
    RootInfo *mockRoot = reinterpret_cast<RootInfo*>(0x12345);
    stub.set_lamda(ADDR(FileDataManager, createRoot), [mockRoot]() {
        return mockRoot;
    });
    
    RootInfo *result = manager->fetchRoot(testUrl);
    
    EXPECT_EQ(result, mockRoot);
}

TEST_F(FileDataManagerTest, FetchRoot_ExistingUrl_ReturnsExistingRootInfo)
{
    // Test fetching root info for an existing URL
    FileDataManager *manager = FileDataManager::instance();
    QUrl testUrl("file:///tmp/test");
    
    // First, create a root info
    RootInfo *mockRoot = reinterpret_cast<RootInfo*>(0x12345);
    stub.set_lamda(ADDR(FileDataManager, createRoot), [mockRoot]() {
        return mockRoot;
    });
    
    // Call fetchRoot twice - second call should return the existing root
    RootInfo *result1 = manager->fetchRoot(testUrl);
    RootInfo *result2 = manager->fetchRoot(testUrl);
    
    EXPECT_EQ(result1, result2);
    EXPECT_EQ(result1, mockRoot);
}

TEST_F(FileDataManagerTest, FetchFiles_ValidRootUrl_ReturnsTrue)
{
    // Test fetching files for a valid root URL
    FileDataManager *manager = FileDataManager::instance();
    QUrl testUrl("file:///tmp/test");
    QString testKey("test_key");
    
    // Mock root info
    RootInfo *mockRoot = reinterpret_cast<RootInfo*>(0x12345);
    stub.set_lamda(ADDR(FileDataManager, createRoot), [mockRoot]() {
        return mockRoot;
    });
    
    // Mock RootInfo methods
    stub.set_lamda(ADDR(RootInfo, initThreadOfFileData), []() {
        return true;
    });
    
    stub.set_lamda(ADDR(RootInfo, startWork), []() {
        // Do nothing
    });
    
    bool result = manager->fetchFiles(testUrl, testKey);
    
    EXPECT_TRUE(result);
}

TEST_F(FileDataManagerTest, FetchFiles_InvalidRootUrl_ReturnsFalse)
{
    // Test fetching files for an invalid root URL (no RootInfo)
    FileDataManager *manager = FileDataManager::instance();
    QUrl testUrl("file:///tmp/nonexistent");
    QString testKey("test_key");
    
    // Don't mock createRoot, so fetchRoot will return nullptr
    
    bool result = manager->fetchFiles(testUrl, testKey);
    
    EXPECT_FALSE(result);
}

TEST_F(FileDataManagerTest, CleanRoot_ValidUrl_CleansRoot)
{
    // Test cleaning root for a valid URL
    FileDataManager *manager = FileDataManager::instance();
    QUrl testUrl("file:///tmp/test");
    QString testKey("test_key");
    
    // Mock root info
    RootInfo *mockRoot = reinterpret_cast<RootInfo*>(0x12345);
    stub.set_lamda(ADDR(FileDataManager, createRoot), [mockRoot]() {
        return mockRoot;
    });
    
    // Mock RootInfo methods
    stub.set_lamda(ADDR(RootInfo, clearTraversalThread), []() {
        return 0; // Return 0 to indicate no remaining threads
    });
    
    stub.set_lamda(ADDR(FileDataManager, checkNeedCache), []() {
        return false; // Don't need cache
    });
    
    stub.set_lamda(ADDR(FileDataManager, handleDeletion), []() {
        // Do nothing
    });
    
    // This should not crash
    manager->cleanRoot(testUrl, testKey);
}

TEST_F(FileDataManagerTest, CleanRootComplete_ValidUrl_CompletelyCleansRoot)
{
    // Test complete root cleanup for a valid URL
    FileDataManager *manager = FileDataManager::instance();
    QUrl testUrl("file:///tmp/test");
    
    // Mock root info
    RootInfo *mockRoot = reinterpret_cast<RootInfo*>(0x12345);
    stub.set_lamda(ADDR(FileDataManager, createRoot), [mockRoot]() {
        return mockRoot;
    });
    
    stub.set_lamda(ADDR(FileDataManager, handleDeletion), []() {
        // Do nothing
    });
    
    // This should not crash
    manager->cleanRoot(testUrl);
}

TEST_F(FileDataManagerTest, SetFileActive_ValidUrl_SetsFileActive)
{
    // Test setting file active state
    FileDataManager *manager = FileDataManager::instance();
    QUrl rootUrl("file:///tmp/test");
    QUrl childUrl("file:///tmp/test/file.txt");
    
    // Mock root info
    RootInfo *mockRoot = reinterpret_cast<RootInfo*>(0x12345);
    stub.set_lamda(ADDR(FileDataManager, createRoot), [mockRoot]() {
        return mockRoot;
    });
    
    // Mock RootInfo methods
    stub.set_lamda(ADDR(RootInfo, clearTraversalThread), []() {
        return 0;
    });
    
    // Mock watcher
    class MockWatcher {
    public:
        void setEnabledSubfileWatcher(const QUrl &, bool) {}
    };
    
    MockWatcher *mockWatcher = new MockWatcher();
    stub.set_lamda(ADDR(RootInfo, clearTraversalThread), [mockWatcher]() {
        return 0;
    });
    
    // This should not crash
    manager->setFileActive(rootUrl, childUrl, true);
}

TEST_F(FileDataManagerTest, OnAppAttributeChanged_FileAndDirMixedSort_UpdatesMixedSort)
{
    // Test handling app attribute change for file and dir mixed sort
    FileDataManager *manager = FileDataManager::instance();
    
    // This should not crash
    manager->onAppAttributeChanged(DFMBASE_NAMESPACE::Application::kFileAndDirMixedSort, true);
}

TEST_F(FileDataManagerTest, OnHandleFileDeleted_ValidUrl_CleansRoot)
{
    // Test handling file deletion
    FileDataManager *manager = FileDataManager::instance();
    QUrl testUrl("file:///tmp/test/file.txt");
    
    // Mock the cleanRoot method with specific signature
    using CleanRootFunc = void (FileDataManager::*)(const QUrl &);
    stub.set_lamda(static_cast<CleanRootFunc>(&FileDataManager::cleanRoot), [](FileDataManager *, const QUrl &) {
        // Do nothing
    });
    
    // This should not crash
    manager->onHandleFileDeleted(testUrl);
}

TEST_F(FileDataManagerTest, CheckNeedCache_FileScheme_ReturnsFalse)
{
    // Test cache need check for file scheme
    FileDataManager *manager = FileDataManager::instance();
    QUrl testUrl("file:///tmp/test");
    
    // This should return false for local file scheme
    bool result = manager->checkNeedCache(testUrl);
    
    EXPECT_FALSE(result);
}

TEST_F(FileDataManagerTest, CheckNeedCache_NonFileScheme_ReturnsTrue)
{
    // Test cache need check for non-file scheme
    FileDataManager *manager = FileDataManager::instance();
    QUrl testUrl("ftp://server/path");
    
    // This should return true for non-local file scheme
    bool result = manager->checkNeedCache(testUrl);
    
    EXPECT_TRUE(result);
}
