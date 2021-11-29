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
#ifndef DFMWINDOWMANAGERSERVICE_H
#define DFMWINDOWMANAGERSERVICE_H

#include "browsewindow.h"
#include "displayviewlogic.h"
#include "dfm_filemanager_service_global.h"

#include <dfm-framework/framework.h>

#include <QUrl>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHash>

DWIDGET_USE_NAMESPACE

class Core;
class CoreEventReceiver;

DSB_FM_BEGIN_NAMESPACE

class WindowService final : public dpf::PluginService, dpf::AutoServiceRegister<WindowService>
{
    Q_OBJECT
    Q_DISABLE_COPY(WindowService)

    QHash<quint64, BrowseWindow *> windowHash;

    //私有方法的权限控制，core拥有访问私有方法权限
    friend class ::Core;
    friend class ::CoreEventReceiver;

public:
    static QString name()
    {
        return "org.deepin.service.WindowService";
    }

    explicit WindowService(QObject *parent = nullptr);
    virtual ~WindowService() override;
    bool addSideBarItem(quint64 windowIndex, SideBarItem *item);
    bool removeSideBarItem(quint64 windowIndex, SideBarItem *item);
    bool insertSideBarItem(quint64 windowIndex, int row, SideBarItem *item);
    bool addDetailViewItem(quint64 windowIndex, QWidget *widget);
    bool insertDetailViewItem(quint64 windowIndex, int index, QWidget *widget);

private:   //@Method
    BrowseWindow *newWindow();
    bool setWindowRootUrl(BrowseWindow *newWindow,
                          const QUrl &url,
                          QString *errorString = nullptr);
    bool setWindowRootUrl(quint64 winIdx,
                          const QUrl &url,
                          QString *errorString = nullptr);
};

DSB_FM_END_NAMESPACE

#endif   // DFMWINDOWMANAGERSERVICE_H
