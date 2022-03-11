/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#ifndef OPENWITHSERVICE_H
#define OPENWITHSERVICE_H

#include "dfm_common_service_global.h"

#include <dfm-framework/service/pluginservicecontext.h>

DSC_BEGIN_NAMESPACE

class OpenWithService final : public dpf::PluginService, dpf::AutoServiceRegister<OpenWithService>
{
    Q_OBJECT
    Q_DISABLE_COPY(OpenWithService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.OpenWithService";
    }

    static OpenWithService *service();

public slots:
    void showOpenWithDialog(const QList<QUrl> &urls);

private:
    explicit OpenWithService(QObject *parent = nullptr);
    virtual ~OpenWithService() override;
};

DSC_END_NAMESPACE

#endif   // OPENWITHSERVICE_H
