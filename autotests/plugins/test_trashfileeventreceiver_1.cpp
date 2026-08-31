// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashfileeventreceiver_1.cpp
 * @brief Unit tests for TrashFileEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperationsevent/trashfileeventreceiver.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class TrashFileEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashFileEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashFileEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashFileEventReceiverTest, doCopyFromTrash)
{
    // Test method: JobHandlePointer doCopyFromTrash((const quint64 windowId, const QList<QUrl> &sources, const QUrl &target,
                                                         const AbstractJobHandler::JobFlags flags,
                                                         AbstractJobHandler::OperatorHandleCallback handleCallback))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->doCopyFromTrash(0, _arg1, _arg2, AbstractJobHandler::JobFlags(), AbstractJobHandler::OperatorHandleCallback());
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(TrashFileEventReceiverTest, doRestoreFromTrash)
{
    // Test method: JobHandlePointer doRestoreFromTrash((const quint64 windowId, const QList<QUrl> &sources, const QUrl &target,
                                                            const AbstractJobHandler::JobFlags flags, AbstractJobHandler::OperatorHandleCallback handleCallback, const bool isInit))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->doRestoreFromTrash(0, _arg1, _arg2, AbstractJobHandler::JobFlags(), AbstractJobHandler::OperatorHandleCallback(), false);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(TrashFileEventReceiverTest, handleOperationCleanTrash)
{
    // Test method: void handleOperationCleanTrash((const quint64 windowId, const QList<QUrl> sources,
                                                       DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                       const QVariant custom, AbstractJobHandler::OperatorCallback callback))
    EXPECT_NO_FATAL_FAILURE(obj->handleOperationCleanTrash(0, QList<QUrl>(), DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback(), QVariant(), AbstractJobHandler::OperatorCallback()));
}

TEST_F(TrashFileEventReceiverTest, handleOperationCopyFromTrash)
{
    // Test method: void handleOperationCopyFromTrash((const quint64 windowId,
                                                          const QList<QUrl> &sources, const QUrl &target,
                                                          const AbstractJobHandler::JobFlag flags,
                                                          AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                          const QVariant custom,
                                                          AbstractJobHandler::OperatorCallback callback))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleOperationCopyFromTrash(0, _arg1, _arg2, AbstractJobHandler::JobFlag(), AbstractJobHandler::OperatorHandleCallback(), QVariant(), AbstractJobHandler::OperatorCallback()));
}

TEST_F(TrashFileEventReceiverTest, handleOperationMoveToTrash)
{
    // Test method: void handleOperationMoveToTrash((const quint64 windowId,
                                                        const QList<QUrl> sources,
                                                        const AbstractJobHandler::JobFlag flags,
                                                        DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                        const QVariant custom,
                                                        DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback))
    EXPECT_NO_FATAL_FAILURE(obj->handleOperationMoveToTrash(0, QList<QUrl>(), AbstractJobHandler::JobFlag(), DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback(), QVariant(), DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback()));
}

TEST_F(TrashFileEventReceiverTest, handleOperationRestoreFromTrash)
{
    // Test method: void handleOperationRestoreFromTrash((const quint64 windowId,
                                                             const QList<QUrl> sources, const QUrl target,
                                                             const AbstractJobHandler::JobFlag flags,
                                                             DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                             const QVariant custom,
                                                             DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback))
    EXPECT_NO_FATAL_FAILURE(obj->handleOperationRestoreFromTrash(0, QList<QUrl>(), QUrl(), AbstractJobHandler::JobFlag(), DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback(), QVariant(), DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback()));
}

TEST_F(TrashFileEventReceiverTest, handleOperationUndoMoveToTrash)
{
    // Test method: void handleOperationUndoMoveToTrash((const quint64 windowId,
                                                            const QList<QUrl> &sources,
                                                            const AbstractJobHandler::JobFlag flags,
                                                            AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                            const QVariantMap &op))
    QList<QUrl> _arg1{};
    QVariantMap _arg4{};
    EXPECT_NO_FATAL_FAILURE(obj->handleOperationUndoMoveToTrash(0, _arg1, AbstractJobHandler::JobFlag(), AbstractJobHandler::OperatorHandleCallback(), _arg4));
}

TEST_F(TrashFileEventReceiverTest, handleOperationUndoRestoreFromTrash)
{
    // Test method: void handleOperationUndoRestoreFromTrash((const quint64 windowId, const QList<QUrl> &sources, const QUrl &target, const AbstractJobHandler::JobFlag flags, AbstractJobHandler::OperatorHandleCallback handleCallback, const QVariantMap &op))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    QVariantMap _arg5{};
    EXPECT_NO_FATAL_FAILURE(obj->handleOperationUndoRestoreFromTrash(0, _arg1, _arg2, AbstractJobHandler::JobFlag(), AbstractJobHandler::OperatorHandleCallback(), _arg5));
}

TEST_F(TrashFileEventReceiverTest, onCleanTrashUrls)
{
    // Test method: JobHandlePointer onCleanTrashUrls((const quint64 windowId, const QList<QUrl> &sources,
                                                          const AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                                          AbstractJobHandler::OperatorHandleCallback handleCallback))
    QList<QUrl> _arg1{};
    auto result = obj->onCleanTrashUrls(0, _arg1, AbstractJobHandler::DeleteDialogNoticeType(), AbstractJobHandler::OperatorHandleCallback());
    EXPECT_NE(result.get(), nullptr);

}
