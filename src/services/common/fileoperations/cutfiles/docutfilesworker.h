/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
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
#ifndef DOCUTFILESWORKER_H
#define DOCUTFILESWORKER_H

#include "dfm_common_service_global.h"
#include "dfm-base/base/abstractjobhandler.h"
#include "fileoperations/fileoperationutils/abstractworker.h"

#include <QObject>

DSC_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE
class DoCutFilesWorker : public AbstractWorker
{
    friend class CutFiles;
    Q_OBJECT
    explicit DoCutFilesWorker(QObject *parent = nullptr);

public:
    virtual ~DoCutFilesWorker() override;

protected:
    void doWork() override;
    void stop() override;
    void pause() override;
};

DSC_END_NAMESPACE

#endif   // DOCUTFILESWORKER_H
