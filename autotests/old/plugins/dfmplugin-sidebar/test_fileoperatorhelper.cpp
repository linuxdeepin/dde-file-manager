// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "utils/fileoperatorhelper.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-framework/dpf.h>

#include <QUrl>

using namespace dfmplugin_sidebar;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_FileOperatorHelper : public testing::Test
{
protected:
    virtual void SetUp() override { }

    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_FileOperatorHelper, Instance)
{
    auto ins1 = FileOperatorHelper::instance();
    auto ins2 = FileOperatorHelper::instance();

    EXPECT_NE(ins1, nullptr);
    EXPECT_EQ(ins1, ins2);
}

TEST_F(UT_FileOperatorHelper, PasteFiles_MoveAction)
{
    bool isCallCut { false };
    quint64 capturedWindowId { 0 };
    QList<QUrl> capturedSrcUrls;
    QUrl capturedTargetUrl;
    AbstractJobHandler::JobFlag capturedFlag;

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, unsigned long long,
                                                        const QList<QUrl> &,
                                                        const QUrl &,
                                                        AbstractJobHandler::JobFlag &&,
                                                        std::nullptr_t &&);

    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, EventType type, unsigned long long windowId,
                       const QList<QUrl> &srcUrls, const QUrl &targetUrl,
                       AbstractJobHandler::JobFlag flag, std::nullptr_t) {
                       __DBG_STUB_INVOKE__
                       if (type == GlobalEventType::kCutFile) {
                           isCallCut = true;
                           capturedWindowId = windowId;
                           capturedSrcUrls = srcUrls;
                           capturedTargetUrl = targetUrl;
                           capturedFlag = flag;
                       }
                       return true;
                   });

    quint64 windowId = 12345;
    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/tmp/test1.txt"),
                            QUrl::fromLocalFile("/tmp/test2.txt") };
    QUrl targetUrl = QUrl::fromLocalFile("/tmp/target");

    FileOperatorHelper::instance()->pasteFiles(windowId, srcUrls, targetUrl, Qt::MoveAction);

    EXPECT_TRUE(isCallCut);
    EXPECT_EQ(capturedWindowId, windowId);
    EXPECT_EQ(capturedSrcUrls, srcUrls);
    EXPECT_EQ(capturedTargetUrl, targetUrl);
    EXPECT_EQ(capturedFlag, AbstractJobHandler::JobFlag::kNoHint);
}

TEST_F(UT_FileOperatorHelper, PasteFiles_CopyAction)
{
    bool isCallCopy { false };
    quint64 capturedWindowId { 0 };
    QList<QUrl> capturedSrcUrls;
    QUrl capturedTargetUrl;
    AbstractJobHandler::JobFlag capturedFlag;

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64,
                                                        const QList<QUrl> &,
                                                        const QUrl &,
                                                        AbstractJobHandler::JobFlag &&,
                                                        std::nullptr_t &&);

    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, EventType type, quint64 windowId,
                       const QList<QUrl> &srcUrls, const QUrl &targetUrl,
                       AbstractJobHandler::JobFlag flag, std::nullptr_t) -> bool {
                       __DBG_STUB_INVOKE__
                       if (type == GlobalEventType::kCopy) {
                           isCallCopy = true;
                           capturedWindowId = windowId;
                           capturedSrcUrls = srcUrls;
                           capturedTargetUrl = targetUrl;
                           capturedFlag = flag;
                       }
                       return true;
                   });

    quint64 windowId = 54321;
    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/tmp/copy1.txt") };
    QUrl targetUrl = QUrl::fromLocalFile("/tmp/copy_target");

    FileOperatorHelper::instance()->pasteFiles(windowId, srcUrls, targetUrl, Qt::CopyAction);

    EXPECT_TRUE(isCallCopy);
    EXPECT_EQ(capturedWindowId, windowId);
    EXPECT_EQ(capturedSrcUrls, srcUrls);
    EXPECT_EQ(capturedTargetUrl, targetUrl);
    EXPECT_EQ(capturedFlag, AbstractJobHandler::JobFlag::kNoHint);
}

TEST_F(UT_FileOperatorHelper, PasteFiles_DefaultActionIsCopy)
{
    bool isCallCopy { false };

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64,
                                                        const QList<QUrl> &,
                                                        const QUrl &,
                                                        AbstractJobHandler::JobFlag &&,
                                                        std::nullptr_t &&);

    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, EventType type, quint64,
                       const QList<QUrl> &, const QUrl &,
                       AbstractJobHandler::JobFlag, std::nullptr_t) -> bool {
                       __DBG_STUB_INVOKE__
                       if (type == GlobalEventType::kCopy) {
                           isCallCopy = true;
                       }
                       return true;
                   });

    // Use any action other than MoveAction, should default to copy
    FileOperatorHelper::instance()->pasteFiles(1, { QUrl() }, QUrl(), Qt::LinkAction);

    EXPECT_TRUE(isCallCopy);
}
