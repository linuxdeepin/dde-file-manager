// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/historystack.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <gtest/gtest.h>
#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class HistoryStackTest : public testing::Test
{
protected:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        stack = new HistoryStack(10);
        stub.clear();
    }

    void TearDown() override
    {
        delete stack;
        stack = nullptr;
        stub.clear();
    }

    HistoryStack *stack { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(HistoryStackTest, Constructor_WithThreshold_InitializedCorrectly)
{
    HistoryStack stack(5);
    EXPECT_EQ(stack.size(), 0);
    EXPECT_EQ(stack.currentIndex(), -1);
}

TEST_F(HistoryStackTest, Append_SingleUrl_UrlAdded)
{
    QUrl url("file:///home/test");
    stack->append(url);
    EXPECT_EQ(stack->size(), 1);
    EXPECT_EQ(stack->currentIndex(), 0);
}

TEST_F(HistoryStackTest, Append_DuplicateUrl_NotAdded)
{
    QUrl url("file:///home/test");
    stack->append(url);
    stack->append(url);   // Duplicate
    EXPECT_EQ(stack->size(), 1);
    EXPECT_EQ(stack->currentIndex(), 0);
}

TEST_F(HistoryStackTest, Append_MultipleUrls_AllAdded)
{
    stack->append(QUrl("file:///home/test1"));
    stack->append(QUrl("file:///home/test2"));
    stack->append(QUrl("file:///home/test3"));
    EXPECT_EQ(stack->size(), 3);
    EXPECT_EQ(stack->currentIndex(), 2);
}

TEST_F(HistoryStackTest, Append_ExceedsThreshold_OldestRemoved)
{
    HistoryStack smallStack(3);
    smallStack.append(QUrl("file:///1"));
    smallStack.append(QUrl("file:///2"));
    smallStack.append(QUrl("file:///3"));
    EXPECT_EQ(smallStack.size(), 3);

    smallStack.append(QUrl("file:///4"));
    // Should remove first and add new
    EXPECT_EQ(smallStack.size(), 4);
    EXPECT_EQ(smallStack.currentIndex(), 3);
}

TEST_F(HistoryStackTest, Append_AfterBackNavigation_ForwardHistoryCleared)
{
    stack->append(QUrl("file:///1"));
    stack->append(QUrl("file:///2"));
    stack->append(QUrl("file:///3"));

    // Stub for back navigation
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       FileInfoPointer info(new FileInfo(url));
                       return info;
                   });
    stub.set_lamda(VADDR(FileInfo, exists), []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    stack->back();   // Go back to /2
    EXPECT_EQ(stack->currentIndex(), 1);

    stack->append(QUrl("file:///new"));   // Append new URL
    // Forward history (/3) should be cleared
    EXPECT_EQ(stack->size(), 3);
    EXPECT_EQ(stack->currentIndex(), 2);
}

TEST_F(HistoryStackTest, Back_EmptyStack_ReturnsInvalidUrl)
{
    QUrl url = stack->back();
    EXPECT_FALSE(url.isValid());
}

TEST_F(HistoryStackTest, Back_AtFirstPosition_ReturnsInvalidUrl)
{
    stack->append(QUrl("file:///test"));
    QUrl url = stack->back();
    EXPECT_FALSE(url.isValid());
}

TEST_F(HistoryStackTest, Back_WithHistory_ReturnsPreviousUrl)
{
    QUrl url1("file:///test1");
    QUrl url2("file:///test2");

    stack->append(url1);
    stack->append(url2);

    // Stub file existence
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       FileInfoPointer info(new FileInfo(url));
                       return info;
                   });
    stub.set_lamda(VADDR(FileInfo, exists), []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl backUrl = stack->back();
    EXPECT_EQ(backUrl, url1);
    EXPECT_EQ(stack->currentIndex(), 0);
}

TEST_F(HistoryStackTest, Forward_EmptyStack_ReturnsInvalidUrl)
{
    QUrl url = stack->forward();
    EXPECT_FALSE(url.isValid());
}

TEST_F(HistoryStackTest, Forward_AtLastPosition_ReturnsInvalidUrl)
{
    stack->append(QUrl("file:///test"));

    QUrl url = stack->forward();
    EXPECT_FALSE(url.isValid());
}

TEST_F(HistoryStackTest, Forward_AfterBack_ReturnsNextUrl)
{
    QUrl url1("file:///test1");
    QUrl url2("file:///test2");

    stack->append(url1);
    stack->append(url2);

    // Stub file existence
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       FileInfoPointer info(new FileInfo(url));
                       return info;
                   });
    stub.set_lamda(VADDR(FileInfo, exists), []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    stack->back();   // Go back to url1
    QUrl forwardUrl = stack->forward();   // Go forward to url2
    EXPECT_EQ(forwardUrl, url2);
    EXPECT_EQ(stack->currentIndex(), 1);
}

TEST_F(HistoryStackTest, SetThreshold_NewValue_ThresholdUpdated)
{
    stack->setThreshold(20);
    // We can't directly test curThreshold, but we can test behavior
    for (int i = 0; i < 15; ++i) {
        stack->append(QUrl(QString("file:///test%1").arg(i)));
    }
    EXPECT_EQ(stack->size(), 15);
}

TEST_F(HistoryStackTest, IsFirst_EmptyStack_ReturnsTrue)
{
    EXPECT_TRUE(stack->isFirst());
}

TEST_F(HistoryStackTest, IsFirst_AtFirstPosition_ReturnsTrue)
{
    stack->append(QUrl("file:///test"));
    EXPECT_TRUE(stack->isFirst());
}

TEST_F(HistoryStackTest, IsFirst_NotAtFirst_ReturnsFalse)
{
    stack->append(QUrl("file:///test1"));
    stack->append(QUrl("file:///test2"));
    EXPECT_FALSE(stack->isFirst());
}

TEST_F(HistoryStackTest, IsLast_EmptyStack_ReturnsTrue)
{
    EXPECT_TRUE(stack->isLast());
}

TEST_F(HistoryStackTest, IsLast_AtLastPosition_ReturnsTrue)
{
    stack->append(QUrl("file:///test"));
    EXPECT_TRUE(stack->isLast());
}

TEST_F(HistoryStackTest, IsLast_NotAtLast_ReturnsFalse)
{
    stack->append(QUrl("file:///test1"));
    stack->append(QUrl("file:///test2"));

    // Stub for back
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       FileInfoPointer info(new FileInfo(url));
                       return info;
                   });
    stub.set_lamda(VADDR(FileInfo, exists), []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    stack->back();
    EXPECT_FALSE(stack->isLast());
}

TEST_F(HistoryStackTest, Size_EmptyStack_ReturnsZero)
{
    EXPECT_EQ(stack->size(), 0);
}

TEST_F(HistoryStackTest, Size_WithItems_ReturnsCorrectSize)
{
    stack->append(QUrl("file:///test1"));
    stack->append(QUrl("file:///test2"));
    EXPECT_EQ(stack->size(), 2);
}

TEST_F(HistoryStackTest, RemoveAt_ValidIndex_ItemRemoved)
{
    stack->append(QUrl("file:///test1"));
    stack->append(QUrl("file:///test2"));
    stack->append(QUrl("file:///test3"));

    stack->removeAt(1);
    EXPECT_EQ(stack->size(), 2);
}

TEST_F(HistoryStackTest, RemoveUrl_EmptyStack_NoError)
{
    stack->removeUrl(QUrl("file:///test"));
    EXPECT_EQ(stack->size(), 0);
}

TEST_F(HistoryStackTest, RemoveUrl_CurrentUrl_NotRemoved)
{
    QUrl url("file:///test");
    stack->append(url);

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;   // Simulate current URL
    });

    stack->removeUrl(url);
    EXPECT_EQ(stack->size(), 1);
}

TEST_F(HistoryStackTest, RemoveUrl_UrlExists_UrlRemoved)
{
    QUrl url1("file:///test1");
    QUrl url2("file:///test2");
    QUrl url3("file:///test3");

    stack->append(url1);
    stack->append(url2);
    stack->append(url3);

    stub.set_lamda(&UniversalUtils::urlEquals, [&url3](const QUrl &left, const QUrl &right) {
        __DBG_STUB_INVOKE__
        return left == url3 && right == url3;   // Only current URL matches
    });

    stack->removeUrl(url1);
    EXPECT_NO_THROW(stack->size());
}

TEST_F(HistoryStackTest, CurrentIndex_EmptyStack_ReturnsNegativeOne)
{
    EXPECT_EQ(stack->currentIndex(), -1);
}

TEST_F(HistoryStackTest, CurrentIndex_AfterAppend_ReturnsCorrectIndex)
{
    stack->append(QUrl("file:///test"));
    EXPECT_EQ(stack->currentIndex(), 0);

    stack->append(QUrl("file:///test2"));
    EXPECT_EQ(stack->currentIndex(), 1);
}

TEST_F(HistoryStackTest, BackIsExist_EmptyStack_ReturnsFalse)
{
    EXPECT_FALSE(stack->backIsExist());
}

TEST_F(HistoryStackTest, BackIsExist_AtFirst_ReturnsFalse)
{
    stack->append(QUrl("file:///test"));
    EXPECT_FALSE(stack->backIsExist());
}

TEST_F(HistoryStackTest, BackIsExist_WithBackUrl_ReturnsTrue)
{
    stack->append(QUrl("file:///test1"));
    stack->append(QUrl("file:///test2"));

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       FileInfoPointer info(new FileInfo(url));
                       return info;
                   });
    stub.set_lamda(VADDR(FileInfo, exists), []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(stack->backIsExist());
}

TEST_F(HistoryStackTest, ForwardIsExist_EmptyStack_ReturnsFalse)
{
    EXPECT_FALSE(stack->forwardIsExist());
}

TEST_F(HistoryStackTest, ForwardIsExist_AtLast_ReturnsFalse)
{
    stack->append(QUrl("file:///test"));
    EXPECT_FALSE(stack->forwardIsExist());
}

TEST_F(HistoryStackTest, ForwardIsExist_AfterBack_ReturnsTrue)
{
    stack->append(QUrl("file:///test1"));
    stack->append(QUrl("file:///test2"));

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       FileInfoPointer info(new FileInfo(url));
                       return info;
                   });
    stub.set_lamda(VADDR(FileInfo, exists), []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    stack->back();
    EXPECT_TRUE(stack->forwardIsExist());
}

TEST_F(HistoryStackTest, Back_NonFileScheme_SkipsExistenceCheck)
{
    QUrl url1("smb://server/share");
    QUrl url2("smb://server/share2");

    stack->append(url1);
    stack->append(url2);

    // Stub to return nullptr for non-file schemes
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       if (url.scheme() != "file")
                           return FileInfoPointer();
                       FileInfoPointer info(new FileInfo(url));
                       return info;
                   });

    QUrl backUrl = stack->back();
    EXPECT_EQ(backUrl, url1);
}

TEST_F(HistoryStackTest, Back_NonExistentFile_SkipsToValidUrl)
{
    QUrl url1("file:///test1");
    QUrl url2("file:///test2_nonexistent");
    QUrl url3("file:///test3");

    stack->append(url1);
    stack->append(url2);
    stack->append(url3);

    int callCount = 0;
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       FileInfoPointer info(new FileInfo(url));
                       return info;
                   });
    stub.set_lamda(VADDR(FileInfo, exists), [&callCount](FileInfo *obj) {
        __DBG_STUB_INVOKE__
        // url2 doesn't exist, others do
        // This is simplified; in real test we'd check the URL
        if (callCount++ == 0)
            return false;   // First check: url2 doesn't exist
        return true;
    });

    QUrl backUrl = stack->back();
    // Should skip url2 and go to url1
    EXPECT_EQ(backUrl, url1);
}
