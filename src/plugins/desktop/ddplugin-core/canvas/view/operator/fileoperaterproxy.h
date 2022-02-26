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
#ifndef EVENTPROXY_H
#define EVENTPROXY_H

#include "dfm_desktop_service_global.h"
#include "dfm_global_defines.h"
#include "dfm-base/dfm_global_defines.h"

#include <QObject>
#include <QPoint>

DSB_D_BEGIN_NAMESPACE
class CanvasView;
class FileOperaterProxyPrivate;
class FileOperaterProxy : public QObject
{
    Q_OBJECT
public:
    static FileOperaterProxy *instance();
    void touchFile(const CanvasView *view, const QPoint pos, const dfmbase::Global::CreateFileType type, QString suffix = "");
    void touchFolder(const CanvasView *view, const QPoint pos);
    void copyFiles(const CanvasView *view);
    void cutFiles(const CanvasView *view);
    void pasteFiles(const CanvasView *view, const QPoint pos = QPoint(0, 0));
    void openFiles(const CanvasView *view);
    void openFiles(const CanvasView *view, const QList<QUrl> &urls);
    void renameFile(const CanvasView *view, const QUrl &oldUrl, const QUrl &newUrl);
    void renameFiles(const CanvasView *view, const QList<QUrl> &urls, const QPair<QString, QString> &pair, const bool replace);
    void renameFiles(const CanvasView *view, const QList<QUrl> &urls, const QPair<QString, dfmbase::AbstractJobHandler::FileBatchAddTextFlags> pair);
    void openFilesByApp(const CanvasView *view);
    void moveToTrash(const CanvasView *view);
    void deleteFiles(const CanvasView *view);
    void showFilesProperty(const CanvasView *view);

    void dropFiles(const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls);
    void dropToTrash(const QList<QUrl> &urls);
    void dropToApp(const QList<QUrl> &urls, const QString &app);

public:
    static void callBackFunction(const dfmbase::Global::CallbackArgus args);

protected:
    explicit FileOperaterProxy(QObject *parent = nullptr);

private:
    QSharedPointer<FileOperaterProxyPrivate> d;
};

#define FileOperaterProxyIns DSB_D_NAMESPACE::FileOperaterProxy::instance()

DSB_D_END_NAMESPACE

#endif // EVENTPROXY_H
