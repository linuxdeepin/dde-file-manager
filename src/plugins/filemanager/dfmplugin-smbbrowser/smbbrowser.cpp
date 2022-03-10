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

#include "smbbrowser.h"
#include "utils/smbbrowserutils.h"
#include "fileinfo/smbsharefileinfo.h"
#include "iterator/smbshareiterator.h"
#include "menu/smbsharefilemenu.h"

#include "services/filemanager/workspace/workspaceservice.h"
#include "services/common/menu/menuservice.h"
#include "services/common/fileoperations/fileoperationsservice.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void SmbBrowser::initialize()
{
    UrlRoute::regScheme(SmbBrowserUtils::scheme(), "/", SmbBrowserUtils::icon(), true);

    InfoFactory::regClass<SmbShareFileInfo>(SmbBrowserUtils::scheme());
    DirIteratorFactory::regClass<SmbShareIterator>(SmbBrowserUtils::scheme());
    DSC_NAMESPACE::MenuService::regClass<SmbShareFileMenu>(SmbBrowserScene::kSmbBrowserScene);
}

bool SmbBrowser::start()
{
    DSB_FM_USE_NAMESPACE
    WorkspaceService::service()->addScheme(SmbBrowserUtils::scheme());
    WorkspaceService::service()->setWorkspaceMenuScene(SmbBrowserUtils::scheme(), SmbBrowserScene::kSmbBrowserScene);
    initOperations();
    return true;
}

dpf::Plugin::ShutdownFlag SmbBrowser::stop()
{
    return dpf::Plugin::ShutdownFlag::kSync;
}

void SmbBrowser::onWindowCreated(quint64 winId)
{
}

void SmbBrowser::onWindowOpened(quint64 winId)
{
}

void SmbBrowser::onWindowClosed(quint64 winId)
{
}

void SmbBrowser::initOperations()
{
    DSC_USE_NAMESPACE
    FileOperationsFunctions funcs(new FileOperationsSpace::FileOperationsInfo);
    funcs->openFiles = &SmbBrowserUtils::mountSmb;
    FileOperationsService::service()->registerOperations(SmbBrowserUtils::scheme(), funcs);
}

QDebug operator<<(QDebug dbg, const DPSMBBROWSER_NAMESPACE::SmbShareNode &node)
{
    dbg.nospace() << "SmbShareNode: {"
                  << "\n\turl: " << node.url
                  << "\n\tname: " << node.displayName
                  << "\n\ticon: " << node.iconType
                  << "\n}";
    return dbg;
}
