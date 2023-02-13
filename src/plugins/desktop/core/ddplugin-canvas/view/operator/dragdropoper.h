// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRAGDROPOPER_H
#define DRAGDROPOPER_H

#include "ddplugin_canvas_global.h"
#include "view/canvasview.h"

#include <QObject>
#include <QModelIndex>
#include <QUrl>

namespace ddplugin_canvas {
class CanvasView;
class DragDropOper : public QObject
{
    Q_OBJECT
public:
    explicit DragDropOper(CanvasView *parent);
    bool enter(QDragEnterEvent *event);
    void leave(QDragLeaveEvent *event);
    bool move(QDragMoveEvent *event);
    bool drop(QDropEvent *event);
signals:

public slots:
protected:
    virtual void preproccessDropEvent(QDropEvent *event, const QList<QUrl> &urls, const QUrl &targetFileUrl) const;
    void updateTarget(const QMimeData *data, const QUrl &url);
    bool checkXdndDirectSave(QDragEnterEvent *event) const;
    bool checkProhibitPaths(QDragEnterEvent *event) const;
    void selectItems(const QList<QUrl> &fileUrl) const;
    virtual bool dropFilter(QDropEvent *event);
    bool dropClientDownload(QDropEvent *event) const;
    bool dropBetweenView(QDropEvent *event) const;
    bool dropDirectSaveMode(QDropEvent *event) const;
    bool dropMimeData(QDropEvent *event) const;
    void handleMoveMimeData(QDropEvent *event, const QUrl &url);

private:
    void updatePrepareDodgeValue(QEvent *event);
    void tryDodge(QDragMoveEvent *event);
    void stopDelayDodge();

protected:
    CanvasView *view;
    QUrl m_target;   //must be file:///
};

}
#endif   // DRAGDROPOPER_H
