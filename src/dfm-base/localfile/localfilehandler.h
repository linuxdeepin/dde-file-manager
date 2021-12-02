/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef LOCALFILEHANDLER_H
#define LOCALFILEHANDLER_H

#include "dfm-base/dfm_base_global.h"

#include <QString>
#include <QFileDevice>

class QUrl;
class QString;
DFMBASE_BEGIN_NAMESPACE
class LocalFileHandler
{
public:
    LocalFileHandler();
    virtual ~LocalFileHandler();
    virtual bool touchFile(const QUrl &url);
    virtual bool mkdir(const QUrl &dir);
    virtual bool rmdir(const QUrl &url);
    virtual bool renameFile(const QUrl &url, const QUrl &newUrl);
    virtual bool openFile(const QUrl &file);
    virtual bool openFiles(const QList<QUrl> &files);
    virtual bool openFileByApp(const QUrl &file, const QString &appDesktop);
    virtual bool openFilesByApp(const QList<QUrl> &files, const QString &appDesktop);
    virtual bool createSystemLink(const QUrl &sourcfile, const QUrl &link);
    virtual bool setPermissions(const QUrl &url, QFileDevice::Permissions permissions);
    virtual bool deleteFile(const QUrl &file);
    virtual bool setFileTime(const QUrl &url, const QDateTime &accessDateTime,
                             const QDateTime &lastModifiedTime);
    QString errorString();

private:
    void setError(const QString &error);
    QString *errorStr = nullptr;   // 错误信息
};

DFMBASE_END_NAMESPACE

#endif   // LOCALFILEHANDLER_H
