/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "computer.h"
#include "utils/computerutils.h"
#include "views/computerview.h"
#include "fileentity/appentryfileentity.h"

#include "services/filemanager/windows/windowsservice.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/entry/entities/entryentities.h"
#include "dfm-base/file/entry/entryfileinfo.h"

#include <sidebar/sidebarservice.h>
#include <computer/computerservice.h>
#include <base/urlroute.h>

DSB_FM_USE_NAMESPACE

DPCOMPUTER_BEGIN_NAMESPACE
/*!
 * \class Computer
 * \brief the plugin initializer
 */
void Computer::initialize()
{
    DFMBASE_USE_NAMESPACE
    DPCOMPUTER_USE_NAMESPACE
    UrlRoute::regScheme(ComputerUtils::scheme(), "/", ComputerUtils::icon(), true);
    ViewFactory::regClass<ComputerView>(ComputerUtils::scheme());

    EntryEntityFactor::registCreator<UserEntryFileEntity>(SuffixInfo::kUserDir);
    EntryEntityFactor::registCreator<BlockEntryFileEntity>(SuffixInfo::kBlock);
    EntryEntityFactor::registCreator<ProtocolEntryFileEntity>(SuffixInfo::kProtocol);
    EntryEntityFactor::registCreator<StashedProtocolEntryFileEntity>(SuffixInfo::kStashedRemote);
    EntryEntityFactor::registCreator<AppEntryFileEntity>(SuffixInfo::kAppEntry);
}

bool Computer::start()
{
    return true;
}

dpf::Plugin::ShutdownFlag Computer::stop()
{
    return kSync;
}
DPCOMPUTER_END_NAMESPACE
