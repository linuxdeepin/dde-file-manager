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
#ifndef ERRORMESSAGEANDACTION_H
#define ERRORMESSAGEANDACTION_H

#include "dfmplugin_fileoperations_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"

#include <QObject>

DPFILEOPERATIONS_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE
class ErrorMessageAndAction : public QObject
{
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
DPFILEOPERATIONS_END_NAMESPACE
#endif   // ERRORMESSAGEANDACTION_H
