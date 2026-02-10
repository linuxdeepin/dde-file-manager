// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRAGDROPOPER_H
#define DRAGDROPOPER_H

#include "ddplugin_canvas_global.h"
#include "view/canvasview.h"

#include <dfm-base/mimedata/dfmmimedata.h>

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

    inline QModelIndex hoverIndex() const{
        return dragHoverIndex;
    }
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
    void updateDragHover(const QPoint &pos);
    void stopDelayDodge();
    void updateDFMMimeData(QDropEvent *event);
    bool checkTargetEnable(const QUrl &targetUrl);
    bool checkSourceValid(const QList<QUrl> &srcUrls);

protected:
    CanvasView *view = nullptr;
    QUrl m_target;   //must be file:///
    QPersistentModelIndex dragHoverIndex;
    DFMBASE_NAMESPACE::DFMMimeData dfmmimeData;
};

}
#endif   // DRAGDROPOPER_H
