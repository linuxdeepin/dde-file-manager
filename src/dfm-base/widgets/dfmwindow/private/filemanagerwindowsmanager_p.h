// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEMANAGERWINDOWSMANAGER_P_H
#define FILEMANAGERWINDOWSMANAGER_P_H

#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

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
    bool isValidUrl(const QUrl &url, QString *error);
    void onWindowClosed(DFMBASE_NAMESPACE::FileManagerWindow *window);
    void onShowHotkeyHelp(DFMBASE_NAMESPACE::FileManagerWindow *window);

private:
    QPointer<FileManagerWindowsManager> manager;
    QHash<quint64, DFMBASE_NAMESPACE::FileManagerWindow *> windows;
    FileManagerWindowsManager::WindowCreator customCreator {};
    quint64 previousActivedWindowId = 0;
};

DFMBASE_END_NAMESPACE

#endif   // FILEMANAGERWINDOWSMANAGER_P_H
