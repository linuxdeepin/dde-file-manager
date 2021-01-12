/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include <QObject>
#include <QtDBus>
#include "dbusservice/dbustype/dbusinforet.h"


class FileOperationAdaptor;

class FileOperation : public QObject ,protected QDBusContext
{
    Q_OBJECT

public:
    explicit FileOperation(const QString& servicePath, QObject *parent = nullptr);
    ~FileOperation();

    static QString ObjectPath;

signals:

public slots:
    DBusInfoRet NewCreateFolderJob(const QString &fabspath);
    DBusInfoRet NewCreateTemplateFileJob(const QString &templateFile, const QString &targetDir);
    DBusInfoRet NewCopyJob(const QStringList &filelist, const QString &targetDir);
    DBusInfoRet NewMoveJob(const QStringList &filelist, const QString &targetDir);
    DBusInfoRet NewRenameJob(const QString &oldFile, const QString &newFile);
    DBusInfoRet NewDeleteJob(const QStringList &filelist);
    QString test(const QString &oldFile, const QString &newFile, QDBusObjectPath &result2, bool &result3);

private:
    QString m_servicePath;
    FileOperationAdaptor* m_fileOperationAdaptor;
};

typedef QMap<QString, QString> StringMap;

Q_DECLARE_METATYPE(StringMap)

#endif // FILEOPERATION_H
