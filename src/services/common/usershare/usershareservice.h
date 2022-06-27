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
#include "usershare_defines.h"

#include <dfm-framework/service/pluginservicecontext.h>

namespace dfm_service_common {

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

    static UserShareService *service();

    void startSambaService(StartSambaFinished onFinished);
    bool isSambaServiceRunning();
    void setSharePassword(const QString &userName, const QString &passwd);

    bool addShare(const ShareInfo &share);
    void removeShare(const QString &path);
    bool isSharedPath(const QString &path);

    ShareInfoList shareInfos();
    ShareInfo getInfoByPath(const QString &path);
    ShareInfo getInfoByName(const QString &shareName);
    QString getShareNameByPath(const QString &path);
    QString getCurrentUserName();
    uint getUidByShareName(const QString &name);

Q_SIGNALS:
    void shareCountChanged(int count);
    void shareAdded(const QString &path);
    void shareRemoved(const QString &path);
    void shareRemoveFailed(const QString &path);

private:
    explicit UserShareService(QObject *parent = nullptr);
    virtual ~UserShareService() override;

    void initConnect();
    void initEventHandlers();
};

}
#endif   // USERSHARESERVICE_H
