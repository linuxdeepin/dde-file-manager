// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractjobhandler_cov.cpp
 * @brief Coverage tests for include/dfm-base/interfaces/abstractjobhandler.h.
 *
 * The gap list names the moc-generated qt_getEnumName /
 * qt_getEnumMetaObject overloads for the Q_ENUMs of AbstractJobHandler plus
 * the constructor's requestShowTipsDialog lambda (abstractjobhandler.cpp L35).
 *
 * Covered gap-list functions -> case mapping:
 *   qt_getEnumName(JobFlag) + qt_getEnumMetaObject(JobFlag)
 *       -> MetaEnum_JobCoreEnums_ResolveNames
 *   qt_getEnumName(JobState), qt_getEnumName(StatisticState),
 *     qt_getEnumName(JobType)
 *       -> MetaEnum_JobCoreEnums_ResolveNames
 *   qt_getEnumName(JobErrorType), qt_getEnumName(SupportAction)
 *       -> MetaEnum_ErrorAndSupportEnums_ResolveNames
 *   qt_getEnumName(NotifyInfoKey) + qt_getEnumMetaObject(NotifyInfoKey),
 *     qt_getEnumName(NotifyType) + qt_getEnumMetaObject(NotifyType)
 *       -> MetaEnum_NotifyEnums_ResolveNames
 *   qt_getEnumName(FileNameAddFlag) + qt_getEnumMetaObject(FileNameAddFlag),
 *     qt_getEnumName(DeleteDialogNoticeType)
 *       -> MetaEnum_FileNameAndDialogEnums_ResolveNames
 *   AbstractJobHandler ctor lambda (L35)
 *       -> CtorLambda_ShowTipsDialog_InvokesDialogManager,
 *          CtorLambda_CopyMoveToSelf_InvokesDialogManager
 *
 * Branch notes for the ctor lambda (from source): switch has two cases
 * (kRestoreFailed -> showRestoreFailedDialog(urls.count()),
 *  kCopyMoveToSelf -> showCopyMoveToSelfDialog()); DialogManager calls are
 * stubbed so no real dialog is shown.
 *
 * QMetaEnum::fromType<E>() is the canonical caller of qt_getEnumName /
 * qt_getEnumMetaObject, so one fromType call per enum covers both overloads.
 */

#include <gtest/gtest.h>

#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/dialogmanager.h>

#include <QMetaEnum>
#include <QList>
#include <QUrl>

#include "stubext.h"

using namespace dfmbase;

namespace {
class UtCovJobHandler : public AbstractJobHandler
{
public:
    explicit UtCovJobHandler(QObject *parent = nullptr)
        : AbstractJobHandler(parent) { }
};

// Check the common invariants of a resolved Q_ENUM.
::testing::AssertionResult enumLooksRegistered(const QMetaEnum &meta,
                                               const char *enumName)
{
    if (QString(meta.name()) != QString(enumName))
        return ::testing::AssertionFailure() << "name mismatch: " << meta.name();
    if (QString(meta.scope()) != "dfmbase::AbstractJobHandler")
        return ::testing::AssertionFailure() << "scope mismatch: " << meta.scope();
    if (meta.keyCount() <= 0)
        return ::testing::AssertionFailure() << "no keys for " << enumName;
    return ::testing::AssertionSuccess();
}
}   // namespace

class UT_AbstractJobHandlerCov : public testing::Test
{
protected:
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
};

TEST_F(UT_AbstractJobHandlerCov, MetaEnum_JobCoreEnums_ResolveNames)
{
    // Arrange
    // Act
    const QMetaEnum jobFlag = QMetaEnum::fromType<AbstractJobHandler::JobFlag>();
    const QMetaEnum jobState = QMetaEnum::fromType<AbstractJobHandler::JobState>();
    const QMetaEnum statistic = QMetaEnum::fromType<AbstractJobHandler::StatisticState>();
    const QMetaEnum jobType = QMetaEnum::fromType<AbstractJobHandler::JobType>();

    // Assert — exact name/scope plus a key<->value roundtrip.
    EXPECT_TRUE(enumLooksRegistered(jobFlag, "JobFlag"));
    EXPECT_TRUE(enumLooksRegistered(jobState, "JobState"));
    EXPECT_TRUE(enumLooksRegistered(statistic, "StatisticState"));
    EXPECT_TRUE(enumLooksRegistered(jobType, "JobType"));
    const char *firstFlagKey = jobFlag.key(0);
    EXPECT_STREQ(jobFlag.valueToKey(jobFlag.keyToValue(firstFlagKey)), firstFlagKey);
    const char *firstStateKey = jobState.key(0);
    EXPECT_STREQ(jobState.valueToKey(jobState.keyToValue(firstStateKey)), firstStateKey);
}

TEST_F(UT_AbstractJobHandlerCov, MetaEnum_ErrorAndSupportEnums_ResolveNames)
{
    // Arrange
    // Act
    const QMetaEnum errorType = QMetaEnum::fromType<AbstractJobHandler::JobErrorType>();
    const QMetaEnum support = QMetaEnum::fromType<AbstractJobHandler::SupportAction>();

    // Assert
    EXPECT_TRUE(enumLooksRegistered(errorType, "JobErrorType"));
    EXPECT_TRUE(enumLooksRegistered(support, "SupportAction"));
    EXPECT_GT(errorType.keyCount(), 10);   // the error catalogue is large
    const char *supportKey = support.key(0);
    EXPECT_STREQ(support.valueToKey(support.keyToValue(supportKey)), supportKey);
}

TEST_F(UT_AbstractJobHandlerCov, MetaEnum_NotifyEnums_ResolveNames)
{
    // Arrange
    // Act
    const QMetaEnum infoKey = QMetaEnum::fromType<AbstractJobHandler::NotifyInfoKey>();
    const QMetaEnum notifyType = QMetaEnum::fromType<AbstractJobHandler::NotifyType>();

    // Assert
    EXPECT_TRUE(enumLooksRegistered(infoKey, "NotifyInfoKey"));
    EXPECT_TRUE(enumLooksRegistered(notifyType, "NotifyType"));
    const char *infoKeyName = infoKey.key(0);
    EXPECT_STREQ(infoKey.valueToKey(infoKey.keyToValue(infoKeyName)), infoKeyName);
    const char *notifyTypeName = notifyType.key(0);
    EXPECT_STREQ(notifyType.valueToKey(notifyType.keyToValue(notifyTypeName)), notifyTypeName);
}

TEST_F(UT_AbstractJobHandlerCov, MetaEnum_FileNameAndDialogEnums_ResolveNames)
{
    // Arrange
    // Act
    const QMetaEnum addFlag = QMetaEnum::fromType<AbstractJobHandler::FileNameAddFlag>();
    const QMetaEnum notice = QMetaEnum::fromType<AbstractJobHandler::DeleteDialogNoticeType>();

    // Assert
    EXPECT_TRUE(enumLooksRegistered(addFlag, "FileNameAddFlag"));
    EXPECT_TRUE(enumLooksRegistered(notice, "DeleteDialogNoticeType"));
    const char *addFlagKey = addFlag.key(0);
    EXPECT_STREQ(addFlag.valueToKey(addFlag.keyToValue(addFlagKey)), addFlagKey);
    const char *noticeKey = notice.key(0);
    EXPECT_STREQ(notice.valueToKey(notice.keyToValue(noticeKey)), noticeKey);
}

TEST_F(UT_AbstractJobHandlerCov, CtorLambda_ShowTipsDialog_InvokesDialogManager)
{
    // Arrange — stub the dialog backend, the lambda only forwards the count.
    int restoreCount = -1;
    stub.set_lamda(ADDR(dfmbase::DialogManager, showRestoreFailedDialog),
                   [&restoreCount](dfmbase::DialogManager *, int count) { restoreCount = count; });
    UtCovJobHandler handler;
    const QList<QUrl> urls { QUrl::fromLocalFile("/tmp/a"), QUrl::fromLocalFile("/tmp/b"),
                             QUrl::fromLocalFile("/tmp/c") };

    // Act — the constructor connected this lambda to requestShowTipsDialog.
    emit handler.requestShowTipsDialog(
            AbstractJobHandler::ShowDialogType::kRestoreFailed, urls);

    // Assert
    EXPECT_EQ(restoreCount, 3);
    emit handler.requestShowTipsDialog(
            AbstractJobHandler::ShowDialogType::kRestoreFailed, QList<QUrl> {});
    EXPECT_EQ(restoreCount, 0);
}

TEST_F(UT_AbstractJobHandlerCov, CtorLambda_CopyMoveToSelf_InvokesDialogManager)
{
    // Arrange
    int selfDialogCalls = 0;
    stub.set_lamda(ADDR(dfmbase::DialogManager, showCopyMoveToSelfDialog),
                   [&selfDialogCalls](dfmbase::DialogManager *) { ++selfDialogCalls; });
    UtCovJobHandler handler;

    // Act
    emit handler.requestShowTipsDialog(
            AbstractJobHandler::ShowDialogType::kCopyMoveToSelf, QList<QUrl> {});

    // Assert — the kCopyMoveToSelf branch ran exactly once.
    EXPECT_EQ(selfDialogCalls, 1);
    emit handler.requestShowTipsDialog(
            AbstractJobHandler::ShowDialogType::kCopyMoveToSelf, QList<QUrl> {});
    EXPECT_EQ(selfDialogCalls, 2);
}
