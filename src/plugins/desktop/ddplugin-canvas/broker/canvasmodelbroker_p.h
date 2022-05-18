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
#ifndef CANVASMODELBROKER_P_H
#define CANVASMODELBROKER_P_H

#include "canvasmodelbroker.h"
#include "canvaseventprovider.h"
#include "model/canvasproxymodel.h"

DDP_CANVAS_BEGIN_NAMESPACE

static constexpr char kSlotCanvasModelRootUrl[] = "CanvasModel_Method_rootUrl";
static constexpr char kSlotCanvasModelUrlIndex[] = "CanvasModel_Method_urlIndex";
static constexpr char kSlotCanvasModelIndex[] = "CanvasModel_Method_index";
static constexpr char kSlotCanvasModelFileUrl[] = "CanvasModel_Method_fileUrl";
static constexpr char kSlotCanvasModelFiles[] = "CanvasModel_Method_files";
static constexpr char kSlotCanvasModelShowHiddenFiles[] = "CanvasModel_Method_showHiddenFiles";
static constexpr char kSlotCanvasModelsetShowHiddenFiles[] = "CanvasModel_Method_setShowHiddenFiles";
static constexpr char kSlotCanvasModelSortOrder[] = "CanvasModel_Method_sortOrder";
static constexpr char kSlotCanvasModelSetSortOrder[] = "CanvasModel_Method_setSortOrder";
static constexpr char kSlotCanvasModelSortRole[] = "CanvasModel_Method_sortRole";
static constexpr char kSlotCanvasModelSetSortRole[] = "CanvasModel_Method_SetSortRole";
static constexpr char kSlotCanvasModelRowCount[] = "CanvasModel_Method_rowCount";
static constexpr char kSlotCanvasModelData[] = "CanvasModel_Method_data";
static constexpr char kSlotCanvasModelSort[] = "CanvasModel_Method_sort";
static constexpr char kSlotCanvasModelRefresh[] = "CanvasModel_Method_refresh";
static constexpr char kSlotCanvasModelFetch[] = "CanvasModel_Method_fetch";
static constexpr char kSlotCanvasModelTake[] = "CanvasModel_Method_take";

class CanvasModelBrokerPrivate : public QObject, public CanvasEventProvider
{
    Q_OBJECT
public:
    explicit CanvasModelBrokerPrivate(CanvasModelBroker *);
    ~CanvasModelBrokerPrivate() override;
public:
protected:
    void registerEvent() override;
public:
    CanvasProxyModel *model = nullptr;
private:
    CanvasModelBroker *q;
};

DDP_CANVAS_END_NAMESPACE
#endif // CANVASMODELBROKER_P_H
