/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef WINDOWSERVICEPRIVATE_H
#define WINDOWSERVICEPRIVATE_H

#include "dfm_filemanager_service_global.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"

#include <QObject>
#include <QPointer>

DSB_FM_BEGIN_NAMESPACE

class WindowsService;
class WindowsServicePrivate : public QObject
{
    friend class WindowsService;
    Q_OBJECT

public:
    explicit WindowsServicePrivate(WindowsService *serv);
    dfmbase::FileManagerWindow *activeExistsWindowByUrl(const QUrl &url);
    void moveWindowToScreenCenter(dfmbase::FileManagerWindow *window);
    bool isValidUrl(const QUrl &url, QString *error);
    void loadWindowState(dfmbase::FileManagerWindow *window);
    void saveWindowState(dfmbase::FileManagerWindow *window);
    void onWindowClosed(dfmbase::FileManagerWindow *window);

private:
    QPointer<WindowsService> service;
    QHash<quint64, dfmbase::FileManagerWindow *> windows;
};

DSB_FM_END_NAMESPACE

#endif   // WINDOWSERVICEPRIVATE_H
