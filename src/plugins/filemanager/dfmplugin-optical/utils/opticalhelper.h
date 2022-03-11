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
#include "services/common/fileoperations/fileoperationsservice.h"
#include "services/common/delegate/delegateservice.h"
#include "services/common/menu/menuservice.h"

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
    static QUrl localStagingRoot();
    static QUrl localStagingFile(const QUrl &dest);
    static QUrl localStagingFile(QString dev);
    static QUrl localDiscFile(const QUrl &dest);
    static QString burnDestDevice(const QUrl &url);
    static QString burnFilePath(const QUrl &url);
    static bool burnIsOnDisc(const QUrl &url);
    static bool burnIsOnStaging(const QUrl &url);
    static QUrl tansToBurnFile(const QUrl &in);
    static QUrl tansToLocalFile(const QUrl &in);
    static bool isSupportedUDFVersion(const QString &version);
    static bool isSupportedUDFMedium(int type);
    static void createStagingFolder(const QString &path);

    // services instance
    static DSB_FM_NAMESPACE::WindowsService *winServIns();
    static DSB_FM_NAMESPACE::TitleBarService *titleServIns();
    static DSB_FM_NAMESPACE::WorkspaceService *workspaceServIns();
    static DSC_NAMESPACE::FileOperationsService *fileOperationsServIns();
    static DSC_NAMESPACE::DelegateService *dlgateServIns();
    static DSC_NAMESPACE::MenuService *menuServIns();

private:
    static QRegularExpression burnRxp();
};

DPOPTICAL_END_NAMESPACE

#endif   // OPTICALHELPER_H
