/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: zhangsheng<zhangsheng@uniontech.com>
 *             lvwujun<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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
#ifndef USERSHARESERVICE_H
#define USERSHARESERVICE_H

#include "dfm_common_service_global.h"

#include <dfm-framework/service/pluginservicecontext.h>

DSC_BEGIN_NAMESPACE

class UserShareService final : public dpf::PluginService, dpf::AutoServiceRegister<UserShareService>
{
    Q_OBJECT
    Q_DISABLE_COPY(UserShareService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.UserShareService";
    }

Q_SIGNALS:
    void shareCountChanged(int count);
    void shareAdded(const QString &path);
    void shareRemoved(const QString &path);
    void shareRemoveFailed(const QString &path);

private:
    explicit UserShareService(QObject *parent = nullptr);
    virtual ~UserShareService() override;
};

DSC_END_NAMESPACE
#endif   // USERSHARESERVICE_H
