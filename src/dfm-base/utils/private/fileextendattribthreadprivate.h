/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef FILEEXTENDATTRIBTHREADPRIVATE_H
#define FILEEXTENDATTRIBTHREADPRIVATE_H

#include "dfm-base/dfm_base_global.h"

#include <QUrl>
#include <QList>
#include <QQueue>

namespace dfmbase {
class FileExtendAttribThread;
class FileExtendAttribThreadPrivate
{
    friend class FileExtendAttribThread;
    Q_DISABLE_COPY(FileExtendAttribThreadPrivate)
private:
    explicit FileExtendAttribThreadPrivate(FileExtendAttribThread *extendAttribThread = nullptr);

    virtual ~FileExtendAttribThreadPrivate();

private:
    FileExtendAttribThread *fileExtendAttibThread { nullptr };
    QList<QUrl> urlList {};
    qint64 totalSize { 0 };
    QQueue<QUrl> dirList {};
    int state { 0 };
};
}
#endif   //FILEEXTENDATTRIBTHREADPRIVATE_H
