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

inline constexpr char kSlotCanvasModelRootUrl[] = "CanvasModel_Method_rootUrl";
inline constexpr char kSlotCanvasModelUrlIndex[] = "CanvasModel_Method_urlIndex";
inline constexpr char kSlotCanvasModelIndex[] = "CanvasModel_Method_index";
inline constexpr char kSlotCanvasModelFileUrl[] = "CanvasModel_Method_fileUrl";
inline constexpr char kSlotCanvasModelFiles[] = "CanvasModel_Method_files";
inline constexpr char kSlotCanvasModelShowHiddenFiles[] = "CanvasModel_Method_showHiddenFiles";
inline constexpr char kSlotCanvasModelsetShowHiddenFiles[] = "CanvasModel_Method_setShowHiddenFiles";
inline constexpr char kSlotCanvasModelSortOrder[] = "CanvasModel_Method_sortOrder";
inline constexpr char kSlotCanvasModelSetSortOrder[] = "CanvasModel_Method_setSortOrder";
inline constexpr char kSlotCanvasModelSortRole[] = "CanvasModel_Method_sortRole";
inline constexpr char kSlotCanvasModelSetSortRole[] = "CanvasModel_Method_SetSortRole";
inline constexpr char kSlotCanvasModelRowCount[] = "CanvasModel_Method_rowCount";
inline constexpr char kSlotCanvasModelData[] = "CanvasModel_Method_data";
inline constexpr char kSlotCanvasModelSort[] = "CanvasModel_Method_sort";
inline constexpr char kSlotCanvasModelRefresh[] = "CanvasModel_Method_refresh";
inline constexpr char kSlotCanvasModelFetch[] = "CanvasModel_Method_fetch";
inline constexpr char kSlotCanvasModelTake[] = "CanvasModel_Method_take";

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
#endif   // CANVASMODELBROKER_P_H
