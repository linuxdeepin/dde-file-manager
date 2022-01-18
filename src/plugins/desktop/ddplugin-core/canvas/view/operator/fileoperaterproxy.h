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

#include <QObject>

DSB_D_BEGIN_NAMESPACE
class CanvasView;
class FileOperaterProxy : public QObject
{
    Q_OBJECT
public:
    static FileOperaterProxy *instance();
    void touchFile(const CanvasView *view, const dfmbase::Global::CreateFileType type);
    void touchFolder(const CanvasView *view);
    void copyFiles(const CanvasView *view);
    void cutFiles(const CanvasView *view);
    void pasteFiles(const CanvasView *view);
    void openFiles(const CanvasView *view);
    void renameFiles(const CanvasView *view);
    void openFilesByApp(const CanvasView *view);
    void moveToTrash(const CanvasView *view);
    void deleteFiles(const CanvasView *view);

signals:
    void createFileByMenu(const int &screenNum, const QPoint &pos);

protected:
    explicit FileOperaterProxy(QObject *parent = nullptr);
};

#define FileOperaterProxyIns DSB_D_NAMESPACE::FileOperaterProxy::instance()

DSB_D_END_NAMESPACE

#endif // EVENTPROXY_H
