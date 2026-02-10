// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASMANAGER_H
#define CANVASMANAGER_H

#include "ddplugin_canvas_global.h"

#include <QObject>

namespace ddplugin_canvas {
class CanvasView;
class CanvasProxyModel;
class FileInfoModel;
class CanvasSelectionModel;
class CanvasManagerPrivate;
class CanvasManager : public QObject
{
    Q_OBJECT
    friend class CanvasManagerPrivate;
    friend class CanvasDBusInterface;
public:
    explicit CanvasManager(QObject *parent = nullptr);
    ~CanvasManager();
    static CanvasManager *instance();
    void init();
    void update();
    void openEditor(const QUrl &url);
    void setIconLevel(int level);
    int iconLevel() const;
    bool autoArrange() const;
    void setAutoArrange(bool on);
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
    void onFontChanged();
    void refresh(bool silent);
protected slots:
    void reloadItem();
signals:

private:
    CanvasManagerPrivate *d = nullptr;
};
#define CanvasIns CanvasManager::instance()
}
#endif   // CANVASMANAGER_H
