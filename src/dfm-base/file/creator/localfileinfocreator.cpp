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
#include "localfileinfocreator.h"
#include "file/local/desktopfileinfo.h"
#include "base/schemefactory.h"


DFMBASE_USE_NAMESPACE

LocalFileInfoCreator::LocalFileInfoCreator() : AbstractFileInfoCreator()
{

}

bool LocalFileInfoCreator::registerConvertor(AbstractFileInfoCreator::Convertor func, void *extData)
{
    QWriteLocker lk(&funcLocker);
    auto iter = std::find_if(convertFuncs.begin(), convertFuncs.end(), [func](const QPair<Convertor, void *> &cur){
        return cur.first == func;
    });

    if (iter != convertFuncs.end())
        return false;

    convertFuncs.append(qMakePair(func, extData));
    return true;
}

void LocalFileInfoCreator::unregisterConvertor(AbstractFileInfoCreator::Convertor func)
{
    QWriteLocker lk(&funcLocker);
    for (auto iter = convertFuncs.begin(); iter != convertFuncs.end(); ++iter) {
        // remove first func found.
        if (iter->first == func) {
            convertFuncs.erase(iter);
            return;
        }
    }
}

AbstractFileInfoPointer LocalFileInfoCreator::create(const QUrl &url, bool cache, QString *error)
{
    return qSharedPointerDynamicCast<AbstractFileInfo>(createLocalFileInfo(url, cache, error));
}

DFMLocalFileInfoPointer LocalFileInfoCreator::createLocalFileInfo(const QUrl &url, bool cache, QString *error)
{
    auto itemInfo = dfmbase::InfoFactory::create<LocalFileInfo>(url, cache, error);
    if (Q_UNLIKELY(!itemInfo))
        return nullptr;

    QReadLocker lk(&funcLocker);
    for (auto iter = convertFuncs.begin(); iter != convertFuncs.end(); ++iter) {
        auto file = iter->first(itemInfo, iter->second);
        if (auto local = qSharedPointerDynamicCast<LocalFileInfo>(file))
            itemInfo = local;
    }

    return itemInfo;
}

AbstractFileInfoPointer LocalFileInfoCreator::tryCovertDesktopFileInfo(const AbstractFileInfoPointer &info, void *)
{
    if (info->suffix() == "desktop" && info->mimeTypeName() == "application/x-desktop") {
        return DFMLocalFileInfoPointer(new DesktopFileInfo(info->url())); // todo(lxs) 处理watcher问题
    }

    return nullptr;
}
