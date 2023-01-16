/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "fileutil.h"

#include <base/schemefactory.h>
#include <file/local/desktopfileinfo.h>

#include <QReadWriteLock>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

class DesktopFileCreatorGlogal : public DesktopFileCreator
{
};
Q_GLOBAL_STATIC(DesktopFileCreatorGlogal, desktopFileCreatorGlogal)

DesktopFileCreator *DesktopFileCreator::instance()
{
    return desktopFileCreatorGlogal;
}

DFMLocalFileInfoPointer DesktopFileCreator::createFileInfo(const QUrl &url, bool cache)
{
    QString errString;
    auto itemInfo = InfoFactory::create<LocalFileInfo>(url, cache, &errString);
    if (Q_UNLIKELY(!itemInfo)) {
        qInfo() << "create LocalFileInfo error: " << errString << url;
        return nullptr;
    }

    return itemInfo;
}

DesktopFileCreator::DesktopFileCreator()
{
}
