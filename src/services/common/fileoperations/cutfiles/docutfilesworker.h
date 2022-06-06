/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#ifndef DOCUTFILESWORKER_H
#define DOCUTFILESWORKER_H

#include "dfm_common_service_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "fileoperations/fileoperationutils/abstractworker.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"

#include "dfm-io/core/dfile.h"

#include <QObject>

DSC_BEGIN_NAMESPACE

class StorageInfo;

class DoCutFilesWorker : public FileOperateBaseWorker
{
    friend class CutFiles;
    Q_OBJECT
    explicit DoCutFilesWorker(QObject *parent = nullptr);

public:
    virtual ~DoCutFilesWorker() override;

protected:
    bool doWork() override;
    void stop() override;
    bool initArgs() override;
    void onUpdateProccess() override;

    bool cutFiles();
    bool doCutFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo);
    bool doRenameFile(const AbstractFileInfoPointer &sourceInfo, const AbstractFileInfoPointer &targetInfo, bool *ok = nullptr);
    bool renameFileByHandler(const AbstractFileInfoPointer &sourceInfo, const AbstractFileInfoPointer &targetInfo);

    void emitCompleteFilesUpdatedNotify(const qint64 &writCount);
    void doOperateWork(AbstractJobHandler::SupportActions actions) override;

private:
    bool checkSymLink(const AbstractFileInfoPointer &fromInfo);
    bool checkSelf(const AbstractFileInfoPointer &fromInfo);
};

DSC_END_NAMESPACE

#endif   // DOCUTFILESWORKER_H
