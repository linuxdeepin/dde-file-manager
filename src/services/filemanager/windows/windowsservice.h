/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#ifndef DFMWINDOWSERVICE_H
#define DFMWINDOWSERVICE_H

#include "dfm_filemanager_service_global.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"

#include <dfm-framework/framework.h>

DWIDGET_USE_NAMESPACE

DSB_FM_BEGIN_NAMESPACE

class WindowsServicePrivate;
class WindowsService final : public dpf::PluginService, dpf::AutoServiceRegister<WindowsService>
{
    Q_OBJECT
    Q_DISABLE_COPY(WindowsService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    using FMWindow = dfmbase::FileManagerWindow;

    static QString name()
    {
        return "org.deepin.service.WindowService";
    }

    explicit WindowsService(QObject *parent = nullptr);
    virtual ~WindowsService() override;

    FMWindow *showWindow(const QUrl &url, bool isNewWindow = false, QString *errorString = nullptr);
    quint64 findWindowId(const QWidget *window);
    FMWindow *findWindowById(quint64 winId);
    QList<quint64> windowIdList();

signals:
    void windowOpended(quint64 windId);
    void windowClosed(quint64 winId);

private:
    QScopedPointer<WindowsServicePrivate> d;
};

DSB_FM_END_NAMESPACE

#endif   // DFMWINDOWSERVICE_H
