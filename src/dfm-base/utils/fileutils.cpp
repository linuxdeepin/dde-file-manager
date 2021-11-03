/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "utils/fileutils.h"
#include "mimetype/mimedatabase.h"
#include "dfm-base/base/urlroute.h"

#include <QFileInfo>
#include <QTimer>
#include <QDir>
#include <QProcess>
#include <QDebug>

DFMBASE_BEGIN_NAMESPACE

bool FileUtils::mkdir(const QUrl &url, const QString dirName, QString *errorString)
{
    if (!url.isValid()) {
        if (errorString) {
            *errorString = QObject::tr("Failed, can't use empty url from create dir");
        }
        qInfo() << QObject::tr("Failed, can't use empty url from create dir");
        return false;
    }

    if (UrlRoute::isVirtual(url)) {
        if (errorString) {
            *errorString = QObject::tr("Failed, can't use virtual url from create dir");
        }
        qInfo() << QObject::tr("Failed, can't use virtual url from create dir");
        return false;
    }

    QFileInfo info(UrlRoute::urlToPath(url));
    if (!info.exists()) {
        if (errorString) {
            *errorString = QObject::tr("Failed, url not exists");
        }
        qInfo() << QObject::tr("Failed, url not exists");
        return false;
    }

    if (!info.isDir()) {
        if (errorString) {
            *errorString = QObject::tr("Failed, url pat not is dir");
        }
        qInfo() << QObject::tr("Failed, url pat not is dir");
        return false;
    }

    if (!info.permissions().testFlag(QFile::Permission::WriteUser)) {
        if (errorString) {
            *errorString = QObject::tr("Failed, not permission create dir");
        }
        qInfo() << QObject::tr("Failed, not permission create dir");
        return false;
    }

    auto localDirPath = info.filePath() + "/" + dirName;
    if (QFileInfo(localDirPath).exists()) {
        if (errorString) {
            *errorString = QObject::tr("Failed, current dir is exists \n %0").arg(localDirPath);
        }
        qInfo() << QObject::tr("Failed, current dir is exists \n %0").arg(localDirPath);
        return false;
    }

    if (!QDir().mkdir(localDirPath)) {
        if (errorString) {
            *errorString = QObject::tr("Failed, unknown error from create new dir");
        }
        qInfo() << QObject::tr("Failed, unknown error from create new dir");
        abort();
    }
    return true;
}

bool FileUtils::touch(const QUrl &url,
                      const QString fileName,
                      QString *errorString)
{
    if (!url.isValid()) {
        if (errorString) {
            *errorString = QObject::tr("Failed, can't use empty url from create dir");
        }
        qInfo() << QObject::tr("Failed, can't use empty url from create dir");
        return false;
    }

    if (UrlRoute::isVirtual(url)) {
        if (errorString) {
            *errorString = QObject::tr("Failed, can't use virtual url from create dir");
        }
        qInfo() << QObject::tr("Failed, can't use virtual url from create dir");
        return false;
    }

    if (!UrlRoute::isVirtual(url)) {
        QFileInfo info(UrlRoute::urlToPath(url));
        if (!info.isDir() || !info.exists())
            return false;
        if (!info.permissions().testFlag(QFile::Permission::WriteUser))
            return false;
        QFile file(info.path() + "/" + fileName);
        if (!file.open(QIODevice::Truncate)) {
            qInfo() << "create new dir unknown error";
            abort();
        }
    }
    return true;
}

QMap<QString, QString> FileUtils::getKernelParameters()
{
    QFile cmdline("/proc/cmdline");
    cmdline.open(QIODevice::ReadOnly);
    QByteArray content = cmdline.readAll();

    QByteArrayList paraList(content.split(' '));

    QMap<QString, QString> result;
    result.insert("_ori_proc_cmdline", content);

    for (const QByteArray &onePara : paraList) {
        int equalsIdx = onePara.indexOf('=');
        QString key = equalsIdx == -1 ? onePara.trimmed() : onePara.left(equalsIdx).trimmed();
        QString value = equalsIdx == -1 ? QString() : onePara.right(equalsIdx).trimmed();
        result.insert(key, value);
    }

    return result;
}

DFMBASE_END_NAMESPACE
