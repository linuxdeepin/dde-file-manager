/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "dfm-framework/service/pluginservice.h"

#include <QUrl>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

DWIDGET_USE_NAMESPACE

class Core;

DSB_FM_BEGIN_NAMESPACE

class WindowService : public dpf::PluginService
{
    Q_OBJECT
    Q_DISABLE_COPY(WindowService)

    QList<DFMBrowseWindow*> m_windowlist;

    //私有方法的权限控制，core拥有访问私有方法权限
    friend class ::Core;

public:
    explicit WindowService();

    virtual ~WindowService() override;

private: //@Method
    DFMBrowseWindow* newWindow();

    bool setWindowRootUrl(DFMBrowseWindow * newWindow,
                          const QUrl &url,
                          QString *errorString = nullptr);

}; PLUGIN_SERVICE(WindowService);

DSB_FM_END_NAMESPACE

#endif // DFMWINDOWMANAGERSERVICE_H
