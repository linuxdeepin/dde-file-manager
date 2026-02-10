// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASMANAGER_P_H
#define CANVASMANAGER_P_H

#include "canvasmanager.h"
#include "view/canvasview.h"
#include "delegate/canvasitemdelegate.h"
#include "model/fileinfomodel.h"
#include "model/canvasproxymodel.h"
#include "model/canvasselectionmodel.h"
#include "hook/canvasmanagerhook.h"
#include "hook/canvasmodelhook.h"
#include "hook/canvasviewhook.h"
#include "hook/canvasselectionhook.h"
#include "broker/canvasmanagerbroker.h"
#include "broker/fileinfomodelbroker.h"
#include "broker/canvasmodelbroker.h"
#include "broker/canvasviewbroker.h"
#include "broker/canvasgridbroker.h"
#include "recentproxy/canvasrecentproxy.h"

namespace ddplugin_canvas {

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
    void onAboutToFileSort();
    void onFileSorted();

protected slots:

public:
    CanvasManagerHook *hookIfs = nullptr;
    CanvasManagerBroker *broker = nullptr;
    FileInfoModel *sourceModel = nullptr;
    CanvasProxyModel *canvasModel = nullptr;
    CanvasModelHook *modelHook = nullptr;
    CanvasSelectionModel *selectionModel = nullptr;
    CanvasSelectionHook *selectionHook = nullptr;
    CanvasViewHook *viewHook = nullptr;
    CanvasRecentProxy* recentFileProxy = nullptr;
    QMap<QString, CanvasViewPointer> viewMap;
public:
    FileInfoModelBroker *sourceModelBroker = nullptr;
    CanvasModelBroker *modelBroker = nullptr;
    CanvasViewBroker *viewBroker = nullptr;
    CanvasGridBroker *gridBroker = nullptr;
    static CanvasManager *global;
private:
    CanvasManager *q = nullptr;
};

}
#endif   // CANVASMANAGER_P_H
