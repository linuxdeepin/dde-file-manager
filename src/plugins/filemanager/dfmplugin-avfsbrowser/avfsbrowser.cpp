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
#include "avfsbrowser.h"
#include "files/avfsfileinfo.h"
#include "files/avfsfilewatcher.h"
#include "files/avfsfileiterator.h"
#include "utils/avfsutils.h"
#include "menu/avfsmenuscene.h"

#include "services/common/menu/menuservice.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"

DPAVFSBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void AvfsBrowser::initialize()
{
    InfoFactory::regClass<AvfsFileInfo>(AvfsUtils::scheme());
    WatcherFactory::regClass<AvfsFileWatcher>(AvfsUtils::scheme());
    DirIteratorFactory::regClass<AvfsFileIterator>(AvfsUtils::scheme());

    DSC_USE_NAMESPACE
    MenuService::service()->registerScene(AvfsMenuSceneCreator::name(), new AvfsMenuSceneCreator);
}

bool AvfsBrowser::start()
{
    return true;
}

dpf::Plugin::ShutdownFlag AvfsBrowser::stop()
{
    return kSync;
}

void AvfsBrowser::followEvents()
{
}
