// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/fileoperationutils/errormessageandaction.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <gtest/gtest.h>


DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
class UT_ErrorMessageAndAction : public testing::Test
{
public:
    void SetUp() override {
        // 注册路由
        UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/", QIcon(), false, QObject::tr("System Disk"));
        // 注册Scheme为"file"的扩展的文件信息 本地默认文件的
        InfoFactory::regClass<dfmbase::SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
    }
    void TearDown() override {}
};


TEST_F(UT_ErrorMessageAndAction, testErrorMsg)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&ErrorMessageAndAction::errorToString, []{ __DBG_STUB_INVOKE__ return QString();});
    stub.set_lamda(&ErrorMessageAndAction::errorToStringByCause, []{ __DBG_STUB_INVOKE__ return QString();});
    EXPECT_TRUE(ErrorMessageAndAction::errorMsg(QUrl(), QUrl(), AbstractJobHandler::JobErrorType::kNoError).isEmpty());

    EXPECT_TRUE(ErrorMessageAndAction::errorMsg(QUrl(), QUrl(), AbstractJobHandler::JobErrorType::kNoError, false, QString("testee")).isEmpty());
    EXPECT_TRUE(!ErrorMessageAndAction::errorMsg(QUrl(), QUrl(), AbstractJobHandler::JobErrorType::kNoError, false, QString("testee"), true).isEmpty());
}

TEST_F(UT_ErrorMessageAndAction, testSupportActions)
{
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kNoError).testFlag(AbstractJobHandler::SupportAction::kCancelAction));

    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kPermissionError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kOpenError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kReadError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kWriteError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kSymlinkError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kMkdirError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kResizeError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kRemoveError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kRenameError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kIntegrityCheckingError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kNonexistenceError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kDeleteFileError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kDeleteTrashFileError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kNoSourceError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kPermissionDeniedError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kCancelError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kUnknowUrlError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kNotSupportedError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kSeekError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kProrogramError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kDfmIoError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kMakeStandardTrashError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kGetRestorePathError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kIsNotTrashFileError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kCreateParentDirError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kFailedParseUrlOfTrash).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kUnknowError).testFlag(AbstractJobHandler::SupportAction::kRetryAction));

    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kSpecialFileError).testFlag(AbstractJobHandler::SupportAction::kSkipAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kFileSizeTooBigError).testFlag(AbstractJobHandler::SupportAction::kEnforceAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kNotEnoughSpaceError).testFlag(AbstractJobHandler::SupportAction::kEnforceAction));

    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kFileExistsError).testFlag(AbstractJobHandler::SupportAction::kCoexistAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kDirectoryExistsError).testFlag(AbstractJobHandler::SupportAction::kMergeAction));

    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kTargetReadOnlyError).testFlag(AbstractJobHandler::SupportAction::kEnforceAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kTargetIsSelfError).testFlag(AbstractJobHandler::SupportAction::kEnforceAction));
    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kSymlinkToGvfsError).testFlag(AbstractJobHandler::SupportAction::kSkipAction));

    EXPECT_TRUE(ErrorMessageAndAction::supportActions(AbstractJobHandler::JobErrorType::kFailedObtainTrashOriginalFile).testFlag(AbstractJobHandler::SupportAction::kRetryAction));
}

TEST_F(UT_ErrorMessageAndAction, testErrorSrcAndDestString)
{
    QString sorceMsg, toMsg;
    EXPECT_NO_FATAL_FAILURE(ErrorMessageAndAction::errorSrcAndDestString(QUrl(), QUrl(), &sorceMsg, nullptr, AbstractJobHandler::JobErrorType::kOpenError));
    EXPECT_NO_FATAL_FAILURE(ErrorMessageAndAction::errorSrcAndDestString(QUrl(), QUrl(), &sorceMsg, &toMsg, AbstractJobHandler::JobErrorType::kDirectoryExistsError));
    EXPECT_NO_FATAL_FAILURE(ErrorMessageAndAction::errorSrcAndDestString(QUrl(), QUrl(), &sorceMsg, &toMsg, AbstractJobHandler::JobErrorType::kDirectoryExistsError));

    EXPECT_NO_FATAL_FAILURE(ErrorMessageAndAction::errorSrcAndDestString(QUrl(), QUrl(), &sorceMsg, &toMsg, AbstractJobHandler::JobErrorType::kOpenError));
    EXPECT_FALSE(sorceMsg.isEmpty());
    EXPECT_FALSE(toMsg.isEmpty());
    QString srcmsg;
    ErrorMessageAndAction::srcAndDestString(QUrl(), QUrl(), &srcmsg, nullptr, AbstractJobHandler::JobType::kCopyType, AbstractJobHandler::JobErrorType::kDirectoryExistsError);

    EXPECT_NO_FATAL_FAILURE(ErrorMessageAndAction::errorSrcAndDestString(QUrl("file:///home/kjfsdakjdfjkashdjkfhasjkdfhajksdfjaksdfhjksdhnfkjsdfhjksadjfhaksjdfhkjs"),
                                                                         QUrl(), &sorceMsg, &toMsg, AbstractJobHandler::JobErrorType::kDirectoryExistsError));
    EXPECT_NO_FATAL_FAILURE(ErrorMessageAndAction::errorSrcAndDestString(QUrl("file:///home/kjfsdakjdfjkashdjkfhasjkdfhajksdfjaksdfhjksdhnfkjsdfhjksadjfhaksjdfhkjs"),
                                                                         QUrl("file:///home/kjfsdakjdfjkashdjkfhasjkdfhajksdfjaksdfhjksdhnfkjsdfhjksadjfhaksjdfhkjs/tees"),
                                                                         &sorceMsg, &toMsg, AbstractJobHandler::JobErrorType::kDirectoryExistsError));

    EXPECT_FALSE(sorceMsg.isEmpty());
    EXPECT_FALSE(toMsg.isEmpty());
}

TEST_F(UT_ErrorMessageAndAction, testErrorToString)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kPermissionError) == QObject::tr("Permission error"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kSpecialFileError) == QObject::tr("The action is denied"));
    EXPECT_TRUE(!ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kFileExistsError).isEmpty());
    EXPECT_TRUE(!ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kDirectoryExistsError).isEmpty());
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kOpenError).endsWith(url.path()));
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kReadError).endsWith(url.path()));
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kWriteError).endsWith(url.path()));
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kMkdirError).endsWith(url.path()));
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kCreateParentDirError).endsWith(url.path()));
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kRemoveError).endsWith(url.path()));
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kDeleteFileError).endsWith(url.path()));
    EXPECT_TRUE(!ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kFileMoveToTrashError).isEmpty());
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kRenameError).endsWith(url.path()));
    EXPECT_TRUE(!ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kNonexistenceError).isEmpty());
    EXPECT_TRUE(!ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kFileSizeTooBigError).isEmpty());
    EXPECT_TRUE(!ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kIntegrityCheckingError).isEmpty());
    EXPECT_TRUE(!ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kPermissionDeniedError).isEmpty());
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kNotEnoughSpaceError) == QObject::tr("Not enough free space on the target disk"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kTargetReadOnlyError) == QObject::tr("The target device is read only"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kTargetIsSelfError) == QObject::tr("Target folder is inside the source folder"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kNotSupportedError) == QObject::tr("The action is not supported"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kGetRestorePathError) == QObject::tr("Restore failed, original path could not be found"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kProrogramError) == QObject::tr("Unknown error"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kFailedParseUrlOfTrash) == QObject::tr("Failed to parse the url of trash"));
    EXPECT_FALSE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kFailedObtainTrashOriginalFile) == QObject::tr("Failed to obtain the trash original file"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kUnknowError).isEmpty());
    EXPECT_TRUE(ErrorMessageAndAction::errorToString(url, AbstractJobHandler::JobErrorType::kDfmIoError) == QObject::tr("Copy or Cut File failed!"));
}

TEST_F(UT_ErrorMessageAndAction, testErrorToStringByCause)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kPermissionError, "testerror") == QObject::tr("Permission error"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kSpecialFileError, "testerror") == QObject::tr("The action is denied"));
    EXPECT_TRUE(!ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kSpecialFileError, "testerror").isEmpty());
    EXPECT_TRUE(!ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kFileExistsError, "testerror").isEmpty());
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kOpenError, "testerror").endsWith("testerror"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kReadError, "testerror").endsWith("testerror"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kWriteError, "testerror").endsWith("testerror"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kMkdirError, "testerror").endsWith("testerror"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kCreateParentDirError, "testerror").endsWith("testerror"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kRemoveError, "testerror").endsWith("testerror"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kDeleteFileError, "testerror").endsWith("testerror"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kFileMoveToTrashError, "testerror").endsWith("testerror"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kRenameError, "testerror").endsWith("testerror"));
    EXPECT_TRUE(!ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kNonexistenceError, "testerror").isEmpty());
    EXPECT_TRUE(!ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kFileSizeTooBigError, "testerror").isEmpty());
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kIntegrityCheckingError, "testerror").endsWith("testerror"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kPermissionDeniedError, "testerror").endsWith(url.path()));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kNotEnoughSpaceError, "testerror") == QObject::tr("Not enough free space on the target disk"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kTargetReadOnlyError, "testerror") == QObject::tr("The target device is read only"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kTargetIsSelfError, "testerror") == QObject::tr("Target folder is inside the source folder"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kNotSupportedError, "testerror") == QObject::tr("The action is not supported"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kSymlinkError, "testerror").endsWith("testerror"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kUnknowError, "testerror").isEmpty());
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kDfmIoError, "testerror").endsWith("testerror"));
    EXPECT_TRUE(ErrorMessageAndAction::errorToStringByCause(url, AbstractJobHandler::JobErrorType::kDirectoryExistsError, "testerror").endsWith("already exists"));
}
