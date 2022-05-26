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
#include "shareutils.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/base/device/deviceproxymanager.h"

#include <QDebug>

DPDIRSHARE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

bool ShareUtils::canShare(AbstractFileInfoPointer info)
{
    if (!info || !info->isDir() || !info->isReadable())
        return false;

    // in v20, this part controls whether to disable share action.
    if (info->ownerId() != SysInfoUtils::getUserId() && !SysInfoUtils::isRootUser())
        return false;

    if (DevProxyMng->isFileOfProtocolMounts(info->filePath()))
        return false;

    if (info->url().scheme() == Global::kBurn || DevProxyMng->isFileFromOptical(info->filePath()))
        return false;

    return true;
}
