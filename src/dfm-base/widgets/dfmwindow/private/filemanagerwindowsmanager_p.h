/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef FILEMANAGERWINDOWSMANAGER_P_H
#define FILEMANAGERWINDOWSMANAGER_P_H

#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <QObject>
#include <QPointer>

DFMBASE_BEGIN_NAMESPACE

class FileManagerWindowsManager;
class FileManagerWindowsManagerPrivate : public QObject
{
    friend class FileManagerWindowsManager;
    Q_OBJECT

public:
    explicit FileManagerWindowsManagerPrivate(FileManagerWindowsManager *serv);
    DFMBASE_NAMESPACE::FileManagerWindow *activeExistsWindowByUrl(const QUrl &url);
    void moveWindowToScreenCenter(DFMBASE_NAMESPACE::FileManagerWindow *window);
    bool isValidUrl(const QUrl &url, QString *error);
    void loadWindowState(DFMBASE_NAMESPACE::FileManagerWindow *window);
    void saveWindowState(DFMBASE_NAMESPACE::FileManagerWindow *window);
    void onWindowClosed(DFMBASE_NAMESPACE::FileManagerWindow *window);
    void onShowHotkeyHelp(DFMBASE_NAMESPACE::FileManagerWindow *window);

private:
    QPointer<FileManagerWindowsManager> manager;
    QHash<quint64, DFMBASE_NAMESPACE::FileManagerWindow *> windows;
    FileManagerWindowsManager::WindowCreator customCreator {};
};

DFMBASE_END_NAMESPACE

#endif   // FILEMANAGERWINDOWSMANAGER_P_H
