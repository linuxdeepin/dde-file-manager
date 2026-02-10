// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ERRORMESSAGEANDACTION_H
#define ERRORMESSAGEANDACTION_H

#include "dfmplugin_fileoperations_global.h"
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>

namespace dfmplugin_fileoperations {
DFMBASE_USE_NAMESPACE
class ErrorMessageAndAction : public QObject
{
    Q_OBJECT
public:
    static QString errorMsg(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error,
                            const bool isTo = false, const QString &errorMsg = QString(),
                            const bool allUsErrorMsg = false);
    static void srcAndDestString(const QUrl &from, const QUrl &to, QString *fromMsg,
                                 QString *toMsg,
                                 const AbstractJobHandler::JobType jobType,
                                 const AbstractJobHandler::JobErrorType error = AbstractJobHandler::JobErrorType::kNoError);
    static AbstractJobHandler::SupportActions supportActions(const AbstractJobHandler::JobErrorType &error);

private:
    static QString errorToString(const QUrl &url, const AbstractJobHandler::JobErrorType &error);
    static QString errorToStringByCause(const QUrl &url, const AbstractJobHandler::JobErrorType &error,
                                        const QString &errorMsg);
    static void errorSrcAndDestString(const QUrl &from, const QUrl &to, QString *sorceMsg, QString *toMsg,
                                      const AbstractJobHandler::JobErrorType error = AbstractJobHandler::JobErrorType::kNoError);
};
}
#endif   // ERRORMESSAGEANDACTION_H
