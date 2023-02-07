/*
 * Copyright (C) 2023 ~ 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang1@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "errormessageandaction.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"

#include <QUrl>
#include <QLabel>

namespace dfmplugin_fileoperations {
QString ErrorMessageAndAction::errorMsg(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error, const bool isTo, const QString &errorMsg, const bool allUsErrorMsg)
{
    if (errorMsg.isEmpty())
        return errorToString(isTo ? to : from, error);

    if (allUsErrorMsg)
        return tr(errorMsg.toStdString().c_str());

    return errorToStringByCause(isTo ? to : from, error, errorMsg);
}

void ErrorMessageAndAction::srcAndDestString(const QUrl &from, const QUrl &to, QString *sorceMsg, QString *toMsg,
                                             const AbstractJobHandler::JobType jobType, const AbstractJobHandler::JobErrorType error)
{
    if (!sorceMsg || !toMsg)
        return;
    if (AbstractJobHandler::JobType::kCopyType == jobType) {
        *sorceMsg = QString(tr("Copying %1")).arg(from.path());
        *toMsg = QString(tr("to %1")).arg(UrlRoute::urlParent(to).path());
        errorSrcAndDestString(from, to, sorceMsg, toMsg, error);
    } else if (AbstractJobHandler::JobType::kDeleteType == jobType) {
        *sorceMsg = QString(tr("Deleting %1")).arg(from.path());
    } else if (AbstractJobHandler::JobType::kCutType == jobType) {
        *sorceMsg = QString(tr("Moving %1")).arg(from.path());
        *toMsg = QString(tr("to %1")).arg(UrlRoute::urlParent(to).path());
        errorSrcAndDestString(from, to, sorceMsg, toMsg, error);
    } else if (AbstractJobHandler::JobType::kMoveToTrashType == jobType) {
        *sorceMsg = QString(tr("Trashing %1")).arg(from.path());
    } else if (AbstractJobHandler::JobType::kRestoreType == jobType) {
        *sorceMsg = QString(tr("Restoring %1")).arg(from.path());
        *toMsg = QString(tr("to %1")).arg(UrlRoute::urlParent(to).path());
    } else if (AbstractJobHandler::JobType::kCleanTrashType == jobType) {
        *sorceMsg = QString(tr("Deleting %1")).arg(from.path());
    }
}

AbstractJobHandler::SupportActions ErrorMessageAndAction::supportActions(const AbstractJobHandler::JobErrorType &error)
{
    AbstractJobHandler::SupportActions support = AbstractJobHandler::SupportAction::kCancelAction;
    switch (error) {
    case AbstractJobHandler::JobErrorType::kPermissionError:
    case AbstractJobHandler::JobErrorType::kOpenError:
    case AbstractJobHandler::JobErrorType::kReadError:
    case AbstractJobHandler::JobErrorType::kWriteError:
    case AbstractJobHandler::JobErrorType::kSymlinkError:
    case AbstractJobHandler::JobErrorType::kMkdirError:
    case AbstractJobHandler::JobErrorType::kResizeError:
    case AbstractJobHandler::JobErrorType::kRemoveError:
    case AbstractJobHandler::JobErrorType::kRenameError:
    case AbstractJobHandler::JobErrorType::kIntegrityCheckingError:
    case AbstractJobHandler::JobErrorType::kNonexistenceError:
    case AbstractJobHandler::JobErrorType::kDeleteFileError:
    case AbstractJobHandler::JobErrorType::kDeleteTrashFileError:
    case AbstractJobHandler::JobErrorType::kNoSourceError:
    case AbstractJobHandler::JobErrorType::kCancelError:
    case AbstractJobHandler::JobErrorType::kUnknowUrlError:
    case AbstractJobHandler::JobErrorType::kNotSupportedError:
    case AbstractJobHandler::JobErrorType::kPermissionDeniedError:
    case AbstractJobHandler::JobErrorType::kSeekError:
    case AbstractJobHandler::JobErrorType::kProrogramError:
    case AbstractJobHandler::JobErrorType::kDfmIoError:
    case AbstractJobHandler::JobErrorType::kMakeStandardTrashError:
    case AbstractJobHandler::JobErrorType::kGetRestorePathError:
    case AbstractJobHandler::JobErrorType::kIsNotTrashFileError:
    case AbstractJobHandler::JobErrorType::kCreateParentDirError:
    case AbstractJobHandler::JobErrorType::kUnknowError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kRetryAction;
    case AbstractJobHandler::JobErrorType::kSpecialFileError:
        return AbstractJobHandler::SupportAction::kSkipAction;
    case AbstractJobHandler::JobErrorType::kFileSizeTooBigError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kNotEnoughSpaceError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kRetryAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kFileExistsError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kReplaceAction | AbstractJobHandler::SupportAction::kCoexistAction;
    case AbstractJobHandler::JobErrorType::kDirectoryExistsError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kMergeAction | AbstractJobHandler::SupportAction::kCoexistAction;
    case AbstractJobHandler::JobErrorType::kTargetReadOnlyError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kTargetIsSelfError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kSymlinkToGvfsError:
        return support | AbstractJobHandler::SupportAction::kSkipAction;
    default:
        break;
    }

    return support;
}

QString ErrorMessageAndAction::errorToString(const QUrl &url, const AbstractJobHandler::JobErrorType &error)
{
    switch (error) {
    case AbstractJobHandler::JobErrorType::kPermissionError:
        return tr("Permission error");
    case AbstractJobHandler::JobErrorType::kSpecialFileError:
        return tr("The action is denied");
    case AbstractJobHandler::JobErrorType::kFileExistsError:
        return tr("Target file %1 already exists").arg(url.path());
    case AbstractJobHandler::JobErrorType::kDirectoryExistsError:
        return tr("Target directory %1 already exists").arg(url.path());
    case AbstractJobHandler::JobErrorType::kOpenError:
        return tr("Failed to open the file %1").arg(url.path());
    case AbstractJobHandler::JobErrorType::kReadError:
        return tr("Failed to read the file %1").arg(url.path());
    case AbstractJobHandler::JobErrorType::kWriteError:
        return tr("Failed to write the file %1").arg(url.path());
    case AbstractJobHandler::JobErrorType::kMkdirError:
    case AbstractJobHandler::JobErrorType::kCreateParentDirError:
        return tr("Failed to create the directory %1").arg(url.path());
    case AbstractJobHandler::JobErrorType::kRemoveError:
    case AbstractJobHandler::JobErrorType::kDeleteFileError:
        return tr("Failed to delete the file %1").arg(url.path());
    case AbstractJobHandler::JobErrorType::kRenameError:
        return tr("Failed to move the file %1").arg(url.path());
    case AbstractJobHandler::JobErrorType::kNonexistenceError:
        return tr("Original file %1 does not exist").arg(url.path());
    case AbstractJobHandler::JobErrorType::kFileSizeTooBigError:
        return tr("Failed, the file size of %1 must be less than 4 GB").arg(url.path());
    case AbstractJobHandler::JobErrorType::kNotEnoughSpaceError:
        return tr("Not enough free space on the target disk");
    case AbstractJobHandler::JobErrorType::kIntegrityCheckingError:
        return tr("File %1 integrity was damaged").arg(url.path());
    case AbstractJobHandler::JobErrorType::kTargetReadOnlyError:
        return tr("The target device is read only");
    case AbstractJobHandler::JobErrorType::kTargetIsSelfError:
        return tr("Target folder is inside the source folder");
    case AbstractJobHandler::JobErrorType::kNotSupportedError:
        return tr("The action is not supported");
    case AbstractJobHandler::JobErrorType::kPermissionDeniedError:
        return tr("You do not have permission to traverse files in %1").arg(url.path());
    case AbstractJobHandler::JobErrorType::kProrogramError:
        return tr("Unknown error");
    default:
        break;
    }

    return QString();
}

QString ErrorMessageAndAction::errorToStringByCause(const QUrl &url, const AbstractJobHandler::JobErrorType &error,
                                                    const QString &errorMsg)
{
    switch (error) {
    case AbstractJobHandler::JobErrorType::kPermissionError:
        return tr("Permission error");
    case AbstractJobHandler::JobErrorType::kSpecialFileError:
        return tr("The action is denied");
    case AbstractJobHandler::JobErrorType::kFileExistsError:
        return tr("Target file %1 already exists").arg(url.path());
    case AbstractJobHandler::JobErrorType::kDirectoryExistsError:
        return tr("Target directory %1 already exists").arg(url.path());
    case AbstractJobHandler::JobErrorType::kOpenError:
        return tr("Failed to open the file %1, cause: %2").arg(url.path(), errorMsg);
    case AbstractJobHandler::JobErrorType::kReadError:
    case AbstractJobHandler::JobErrorType::kCreateParentDirError:
        return tr("Failed to read the file %1, cause: %2").arg(url.path(), errorMsg);
    case AbstractJobHandler::JobErrorType::kWriteError:
        return tr("Failed to write the file %1, cause: %2").arg(url.path(), errorMsg);
    case AbstractJobHandler::JobErrorType::kMkdirError:
        return tr("Failed to create the directory %1, cause: %2").arg(url.path(), errorMsg);
    case AbstractJobHandler::JobErrorType::kRemoveError:
    case AbstractJobHandler::JobErrorType::kDeleteFileError:
        return tr("Failed to delete the file %1, cause: %2").arg(url.path(), errorMsg);
    case AbstractJobHandler::JobErrorType::kRenameError:
        return tr("Failed to move the file %1, cause: %2").arg(url.path(), errorMsg);
    case AbstractJobHandler::JobErrorType::kNonexistenceError:
        return tr("Original file %1 does not exist").arg(url.path());
    case AbstractJobHandler::JobErrorType::kFileSizeTooBigError:
        return tr("Failed, the file size of %1 must be less than 4 GB").arg(url.path());
    case AbstractJobHandler::JobErrorType::kNotEnoughSpaceError:
        return tr("Not enough free space on the target disk");
    case AbstractJobHandler::JobErrorType::kIntegrityCheckingError:
        return tr("File %1 integrity was damaged, cause: %2").arg(url.path(), errorMsg);
    case AbstractJobHandler::JobErrorType::kTargetReadOnlyError:
        return tr("The target device is read only");
    case AbstractJobHandler::JobErrorType::kTargetIsSelfError:
        return tr("Target folder is inside the source folder");
    case AbstractJobHandler::JobErrorType::kNotSupportedError:
        return tr("The action is not supported");
    case AbstractJobHandler::JobErrorType::kPermissionDeniedError:
        return tr("You do not have permission to traverse files in %1").arg(url.path());
    case AbstractJobHandler::JobErrorType::kSymlinkError:
        return tr("Failed to create symlink, cause: %1").arg(errorMsg);
    default:
        break;
    }

    return QString();
}

void ErrorMessageAndAction::errorSrcAndDestString(const QUrl &from,
                                                  const QUrl &to,
                                                  QString *sorceMsg, QString *toMsg,
                                                  const AbstractJobHandler::JobErrorType error)
{
    if (error == AbstractJobHandler::JobErrorType::kNoError || sorceMsg || toMsg)
        return;
    if (error == AbstractJobHandler::JobErrorType::kFileExistsError
        || error == AbstractJobHandler::JobErrorType::kDirectoryExistsError) {
        *sorceMsg = QString(tr("%1 already exists in target folder")).arg(from.fileName());
        static QLabel label;
        static QFontMetrics metrics(label.font());
        static Qt::TextElideMode em = Qt::TextElideMode::ElideMiddle;
        int pre = metrics.width(tr("Original path %1").arg(from.path()));
        int last = metrics.width(tr("Target path %1").arg(UrlRoute::urlParent(to).path()));
        static int total = 350;
        if (pre > total / 2 && last > total / 2) {
            *toMsg = metrics.elidedText(tr("Original path %1").arg(from.path()), em, total / 2)
                    + " " + metrics.elidedText(tr("Target path %1").arg(UrlRoute::urlParent(to).path()), em, total / 2);
            return;
        }
        if (pre + last > total) {
            *toMsg = pre > total / 2
                    ? metrics.elidedText(tr("Original path %1").arg(from.path()), em, total - last)
                            + " " + tr("Target path %1").arg(UrlRoute::urlParent(to).path())
                    : tr("Original path %1").arg(from.path())
                            + " " + metrics.elidedText(tr("Target path %1").arg(UrlRoute::urlParent(to).path()), em, total - pre);
            return;
        }
        *toMsg = QString(tr("Original path %1 Target path %2"))
                         .arg(from.path(), UrlRoute::urlParent(to).path());
    }
    return;
}
}
