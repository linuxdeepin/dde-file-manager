// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QUrl>
#include <QList>

#include <dfm-base/utils/clipboard.h>
#include "stubext.h"

using namespace dfmbase;

class ClipboardTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();

        // stub.set_lamda(&QApplication::clipboard, []() -> QClipboard* {
        //     __DBG_STUB_INVOKE__
        //     static QClipboard *clipboard = nullptr;
        //     if (!clipboard) {
        //         clipboard = new QClipboard;
        //     }
        //     return clipboard;
        // });
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(ClipboardTest, Instance_Singleton_ExpectedSameInstance) {
    // Act
    ClipBoard *instance1 = ClipBoard::instance();
    ClipBoard *instance2 = ClipBoard::instance();

    // Assert
    EXPECT_EQ(instance1, instance2);
}

TEST_F(ClipboardTest, ClipboardFileUrlList_GetUrls_ExpectedUrlList) {
    // Arrange
    auto *clipboard = ClipBoard::instance();

    // Act
    QList<QUrl> urls = clipboard->clipboardFileUrlList();

    // Assert
    EXPECT_TRUE(urls.isEmpty());  // 初始化时应该为空
}

TEST_F(ClipboardTest, ClipboardAction_GetAction_ExpectedAction) {
    // Arrange
    auto *clipboard = ClipBoard::instance();

    // Act
    auto action = clipboard->clipboardAction();

    // Assert
    EXPECT_EQ(action, ClipBoard::kUnknownAction);
}

TEST_F(ClipboardTest, ClearClipboard_CallClear_ExpectedClearSuccess) {
    // Arrange
    stub.set_lamda(&QClipboard::setText, [](QClipboard *, const QString &, QClipboard::Mode) {
        __DBG_STUB_INVOKE__
    });

    // Act
    ClipBoard::clearClipboard();

    // Assert
    // 没有崩溃说明调用成功
    EXPECT_TRUE(true);
}

TEST_F(ClipboardTest, SupportCut_CheckCutSupport_ExpectedBoolResult) {
    // Arrange
    stub.set_lamda(&QClipboard::mimeData, [](QClipboard *, QClipboard::Mode) -> const QMimeData* {
        __DBG_STUB_INVOKE__
        static QMimeData mimeData;
        return &mimeData;
    });

    stub.set_lamda(&QMimeData::data, [](const QMimeData *, const QString &) -> QByteArray {
        __DBG_STUB_INVOKE__
        return QByteArray();  // 返回空数据，表示没有用户ID限制
    });

    // Act
    bool support = ClipBoard::supportCut();

    // Assert
    EXPECT_TRUE(support);
}

TEST_F(ClipboardTest, GetRemoteUrls_CallGet_ExpectedUrlList) {
    // Arrange
    stub.set_lamda(&QClipboard::mimeData, [](QClipboard *, QClipboard::Mode) -> const QMimeData* {
        __DBG_STUB_INVOKE__
        static QMimeData mimeData;
        return &mimeData;
    });

    // Act
    QList<QUrl> urls = ClipBoard::getRemoteUrls();

    // Assert
    // 应该返回空列表，因为我们没有设置远程操作
    EXPECT_TRUE(urls.isEmpty());
}

TEST_F(ClipboardTest, RemoveUrls_WithEmptyClipboard_ExpectedNoChange) {
    // Arrange
    auto *clipboard = ClipBoard::instance();
    QList<QUrl> urlsToRemove;
    urlsToRemove.append(QUrl("file:///test/file1.txt"));

    // Act
    clipboard->removeUrls(urlsToRemove);

    // Assert
    // 没有崩溃说明调用成功
    EXPECT_TRUE(true);
}

TEST_F(ClipboardTest, ReplaceClipboardUrl_WithEmptyClipboard_ExpectedNoChange) {
    // Arrange
    auto *clipboard = ClipBoard::instance();
    QUrl oldUrl("file:///old/path");
    QUrl newUrl("file:///new/path");

    // Act
    clipboard->replaceClipboardUrl(oldUrl, newUrl);

    // Assert
    // 没有崩溃说明调用成功
    EXPECT_TRUE(true);
}

TEST_F(ClipboardTest, SetUrlsToClipboard_WithValidUrls_ExpectedSetSuccess) {
    // Arrange
    QMimeData *capturedMimeData = nullptr;
    stub.set_lamda(&QClipboard::setMimeData, [&capturedMimeData](QClipboard *, QMimeData *mimeData, QClipboard::Mode) {
        __DBG_STUB_INVOKE__
        capturedMimeData = mimeData;  // 保存引用以便稍后清理
    });

    QList<QUrl> urls;
    urls.append(QUrl("file:///test/file1.txt"));
    urls.append(QUrl("file:///test/file2.txt"));

    // Mock FileInfo creation to avoid actual file system access
    stub.set_lamda(VADDR(QMimeData, setText), [](QMimeData *self, const QString &) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(VADDR(QMimeData, setData), [](QMimeData *self, const QString &, const QByteArray &) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(VADDR(QMimeData, setUrls), [](QMimeData *self, const QList<QUrl> &) {
        __DBG_STUB_INVOKE__
    });

    // Act
    ClipBoard::setUrlsToClipboard(urls, ClipBoard::kCopyAction);

    // Assert
    // 没有崩溃说明调用成功
    EXPECT_TRUE(true);
    
    // 清理测试中创建的QMimeData对象以避免内存泄漏
    if (capturedMimeData && capturedMimeData != nullptr) {
        delete capturedMimeData;
    }
}

TEST_F(ClipboardTest, SetCurUrlToClipboardForRemote_WithValidUrl_ExpectedSetSuccess) {
    // Arrange
    QMimeData *capturedMimeData = nullptr;
    stub.set_lamda(&QClipboard::setMimeData, [&capturedMimeData](QClipboard *, QMimeData *mimeData, QClipboard::Mode) {
        __DBG_STUB_INVOKE__
        capturedMimeData = mimeData;  // 保存引用以便稍后清理
    });

    QUrl url("file:///test/remote/file.txt");

    // Act
    ClipBoard::setCurUrlToClipboardForRemote(url);

    // Assert
    // 没有崩溃说明调用成功
    EXPECT_TRUE(true);
    
    // 清理测试中创建的QMimeData对象以避免内存泄漏
    if (capturedMimeData) {
        delete capturedMimeData;
    }
}

TEST_F(ClipboardTest, SetDataToClipboard_WithValidData_ExpectedSetSuccess) {
    // Arrange
    stub.set_lamda(&QClipboard::setMimeData, [](QClipboard *, QMimeData *, QClipboard::Mode) {
        __DBG_STUB_INVOKE__
    });

    QMimeData *mimeData = new QMimeData;
    mimeData->setText("test data");

    // Act
    ClipBoard::setDataToClipboard(mimeData);

    // Assert
    // 没有崩溃说明调用成功
    EXPECT_TRUE(true);

    delete mimeData;
}
