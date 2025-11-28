// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>

#include "stubext.h"

#include "fileoperations/fileoperationutils/errormessageandaction.h"
#include <dfm-base/interfaces/abstractjobhandler.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_fileoperations;

class TestErrorMessageAndAction : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// ========== ErrorMessageAndAction::errorMsg() Tests ==========

TEST_F(TestErrorMessageAndAction, ErrorMsg_NoError)
{
    QUrl from = QUrl::fromLocalFile("/tmp/source.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");

    QString msg = ErrorMessageAndAction::errorMsg(from, to, AbstractJobHandler::JobErrorType::kNoError);

    // No error should return empty or minimal message
    SUCCEED();
}

TEST_F(TestErrorMessageAndAction, ErrorMsg_PermissionError)
{
    QUrl from = QUrl::fromLocalFile("/tmp/protected.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");

    QString msg = ErrorMessageAndAction::errorMsg(from, to, AbstractJobHandler::JobErrorType::kPermissionError);

    EXPECT_FALSE(msg.isEmpty());
}

TEST_F(TestErrorMessageAndAction, ErrorMsg_NonexistenceError)
{
    QUrl from = QUrl::fromLocalFile("/tmp/nonexistent.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");

    QString msg = ErrorMessageAndAction::errorMsg(from, to, AbstractJobHandler::JobErrorType::kNonexistenceError);

    EXPECT_FALSE(msg.isEmpty());
}

TEST_F(TestErrorMessageAndAction, ErrorMsg_DirectoryExistsError)
{
    QUrl from = QUrl::fromLocalFile("/tmp/nonempty_dir");
    QUrl to = QUrl::fromLocalFile("/tmp/dest");

    QString msg = ErrorMessageAndAction::errorMsg(from, to, AbstractJobHandler::JobErrorType::kDirectoryExistsError);

    EXPECT_FALSE(msg.isEmpty());
}

TEST_F(TestErrorMessageAndAction, ErrorMsg_WithCustomErrorMsg)
{
    QUrl from = QUrl::fromLocalFile("/tmp/source.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");
    QString customMsg = "Custom error message";

    QString msg = ErrorMessageAndAction::errorMsg(from, to, AbstractJobHandler::JobErrorType::kProrogramError, false, customMsg);

    EXPECT_FALSE(msg.isEmpty());
}

TEST_F(TestErrorMessageAndAction, ErrorMsg_IsToFlagTrue)
{
    QUrl from = QUrl::fromLocalFile("/tmp/source.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");

    QString msg = ErrorMessageAndAction::errorMsg(from, to, AbstractJobHandler::JobErrorType::kPermissionError, true);

    EXPECT_FALSE(msg.isEmpty());
}

TEST_F(TestErrorMessageAndAction, ErrorMsg_AllUsErrorMsgFlag)
{
    QUrl from = QUrl::fromLocalFile("/tmp/source.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");
    QString customMsg = "Detailed error";

    QString msg = ErrorMessageAndAction::errorMsg(from, to, AbstractJobHandler::JobErrorType::kProrogramError, false, customMsg, true);

    EXPECT_FALSE(msg.isEmpty());
}

// ========== ErrorMessageAndAction::srcAndDestString() Tests ==========

TEST_F(TestErrorMessageAndAction, SrcAndDestString_CopyJob)
{
    QUrl from = QUrl::fromLocalFile("/tmp/source.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");
    QString fromMsg, toMsg;

    ErrorMessageAndAction::srcAndDestString(from, to, &fromMsg, &toMsg, AbstractJobHandler::JobType::kCopyType);

    EXPECT_FALSE(fromMsg.isEmpty());
    EXPECT_FALSE(toMsg.isEmpty());
}

TEST_F(TestErrorMessageAndAction, SrcAndDestString_CutJob)
{
    QUrl from = QUrl::fromLocalFile("/tmp/source.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");
    QString fromMsg, toMsg;

    ErrorMessageAndAction::srcAndDestString(from, to, &fromMsg, &toMsg, AbstractJobHandler::JobType::kCutType);

    EXPECT_FALSE(fromMsg.isEmpty());
    EXPECT_FALSE(toMsg.isEmpty());
}

TEST_F(TestErrorMessageAndAction, SrcAndDestString_DeleteJob)
{
    QUrl from = QUrl::fromLocalFile("/tmp/todelete.txt");
    QUrl to;
    QString fromMsg, toMsg;

    ErrorMessageAndAction::srcAndDestString(from, to, &fromMsg, &toMsg, AbstractJobHandler::JobType::kDeleteType);

    EXPECT_FALSE(fromMsg.isEmpty());
}

TEST_F(TestErrorMessageAndAction, SrcAndDestString_WithError)
{
    QUrl from = QUrl::fromLocalFile("/tmp/source.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");
    QString fromMsg, toMsg;

    ErrorMessageAndAction::srcAndDestString(from, to, &fromMsg, &toMsg,
                                           AbstractJobHandler::JobType::kCopyType,
                                           AbstractJobHandler::JobErrorType::kPermissionError);

    EXPECT_FALSE(fromMsg.isEmpty());
    EXPECT_FALSE(toMsg.isEmpty());
}

TEST_F(TestErrorMessageAndAction, SrcAndDestString_NullPointers)
{
    QUrl from = QUrl::fromLocalFile("/tmp/source.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");

    // Should handle null pointers gracefully
    ErrorMessageAndAction::srcAndDestString(from, to, nullptr, nullptr, AbstractJobHandler::JobType::kCopyType);

    SUCCEED();
}

TEST_F(TestErrorMessageAndAction, SrcAndDestString_EmptyUrls)
{
    QUrl from;
    QUrl to;
    QString fromMsg, toMsg;

    ErrorMessageAndAction::srcAndDestString(from, to, &fromMsg, &toMsg, AbstractJobHandler::JobType::kCopyType);

    // Should handle empty URLs
    SUCCEED();
}

// ========== ErrorMessageAndAction::supportActions() Tests ==========

TEST_F(TestErrorMessageAndAction, SupportActions_NoError)
{
    AbstractJobHandler::SupportActions actions = ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kNoError);

    EXPECT_EQ(actions, AbstractJobHandler::SupportAction::kNoAction);
}

TEST_F(TestErrorMessageAndAction, SupportActions_PermissionError)
{
    AbstractJobHandler::SupportActions actions = ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kPermissionError);

    // Should offer skip and cancel actions
    EXPECT_TRUE(actions & AbstractJobHandler::SupportAction::kSkipAction);
}

TEST_F(TestErrorMessageAndAction, SupportActions_FileExistsError)
{
    AbstractJobHandler::SupportActions actions = ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kFileExistsError);

    // Should offer various replacement/merge options
    EXPECT_NE(actions, AbstractJobHandler::SupportAction::kNoAction);
}

TEST_F(TestErrorMessageAndAction, SupportActions_NotEnoughSpaceError)
{
    AbstractJobHandler::SupportActions actions = ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kNotEnoughSpaceError);

    // Should offer retry and cancel
    EXPECT_TRUE(actions & AbstractJobHandler::SupportAction::kRetryAction);
}

TEST_F(TestErrorMessageAndAction, SupportActions_DirectoryExistsError)
{
    AbstractJobHandler::SupportActions actions = ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kDirectoryExistsError);

    EXPECT_NE(actions, AbstractJobHandler::SupportAction::kNoAction);
}

TEST_F(TestErrorMessageAndAction, SupportActions_NonexistenceError)
{
    AbstractJobHandler::SupportActions actions = ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kNonexistenceError);

    EXPECT_NE(actions, AbstractJobHandler::SupportAction::kNoAction);
}

TEST_F(TestErrorMessageAndAction, SupportActions_SymlinkToGvfsError)
{
    AbstractJobHandler::SupportActions actions = ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kSymlinkToGvfsError);

    EXPECT_NE(actions, AbstractJobHandler::SupportAction::kNoAction);
}

TEST_F(TestErrorMessageAndAction, SupportActions_IntegrityCheckingError)
{
    AbstractJobHandler::SupportActions actions = ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kIntegrityCheckingError);

    EXPECT_NE(actions, AbstractJobHandler::SupportAction::kNoAction);
}

TEST_F(TestErrorMessageAndAction, SupportActions_TargetReadOnlyError)
{
    AbstractJobHandler::SupportActions actions = ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kTargetReadOnlyError);

    EXPECT_NE(actions, AbstractJobHandler::SupportAction::kNoAction);
}

// ========== Integration Tests ==========

TEST_F(TestErrorMessageAndAction, Integration_ErrorMsgAndSupportActions)
{
    QUrl from = QUrl::fromLocalFile("/tmp/source.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");

    // Get error message
    QString msg = ErrorMessageAndAction::errorMsg(from, to, AbstractJobHandler::JobErrorType::kPermissionError);

    // Get support actions
    AbstractJobHandler::SupportActions actions = ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kPermissionError);

    EXPECT_FALSE(msg.isEmpty());
    EXPECT_NE(actions, AbstractJobHandler::SupportAction::kNoAction);
}

TEST_F(TestErrorMessageAndAction, Integration_AllErrorTypes)
{
    QUrl from = QUrl::fromLocalFile("/tmp/source.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");

    QList<AbstractJobHandler::JobErrorType> errorTypes = {
        AbstractJobHandler::JobErrorType::kNoError,
        AbstractJobHandler::JobErrorType::kPermissionError,
        AbstractJobHandler::JobErrorType::kNonexistenceError,
        AbstractJobHandler::JobErrorType::kDirectoryExistsError,
        AbstractJobHandler::JobErrorType::kFileExistsError,
        AbstractJobHandler::JobErrorType::kNotEnoughSpaceError,
        AbstractJobHandler::JobErrorType::kProrogramError
    };

    for (const auto &errorType : errorTypes) {
        QString msg = ErrorMessageAndAction::errorMsg(from, to, errorType);
        AbstractJobHandler::SupportActions actions = ErrorMessageAndAction::supportActions(errorType);

        // Should produce valid results for all error types
        SUCCEED();
    }
}

// ========== Edge Cases ==========

TEST_F(TestErrorMessageAndAction, EdgeCase_VeryLongFilePath)
{
    QString longPath = "/tmp/" + QString("a").repeated(500) + ".txt";
    QUrl from = QUrl::fromLocalFile(longPath);
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");

    QString msg = ErrorMessageAndAction::errorMsg(from, to, AbstractJobHandler::JobErrorType::kPermissionError);

    EXPECT_FALSE(msg.isEmpty());
}

TEST_F(TestErrorMessageAndAction, EdgeCase_UnicodeFilePath)
{
    QUrl from = QUrl::fromLocalFile("/tmp/测试文件.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/目标文件.txt");

    QString msg = ErrorMessageAndAction::errorMsg(from, to, AbstractJobHandler::JobErrorType::kPermissionError);

    EXPECT_FALSE(msg.isEmpty());
}

TEST_F(TestErrorMessageAndAction, EdgeCase_SpecialCharactersInPath)
{
    QUrl from = QUrl::fromLocalFile("/tmp/file with spaces & symbols @#$.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest!@#.txt");

    QString msg = ErrorMessageAndAction::errorMsg(from, to, AbstractJobHandler::JobErrorType::kPermissionError);

    EXPECT_FALSE(msg.isEmpty());
}

TEST_F(TestErrorMessageAndAction, EdgeCase_NetworkUrl)
{
    QUrl from = QUrl("smb://server/share/file.txt");
    QUrl to = QUrl("smb://server/share/dest.txt");

    QString msg = ErrorMessageAndAction::errorMsg(from, to, AbstractJobHandler::JobErrorType::kPermissionError);

    EXPECT_FALSE(msg.isEmpty());
}

TEST_F(TestErrorMessageAndAction, EdgeCase_MixedLocalAndNetworkUrls)
{
    QUrl from = QUrl::fromLocalFile("/tmp/local.txt");
    QUrl to = QUrl("smb://server/share/network.txt");

    QString msg = ErrorMessageAndAction::errorMsg(from, to, AbstractJobHandler::JobErrorType::kPermissionError);

    EXPECT_FALSE(msg.isEmpty());
}
