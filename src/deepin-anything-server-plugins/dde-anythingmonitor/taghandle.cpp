// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
