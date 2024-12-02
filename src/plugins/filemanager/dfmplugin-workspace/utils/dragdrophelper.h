// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRAGDROPHELPER_H
#define DRAGDROPHELPER_H

#include <dfm-base/dfm_base_global.h>
#include "dfmplugin_workspace_global.h"

#include <dfm-base/mimedata/dfmmimedata.h>

#include <QObject>
#include <QSharedPointer>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QModelIndex>

namespace dfmbase {
class FileInfo;
}

namespace dfmplugin_workspace {

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

    bool isDragTarget(const QModelIndex &index) const;

private:
    bool handleDFileDrag(const QMimeData *data, const QUrl &url);
    void handleDropEvent(QDropEvent *event, bool *fall = nullptr);
    QSharedPointer<DFMBASE_NAMESPACE::FileInfo> fileInfoAtPos(const QPoint &pos);

    bool checkProhibitPaths(QDragEnterEvent *event, const QList<QUrl> &urls) const;
    bool checkTargetEnable(const QUrl &targetUrl) const;
    Qt::DropAction checkAction(Qt::DropAction srcAction, bool sameUser);

    FileView *view { nullptr };
    QList<QUrl> currentDragUrls;
    QList<QUrl> currentDragSourceUrls;
    QUrl currentHoverIndexUrl;
    bool dragFileFromCurrent { false };
    DFMBASE_NAMESPACE::DFMMimeData dfmmimeData;
};

}   // namespace dfmplugin_workspace

#endif   // DRAGDROPHELPER_H
