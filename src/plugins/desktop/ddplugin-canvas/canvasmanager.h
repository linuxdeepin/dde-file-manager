/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef CANVASMANAGER_H
#define CANVASMANAGER_H

#include "ddplugin_canvas_global.h"

#include <QObject>

DDP_CANVAS_BEGIN_NAMESPACE
class CanvasView;
class CanvasProxyModel;
class FileInfoModel;
class CanvasSelectionModel;
class CanvasManagerPrivate;
class CanvasManager : public QObject
{
    Q_OBJECT
    friend class CanvasManagerPrivate;

public:
    static CanvasManager *instance();
    void init();
    void update();
    void openEditor(const QUrl &url);
    void setIconLevel(int level);
    FileInfoModel *fileModel() const;
    CanvasProxyModel *model() const;
    CanvasSelectionModel *selectionModel() const;
    QList<QSharedPointer<CanvasView> > views() const;
public slots:
    void onCanvasBuild();
    void onDetachWindows();
    void onGeometryChanged();
    void onWallperSetting(CanvasView *);
    void onChangeIconLevel(bool increase);
    void onTrashStateChanged();
protected slots:
    void reloadItem();
protected:
    explicit CanvasManager(QObject *parent = nullptr);
    ~CanvasManager();
signals:

private:
    CanvasManagerPrivate *d = nullptr;
};
#define CanvasIns CanvasManager::instance()
DDP_CANVAS_END_NAMESPACE
#endif   // CANVASMANAGER_H
