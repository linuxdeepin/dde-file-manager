/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef FILEINFOMODELBROKER_P_H
#define FILEINFOMODELBROKER_P_H

#include "fileinfomodelbroker.h"
#include "canvaseventprovider.h"

DDP_CANVAS_BEGIN_NAMESPACE

static constexpr char kSlotFileInfoModelRootUrl[] = "FileInfoModel_Method_rootUrl";
static constexpr char kSlotFileInfoModelRootIndex[] = "FileInfoModel_Method_rootIndex";
static constexpr char kSlotFileInfoModelUrlIndex[] = "FileInfoModel_Method_urlIndex";
static constexpr char kSlotFileInfoModelIndexUrl[] = "FileInfoModel_Method_indexUrl";
static constexpr char kSlotFileInfoModelFiles[] = "FileInfoModel_Method_files";
static constexpr char kSlotFileInfoModelFileInfo[] = "FileInfoModel_Method_fileInfo";
static constexpr char kSlotFileInfoModelRefresh[] = "FileInfoModel_Method_refresh";
static constexpr char kSignalFileInfoModelDataReplaced[] = "FileInfoModel_Signal_dataReplaced";

class FileInfoModelBrokerPrivate : public QObject, public CanvasEventProvider
{
    Q_OBJECT
public:
    explicit FileInfoModelBrokerPrivate(FileInfoModelBroker *);
protected:
    void registerEvent() override;
public:
    FileInfoModel *model = nullptr;
private:
    FileInfoModelBroker *q;
};

DDP_CANVAS_END_NAMESPACE

#endif // FILEINFOMODELBROKER_P_H
