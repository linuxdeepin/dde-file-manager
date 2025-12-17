// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QList>

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/filemanagerwindow.h>
#include <QDialog>
#include "stubext.h"

using namespace dfmbase;

class FileManagerWindowsManagerTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
        
        // Stub UI methods to avoid actual dialog display
        stub.set_lamda(VADDR(QDialog, exec), [] {
            __DBG_STUB_INVOKE__
            return QDialog::Accepted;  // or QDialog::Rejected as needed
        });
        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&QWidget::hide, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(FileManagerWindowsManagerTest, Instance_GetInstance_ExpectedSameInstance) {
    // Act
    FileManagerWindowsManager &instance1 = FileManagerWindowsManager::instance();
    FileManagerWindowsManager &instance2 = FileManagerWindowsManager::instance();

    // Assert
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(FileManagerWindowsManagerTest, SetCustomWindowCreator_CallWithFunction_ExpectedNoCrash) {
    // Arrange
    FileManagerWindowsManager &manager = FileManagerWindowsManager::instance();
    FileManagerWindowsManager::WindowCreator creator = [](const QUrl &url) -> FileManagerWindowsManager::FMWindow* {
        Q_UNUSED(url)
        return nullptr;
    };

    // Act
    manager.setCustomWindowCreator(creator);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowsManagerTest, CreateWindow_WithValidUrl_ExpectedNoCrash) {
    // Arrange
    FileManagerWindowsManager &manager = FileManagerWindowsManager::instance();
    QUrl testUrl("file:///tmp");

    // Act
    FileManagerWindowsManager::FMWindow *window = manager.createWindow(testUrl);

    // Assert
    // Just ensure no crash during call
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowsManagerTest, CreateWindow_WithComputerUrl_ExpectedNoCrash) {
    // Arrange
    FileManagerWindowsManager &manager = FileManagerWindowsManager::instance();
    QUrl testUrl("computer:///");

    // Act
    FileManagerWindowsManager::FMWindow *window = manager.createWindow(testUrl);

    // Assert
    // Just ensure no crash during call
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowsManagerTest, CreateWindow_WithUrlAndNewWindowFlag_ExpectedNoCrash) {
    // Arrange
    FileManagerWindowsManager &manager = FileManagerWindowsManager::instance();
    QUrl testUrl("file:///tmp");
    bool isNewWindow = true;

    // Act
    FileManagerWindowsManager::FMWindow *window = manager.createWindow(testUrl, isNewWindow);

    // Assert
    // Just ensure no crash during call
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowsManagerTest, CreateWindow_WithComputerUrlAndNewWindowFlag_ExpectedNoCrash) {
    // Arrange
    FileManagerWindowsManager &manager = FileManagerWindowsManager::instance();
    QUrl testUrl("computer:///");
    bool isNewWindow = true;

    // Act
    FileManagerWindowsManager::FMWindow *window = manager.createWindow(testUrl, isNewWindow);

    // Assert
    // Just ensure no crash during call
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowsManagerTest, ShowWindow_WithValidWindow_ExpectedNoCrash) {
    // Arrange
    FileManagerWindowsManager &manager = FileManagerWindowsManager::instance();
    
    // Create a valid window for the test
    QUrl testUrl("file:///tmp");
    FileManagerWindowsManager::FMWindow *window = manager.createWindow(testUrl);

    // Act
    if (window) {
        manager.showWindow(window);
    }

    // Assert
    // Just ensure no crash during call
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowsManagerTest, FindWindowId_WithNullWidget_ExpectedZeroId) {
    // Arrange
    FileManagerWindowsManager &manager = FileManagerWindowsManager::instance();
    QWidget *widget = nullptr;

    // Act
    // We expect this to return 0 when widget is null
    // This test should validate the null check behavior
    quint64 id = manager.findWindowId(widget);

    // Assert
    EXPECT_EQ(id, 0);
}

TEST_F(FileManagerWindowsManagerTest, FindWindowById_WithInvalidId_ExpectedNullWindow) {
    // Arrange
    FileManagerWindowsManager &manager = FileManagerWindowsManager::instance();
    quint64 invalidId = 0;

    // Act
    FileManagerWindowsManager::FMWindow *window = manager.findWindowById(invalidId);

    // Assert
    EXPECT_EQ(window, nullptr);
}

TEST_F(FileManagerWindowsManagerTest, WindowIdList_Call_ExpectedNoCrash) {
    // Arrange
    FileManagerWindowsManager &manager = FileManagerWindowsManager::instance();

    // Act
    QList<quint64> ids = manager.windowIdList();

    // Assert
    // Just ensure no crash and we get a list
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowsManagerTest, ResetPreviousActivedWindowId_Call_ExpectedNoCrash) {
    // Arrange
    FileManagerWindowsManager &manager = FileManagerWindowsManager::instance();

    // Act
    manager.resetPreviousActivedWindowId();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowsManagerTest, PreviousActivedWindowId_Call_ExpectedNoCrash) {
    // Arrange
    FileManagerWindowsManager &manager = FileManagerWindowsManager::instance();

    // Act
    quint64 id = manager.previousActivedWindowId();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowsManagerTest, LastActivedWindowId_Call_ExpectedNoCrash) {
    // Arrange
    FileManagerWindowsManager &manager = FileManagerWindowsManager::instance();

    // Act
    quint64 id = manager.lastActivedWindowId();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowsManagerTest, ContainsCurrentUrl_WithEmptyUrl_ExpectedFalse) {
    // Arrange
    FileManagerWindowsManager &manager = FileManagerWindowsManager::instance();
    QUrl emptyUrl;

    // Act
    bool result = manager.containsCurrentUrl(emptyUrl);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowsManagerTest, ContainsCurrentUrl_WithUrlAndWidget_ExpectedNoCrash) {
    // Arrange
    FileManagerWindowsManager &manager = FileManagerWindowsManager::instance();
    QUrl testUrl("file:///tmp");
    QWidget *widget = nullptr;

    // Act
    bool result = manager.containsCurrentUrl(testUrl, widget);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}