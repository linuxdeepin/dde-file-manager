/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
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


#include "taghandle.h"

#include <tag/tagmanager.h>
#include <shutil/danythingmonitorfilter.h>

TagHandle::TagHandle(QObject *const parent)
: DASInterface{ parent } 
{

}

void TagHandle::onFileCreate(const QByteArrayList &files)
{
    (void)files;
}



void TagHandle::onFileDelete(const QByteArrayList &files)
{
    if (!files.isEmpty()) {
        QList<DUrl> url_list{};

        for (const QByteArray &byte_array : files) {
            bool result{ DAnythingMonitorFilter::instance()->whetherFilterCurrentPath(byte_array) };

            if (result) {
                DUrl url{ DUrl::fromLocalFile(byte_array) };
                url_list.push_back(url);
            }

        }

        TagManager::deleteFiles(url_list);
    }
}


void TagHandle::onFileRename(const QList<QPair<QByteArray, QByteArray>> &files)
{
    if (!files.isEmpty()) {

        for (const QPair<QByteArray, QByteArray> &names : files) {
            bool result{ DAnythingMonitorFilter::instance()->whetherFilterCurrentPath(names.second) };

            if (result) {
                TagManager::changeFilesName({ names });
            }
        }
    }
}
