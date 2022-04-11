/*
 * Copyright (C) 2022 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef FILEOPERATORPROXY_H
#define FILEOPERATORPROXY_H

#include "ddplugin_canvas_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <QObject>
#include <QPoint>

DDP_CANVAS_BEGIN_NAMESPACE
class CanvasView;
class FileOperatorProxyPrivate;
class FileOperatorProxy : public QObject
{
    Q_OBJECT
public:
    static FileOperatorProxy *instance();
    void touchFile(const CanvasView *view, const QPoint pos, const DFMBASE_NAMESPACE::Global::CreateFileType type, QString suffix = "");
    void touchFolder(const CanvasView *view, const QPoint pos);
    void copyFiles(const CanvasView *view);
    void cutFiles(const CanvasView *view);
    void pasteFiles(const CanvasView *view, const QPoint pos = QPoint(0, 0));
    void openFiles(const CanvasView *view);
    void openFiles(const CanvasView *view, const QList<QUrl> &urls);
    void renameFile(const CanvasView *view, const QUrl &oldUrl, const QUrl &newUrl);
    void renameFiles(const CanvasView *view, const QList<QUrl> &urls, const QPair<QString, QString> &pair, const bool replace);
    void renameFiles(const CanvasView *view, const QList<QUrl> &urls, const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> pair);
    void openFilesByApp(const CanvasView *view);
    void moveToTrash(const CanvasView *view);
    void deleteFiles(const CanvasView *view);
    void showFilesProperty(const CanvasView *view);
    void sendFilesToBluetooth(const CanvasView *view);

    void dropFiles(const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls);
    void dropToTrash(const QList<QUrl> &urls);
    void dropToApp(const QList<QUrl> &urls, const QString &app);

public:
    void callBackFunction(const DFMBASE_NAMESPACE::Global::CallbackArgus args);

protected:
    explicit FileOperatorProxy(QObject *parent = nullptr);

private:
    QSharedPointer<FileOperatorProxyPrivate> d;
};

#define FileOperatorProxyIns DDP_CANVAS_NAMESPACE::FileOperatorProxy::instance()

DDP_CANVAS_END_NAMESPACE

#endif   // FILEOPERATORPROXY_H
