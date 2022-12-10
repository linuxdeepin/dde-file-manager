/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhuangshu<zhuangshu@uniontech.com>
 *
 * Maintainer: zhuangshu<zhuangshu@uniontech.com>
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
#include "reportlogeventreceiver.h"
#include "rlog/rlog.h"

#include "dfm-base/base/schemefactory.h"

#include <dfm-framework/dpf.h>

using namespace dfmbase;
using namespace dfmplugin_utils;

void ReportLogEventReceiver::bindEvents()
{
    // connect all the slot events of plugins which need report log.
    dpfSlotChannel->connect("dfmplugin_utils", "slot_ReportLog_Commit", this, &ReportLogEventReceiver::commit);
    dpfSlotChannel->connect("dfmplugin_utils", "slot_ReportLog_ReportMenuData", this, &ReportLogEventReceiver::handleMenuData);
}

ReportLogEventReceiver::ReportLogEventReceiver(QObject *parent)
    : QObject(parent)
{
}

void ReportLogEventReceiver::commit(const QString &type, const QVariantMap &args)
{
    RLog::instance()->commit(type, args);
}

void ReportLogEventReceiver::handleMenuData(const QString &name, const QList<QUrl> &urlList)
{
    QVariantMap data {};
    data.insert("item_name", name);

    QString location("");
    QStringList types {};

    if (urlList.count() > 0) {
        location = "File";

        for (auto url : urlList) {
            auto info = InfoFactory::create<AbstractFileInfo>(url);
            if (info) {
                QString type = info->nameInfo(NameInfo::kMimeTypeName);
                if (!types.contains(type))
                    types << type;
            }
        }

    } else {
        location = "Workspace";
    }

    data.insert("location", location);
    data.insert("type", types);

    RLog::instance()->commit("FileMenu", data);
}
