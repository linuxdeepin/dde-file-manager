/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef DRAGDROPHELPER_H
#define DRAGDROPHELPER_H

#include "dfm-base/dfm_base_global.h"
#include "dfmplugin_workspace_global.h"

#include <QObject>
#include <QSharedPointer>

DFMBASE_BEGIN_NAMESPACE
class AbstractFileInfo;
DFMBASE_END_NAMESPACE

DPWORKSPACE_BEGIN_NAMESPACE

class FileView;
class DragDropHelper : public QObject
{
    Q_OBJECT
public:
    explicit DragDropHelper(FileView *parent);

    bool dragEnter(QDragEnterEvent *event);
    bool dragMove(QDragMoveEvent *event);
    bool dragLeave(QDragLeaveEvent *event);
    bool drop(QDropEvent *event);

private:
    bool handleDFileDrag(const QMimeData *data, const QUrl &url);
    void handleDropEvent(QDropEvent *event, bool *fall = nullptr);
    QSharedPointer<DFMBASE_NAMESPACE::AbstractFileInfo> fileInfoAtPos(const QPoint &pos);

    bool isSameUser(const QMimeData *data);
    Qt::DropAction checkAction(Qt::DropAction srcAction, bool sameUser);

    FileView *view { nullptr };
    QList<QUrl> currentDragUrls;
};

DPWORKSPACE_END_NAMESPACE

#endif   // DRAGDROPHELPER_H
