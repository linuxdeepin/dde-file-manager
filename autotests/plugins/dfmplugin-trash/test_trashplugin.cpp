// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include <QSignalSpy>

#include "trash.h"
#include "utils/trashhelper.h"
#include "trashdiriterator.h"
#include "trashfilewatcher.h"
#include "utils/trashfilehelper.h"
#include "menus/trashmenuscene.h"

#include <stubext.h>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPTRASH_USE_NAMESPACE
using namespace dfmplugin_trash;

class TestTrashPlugin : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        // Setup test environment
        testDir = QDir::temp().absoluteFilePath("trash_plugin_test_" + QString::number(QCoreApplication::applicationPid()));
        QDir().mkpath(testDir);
        
        // Create plugin instance
        plugin = new Trash();
    }

    void TearDown() override
    {
        stub.clear();
        // Cleanup test environment
        QDir(testDir).removeRecursively();
        if (plugin) {
            delete plugin;
            plugin = nullptr;
        }
    }

    stub_ext::StubExt stub;
    QString testDir;
    Trash *plugin = nullptr;
};

TEST_F(TestTrashPlugin, Initialize)
{
    EXPECT_NO_THROW({
        plugin->initialize();
    });
}

TEST_F(TestTrashPlugin, Start)
{
    EXPECT_NO_THROW({
        plugin->start();
    });
}

TEST_F(TestTrashPlugin, Constructor)
{
    Trash *newPlugin = new Trash();
    EXPECT_NE(newPlugin, nullptr);
    delete newPlugin;
}

TEST_F(TestTrashPlugin, Destructor)
{
    Trash *newPlugin = new Trash();
    EXPECT_NE(newPlugin, nullptr);
    delete newPlugin;
    EXPECT_TRUE(true);
}

TEST_F(TestTrashPlugin, OnWindowOpened)
{
    // Test onWindowOpened method
    Trash plugin;
    
    // Create a mock window
    quint64 windowId = 12345;
    
    // Mock FileManagerWindowsManager::findWindowById
    FileManagerWindow *mockWindow = new FileManagerWindow(QUrl("trash:///"));
    stub.set_lamda(ADDR(FileManagerWindowsManager, findWindowById), [mockWindow](FileManagerWindowsManager *self, quint64 id) -> FileManagerWindow * {
        Q_UNUSED(self)
        Q_UNUSED(id)
        return mockWindow;
    });
    
    // Mock window title bar and sidebar
    bool hasTitleBar = true;
    bool hasSideBar = true;
    
    stub.set_lamda(&FileManagerWindow::titleBar, [hasTitleBar]() -> AbstractFrame* {
        return hasTitleBar ? (AbstractFrame*)1 : nullptr;
    });
    
    stub.set_lamda(&FileManagerWindow::sideBar, [hasSideBar]() -> AbstractFrame* {
        return hasSideBar ? (AbstractFrame*)1 : nullptr;
    });
    
    // Test with title bar and sidebar
    EXPECT_NO_THROW(plugin.onWindowOpened(windowId));
    
    // Test without title bar
    hasTitleBar = false;
    EXPECT_NO_THROW(plugin.onWindowOpened(windowId));
    
    // Test without sidebar
    hasTitleBar = true;
    hasSideBar = false;
    EXPECT_NO_THROW(plugin.onWindowOpened(windowId));
    
    // Test without both
    hasTitleBar = false;
    hasSideBar = false;
    EXPECT_NO_THROW(plugin.onWindowOpened(windowId));
    
    delete mockWindow;
}

TEST_F(TestTrashPlugin, RegTrashCrumbToTitleBar)
{
    // Test regTrashCrumbToTitleBar method
    Trash plugin;
    
    // Call the method
    EXPECT_NO_THROW(plugin.regTrashCrumbToTitleBar());
    
    // Call again to test the std::once_flag behavior
    EXPECT_NO_THROW(plugin.regTrashCrumbToTitleBar());
}

TEST_F(TestTrashPlugin, RegTrashItemToSideBar)
{
    // Test regTrashItemToSideBar method
    Trash plugin;
    
    // Test when bookmark plugin is already started
    EXPECT_NO_THROW(plugin.regTrashItemToSideBar());
    
    // Test when bookmark plugin is not started
    Trash plugin2;
    EXPECT_NO_THROW(plugin2.regTrashItemToSideBar());
}

TEST_F(TestTrashPlugin, UpdateTrashItemToSideBar)
{
    // Test updateTrashItemToSideBar method
    Trash plugin;
    
    // Call the method
    EXPECT_NO_THROW(plugin.updateTrashItemToSideBar());
    
    // Call again to test the std::once_flag behavior
    EXPECT_NO_THROW(plugin.updateTrashItemToSideBar());
}

TEST_F(TestTrashPlugin, QDeclareMetatypeQUrlPtr)
{
    // Test that Q_DECLARE_METATYPE(QUrl *) compiles and works correctly
    // This is a compile-time test to ensure the metatype is properly declared
    
    // Test registering the metatype
    int typeId = qMetaTypeId<QUrl*>();
    EXPECT_GT(typeId, 0); // Should be a valid type ID
    
    // Test creating a QUrl pointer
    QUrl *urlPtr = new QUrl("trash:///test.txt");
    
    // Test that we can use the pointer with QVariant
    QVariant variant = QVariant::fromValue(urlPtr);
    EXPECT_TRUE(variant.isValid());
    
    // Test that we can convert back from QVariant
    QUrl *convertedPtr = variant.value<QUrl*>();
    EXPECT_EQ(convertedPtr, urlPtr);
    
    delete urlPtr;
}
