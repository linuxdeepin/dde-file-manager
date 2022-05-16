/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
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
#ifndef CANVASMANAGER_P_H
#define CANVASMANAGER_P_H

#include "canvasmanager.h"
#include "view/canvasview.h"
#include "delegate/canvasitemdelegate.h"
#include "model/fileinfomodel.h"
#include "extend/canvasmanagerextend.h"
#include "extend/canvasmodelextend.h"
#include "extend/canvasviewextend.h"
#include "broker/canvasmanagerbroker.h"
#include "broker/fileinfomodelbroker.h"
#include "broker/canvasmodelbroker.h"
#include "broker/canvasviewbroker.h"
#include "broker/canvasgridbroker.h"

#include <services/desktop/frame/frameservice.h>

DDP_CANVAS_BEGIN_NAMESPACE

typedef QSharedPointer<CanvasView> CanvasViewPointer;

class CanvasManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit CanvasManagerPrivate(CanvasManager *qq);
    ~CanvasManagerPrivate();
    void initModel();
    void initSetting();
    CanvasViewPointer createView(QWidget *root, int index);
    void updateView(const CanvasViewPointer &, QWidget *root, int index);
public:
    inline QRect relativeRect(const QRect &avRect, const QRect &geometry)
    {
        QPoint relativePos = avRect.topLeft() - geometry.topLeft();
        return QRect(relativePos, avRect.size());
    }

public slots:
    void onHiddenFlagsChanged(bool);
    void onFileRenamed(const QUrl &oldUrl, const QUrl &newUrl);
    void onFileInserted(const QModelIndex &parent, int first, int last);
    void onFileAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void onFileDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles );
    void onFileModelReset();
    void onFileSorted();

protected slots:

public:
    CanvasManagerExtend *extend = nullptr;
    CanvasManagerBroker *broker = nullptr;
    FileInfoModel *sourceModel = nullptr;
    CanvasProxyModel *canvasModel = nullptr;
    CanvasModelExtend *modelExt = nullptr;
    CanvasSelectionModel *selectionModel = nullptr ;
    DSB_D_NAMESPACE::FrameService *frameService = nullptr;
    CanvasViewExtend *viewExt = nullptr;
    QMap<QString, CanvasViewPointer> viewMap;
public:
    FileInfoModelBroker *sourceModelBroker = nullptr;
    CanvasModelBroker *modelBroker = nullptr;
    CanvasViewBroker *viewBroker = nullptr;
    CanvasGridBroker *gridBroker = nullptr;
private:
    CanvasManager *q = nullptr;
};

DDP_CANVAS_END_NAMESPACE
#endif   // CANVASMANAGER_P_H
