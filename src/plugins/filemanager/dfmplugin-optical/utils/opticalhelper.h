/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef OPTICALHELPER_H
#define OPTICALHELPER_H

#include "dfmplugin_optical_global.h"

#include "services/filemanager/windows/windowsservice.h"
#include "services/filemanager/titlebar/titlebarservice.h"
#include "services/filemanager/workspace/workspaceservice.h"

#include <QIcon>
#include <QRegularExpression>

static constexpr char kDiscburnStaging[] { "discburn" };
static constexpr char kBurnSegOndisc[] { "disc_files" };
static constexpr char kBurnSegStaging[] { "staging_files" };

DPOPTICAL_BEGIN_NAMESPACE

class OpticalHelper
{
public:
    static QIcon icon();
    static QString iconString();
    static QUrl localStagingFile(const QUrl &dest);
    static QUrl localStagingFile(QString dev);
    static QString burnDestDevice(const QUrl &url);
    static QString burnFilePath(const QUrl &url);
    static bool burnIsOnDisc(const QUrl &url);
    static QUrl tansToBurnFile(const QUrl &in);
    static QString deviceId(const QString &device);

    static DSB_FM_NAMESPACE::WindowsService *winServIns();
    static DSB_FM_NAMESPACE::TitleBarService *titleServIns();
    static DSB_FM_NAMESPACE::WorkspaceService *workspaceServIns();

private:
    static QRegularExpression burnRxp();
};

DPOPTICAL_END_NAMESPACE

#endif   // OPTICALHELPER_H
