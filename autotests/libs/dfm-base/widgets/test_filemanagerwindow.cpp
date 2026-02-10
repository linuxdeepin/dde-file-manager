// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>

#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/interfaces/abstractframe.h>
#include <QDialog>
#include "stubext.h"

using namespace dfmbase;

// Mock AbstractFrame for testing purposes
class MockAbstractFrame : public AbstractFrame {
public:
    explicit MockAbstractFrame(QWidget *parent = nullptr) : AbstractFrame(parent) {}
    void setCurrentUrl(const QUrl &url) override { Q_UNUSED(url); }
    QUrl currentUrl() const override { return QUrl(); }
};

class FileManagerWindowTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
        url = QUrl("file:///tmp");
        
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

        window = new FileManagerWindow(url);
    }

    void TearDown() override {
        stub.clear();
        delete window;
    }

    stub_ext::StubExt stub;
    QUrl url;
    FileManagerWindow *window = nullptr;
};

TEST_F(FileManagerWindowTest, Constructor_WithUrlAndParent_ExpectedWindowCreated) {
    // Arrange
    QWidget parent;

    // Act
    FileManagerWindow *tmpWindow = new FileManagerWindow(url, &parent);

    // Assert
    // Just ensure no crash during construction
    EXPECT_NE(tmpWindow, nullptr);
    delete tmpWindow;
}

TEST_F(FileManagerWindowTest, Constructor_WithUrlAndNullParent_ExpectedWindowCreated) {
    // Arrange
    // Act
    FileManagerWindow *tmpWindow = new FileManagerWindow(url, nullptr);

    // Assert
    // Just ensure no crash during construction
    EXPECT_NE(tmpWindow, nullptr);
    delete tmpWindow;
}

TEST_F(FileManagerWindowTest, CurrentUrl_WithInitialUrl_ExpectedSameUrl) {
    // Arrange

    // Act
    QUrl resultUrl = window->currentUrl();

    // Assert
    EXPECT_EQ(resultUrl, url);
}

TEST_F(FileManagerWindowTest, Cd_CallWithNewUrl_ExpectedUrlChanged) {
    // Arrange
    QUrl newUrl("file:///home");

    // Act
    window->cd(newUrl);

    // Assert
    EXPECT_EQ(window->currentUrl(), newUrl);
}

TEST_F(FileManagerWindowTest, SaveClosedSate_Call_ExpectedTrueReturned) {
    // Arrange

    // Act
    bool result = window->saveClosedSate();

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(FileManagerWindowTest, MoveCenter_Call_ExpectedNoCrash) {
    // Arrange

    // Act
    window->moveCenter();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowTest, InstallTitleBar_Call_ExpectedNoCrash) {
    // Arrange

    MockAbstractFrame *frame = new MockAbstractFrame();

    // Act
    window->installTitleBar(frame);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowTest, InstallSideBar_Call_ExpectedNoCrash) {
    // Arrange

    MockAbstractFrame *frame = new MockAbstractFrame();

    // Act
    window->installSideBar(frame);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowTest, InstallWorkSpace_Call_ExpectedNoCrash) {
    // Arrange

    MockAbstractFrame *frame = new MockAbstractFrame();

    // Act
    window->installWorkSpace(frame);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowTest, InstallDetailView_Call_ExpectedNoCrash) {
    // Arrange
    AbstractFrame *frame = nullptr;  // Use null to avoid complex setup

    // Act
    window->installDetailView(frame);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowTest, TitleBar_Call_ExpectedFrameReturned) {
    // Arrange

    // Act
    AbstractFrame *result = window->titleBar();

    // Assert
    EXPECT_EQ(result, nullptr);  // Initially should be null
}

TEST_F(FileManagerWindowTest, SideBar_Call_ExpectedFrameReturned) {
    // Arrange

    // Act
    AbstractFrame *result = window->sideBar();

    // Assert
    EXPECT_EQ(result, nullptr);  // Initially should be null
}

TEST_F(FileManagerWindowTest, WorkSpace_Call_ExpectedFrameReturned) {
    // Arrange

    // Act
    AbstractFrame *result = window->workSpace();

    // Assert
    EXPECT_EQ(result, nullptr);  // Initially should be null
}

TEST_F(FileManagerWindowTest, DetailView_Call_ExpectedFrameReturned) {
    // Arrange

    // Act
    AbstractFrame *result = window->detailView();

    // Assert
    EXPECT_EQ(result, nullptr);  // Initially should be null
}

TEST_F(FileManagerWindowTest, LoadState_Call_ExpectedNoCrash) {
    // Arrange

    // Act
    window->loadState();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(FileManagerWindowTest, SaveState_Call_ExpectedNoCrash) {
    // Arrange
    // Act
    window->saveState();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}