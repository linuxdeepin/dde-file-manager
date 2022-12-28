/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhuangshu <zhuangshu@uniontech.com>
 *
 * Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#include "smbbrowsereventreceiver.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/dfm_global_defines.h"

#include <dfm-framework/dpf.h>

#include <QDebug>

using namespace dfmplugin_smbbrowser;
DFMBASE_USE_NAMESPACE

SmbBrowserEventReceiver *SmbBrowserEventReceiver::instance()
{
    static SmbBrowserEventReceiver instance;
    return &instance;
}

bool SmbBrowserEventReceiver::detailViewIcon(const QUrl &url, QString *iconName)
{
    if (UniversalUtils::urlEquals(url, QUrl(QString("%1:///").arg(Global::Scheme::kNetwork)))) {
        *iconName = SystemPathUtil::instance()->systemPathIconName("Network");
        if (!iconName->isEmpty())
            return true;
    }
    return false;
}

SmbBrowserEventReceiver::SmbBrowserEventReceiver(QObject *parent)
    : QObject(parent) {}
