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
#ifndef FILEMANAGERWINDOWSMANAGER_H
#define FILEMANAGERWINDOWSMANAGER_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"

#include <QObject>

#include <functional>

DFMBASE_BEGIN_NAMESPACE

class FileManagerWindowsManagerPrivate;
class FileManagerWindowsManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileManagerWindowsManager)

public:
    using FMWindow = DFMBASE_NAMESPACE::FileManagerWindow;
    using WindowCreator = std::function<FMWindow *(const QUrl &)>;

public:
    static FileManagerWindowsManager &instance();

    void setCustomWindowCreator(WindowCreator creator);
    FMWindow *createWindow(const QUrl &url, bool isNewWindow = false, QString *errorString = nullptr);
    FMWindow *showWindow(const QUrl &url, bool isNewWindow = false, QString *errorString = nullptr);
    void showWindow(FMWindow *window);
    quint64 findWindowId(const QWidget *window);
    FMWindow *findWindowById(quint64 winId);
    QList<quint64> windowIdList();
    quint64 previousActivedWindowId();

Q_SIGNALS:
    void windowCreated(quint64 windId);
    void windowOpened(quint64 windId);
    void windowClosed(quint64 windId);
    void lastWindowClosed();

private:
    explicit FileManagerWindowsManager(QObject *parent = nullptr);
    ~FileManagerWindowsManager() override;

private:
    QScopedPointer<FileManagerWindowsManagerPrivate> d;
};

DFMBASE_END_NAMESPACE

#define FMWindowsIns DFMBASE_NAMESPACE::FileManagerWindowsManager::instance()

#endif   // FILEMANAGERWINDOWSMANAGER_H
