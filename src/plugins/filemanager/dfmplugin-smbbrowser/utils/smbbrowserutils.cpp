/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "smbbrowserutils.h"

#include "plugins/common/dfmplugin-utils/reportlog/rlog/datas/smbreportdata.h"
#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/dfm_global_defines.h"

#include <dfm-framework/dpf.h>

#include <QUrl>

Q_DECLARE_METATYPE(const char *)

using namespace dfmplugin_smbbrowser;

QMutex SmbBrowserUtils::mutex;
QMap<QUrl, SmbShareNode> SmbBrowserUtils::shareNodes;

SmbBrowserUtils *SmbBrowserUtils::instance()
{
    static SmbBrowserUtils instance;
    return &instance;
}

QString SmbBrowserUtils::networkScheme()
{
    return "network";
}

QUrl SmbBrowserUtils::netNeighborRootUrl()
{
    QUrl u;
    u.setScheme(networkScheme());
    u.setPath("/");
    return u;
}

QIcon SmbBrowserUtils::icon()
{
    return QIcon::fromTheme("network-server-symbolic");
}

bool SmbBrowserUtils::mountSmb(const quint64 windowId, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != DFMBASE_NAMESPACE::Global::Scheme::kSmb
        && urls.first().scheme() != DFMBASE_NAMESPACE::Global::Scheme::kFtp
        && urls.first().scheme() != DFMBASE_NAMESPACE::Global::Scheme::kSFtp)
        return false;

    if (urls.count() != 1)
        return false;

    DFMBASE_USE_NAMESPACE
    QUrl url = urls.first();
    QString urlStr = url.toString();
    DeviceManager::instance()->mountNetworkDeviceAsync(urlStr, [windowId, url, this](bool ok, DFMMOUNT::DeviceError err, const QString &mpt) {
        if (!ok && err != DFMMOUNT::DeviceError::kGIOErrorAlreadyMounted) {
            DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DFMBASE_NAMESPACE::DialogManager::kMount, err);
        } else {
            QUrl u = mpt.isEmpty() ? url : QUrl::fromLocalFile(mpt);
            dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, windowId, u);
        }
    });
    return true;
}
