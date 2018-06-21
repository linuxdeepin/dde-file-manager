/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     shihua <tangtong@deepin.com>
 *
 * Maintainer: shihua <tangtong@deepin.com>
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



void TagHandle::onFileCreate(const QByteArrayList &files)
{
    (void)files;
}



void TagHandle::onFileDelete(const QByteArrayList &files)
{
    if (!files.isEmpty()) {
        QList<DUrl> url_list{};

        for (const QByteArray &byte_array : files) {
            DUrl url{ DUrl::fromLocalFile(byte_array) };
            bool result{ DAnythingMonitorFilter::instance()->whetherFilterCurrentPath(url) };

            if (result) {
                url_list.push_back(url);
            }

        }

        TagManager::instance()->deleteFiles(url_list);
    }
}


void TagHandle::onFileRename(const QList<QPair<QByteArray, QByteArray>> &files)
{
    if (!files.isEmpty()) {
        QList<QPair<DUrl, DUrl>> old_and_new_name{};

        for (const QPair<QByteArray, QByteArray> &name : files) {
            DUrl old{ DUrl::fromLocalFile(name.first) };
            bool result{ DAnythingMonitorFilter::instance()->whetherFilterCurrentPath(old) };

            if (result) {
                DUrl new_name{ DUrl::fromLocalFile(name.second) };
                old_and_new_name.push_back(QPair<DUrl, DUrl> {old, new_name});
                TagManager::instance()->changeFilesName(old_and_new_name);
            }
        }
    }
}
