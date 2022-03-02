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
#ifndef DRAGDROPOPER_H
#define DRAGDROPOPER_H

#include "ddplugin_canvas_global.h"
#include "view/canvasview.h"

#include <QObject>
#include <QModelIndex>
#include <QUrl>

DDP_CANVAS_BEGIN_NAMESPACE
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
    void selectItems(const QList<QUrl> &fileUrl) const;
    virtual bool dropFilter(QDropEvent *event);
    bool dropClientDownload(QDropEvent *event) const;
    bool dropBetweenView(QDropEvent *event) const;
    bool dropDirectSaveMode(QDropEvent *event) const;
    bool dropMimeData(QDropEvent *event) const;
private:

protected:
    CanvasView *view;
    QUrl m_target; //must be file:///
};

DDP_CANVAS_END_NAMESPACE
#endif // DRAGDROPOPER_H
