/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#include "dfm-base/utils/finallyutil.h"

#include <QFileInfo>
#include <QTimer>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QApplication>

DFMBASE_BEGIN_NAMESPACE

/*!
 * \class FileUtils
 *
 * \brief Utility class providing static helper methods for file management
 */

bool FileUtils::mkdir(const QUrl &url, const QString &dirName, QString *errorString)
{
    QString error;
    FinallyUtil finally([&]() {if (errorString) *errorString = error; });
    if (!url.isValid()) {
        error = "Failed, can't use empty url from create dir";
        return false;
    }

    if (UrlRoute::isVirtual(url)) {
        error = "Failed, can't use virtual url from create dir";
        return false;
    }

    QFileInfo info(UrlRoute::urlToPath(url));
    if (!info.exists()) {
        error = "Failed, url not exists";
        return false;
    }

    if (!info.isDir()) {
        error = "Failed, url pat not is dir";
        return false;
    }

    if (!info.permissions().testFlag(QFile::Permission::WriteUser)) {
        error = "Failed, not permission create dir";
        return false;
    }

    auto localDirPath = info.filePath() + "/" + dirName;
    if (QFileInfo(localDirPath).exists()) {
        error = QString("Failed, current dir is exists \n %0").arg(localDirPath);
        return false;
    }

    if (!QDir().mkdir(localDirPath)) {
        error = "Failed, unknown error from create new dir";
        return false;
    }

    finally.dismiss();
    return true;
}

bool FileUtils::touch(const QUrl &url,
                      const QString &fileName,
                      QString *errorString)
{
    QString error;
    FinallyUtil finally([&]() {if (errorString) *errorString = error; });

    if (!url.isValid()) {
        error = "Failed, can't use empty url from create dir";
        return false;
    }

    if (UrlRoute::isVirtual(url)) {
        error = "Failed, can't use virtual url from create dir";
        return false;
    }

    if (!UrlRoute::isVirtual(url)) {
        QFileInfo info(UrlRoute::urlToPath(url));
        if (!info.isDir() || !info.exists()) {
            error = "Failed, Fileinfo error";
            return false;
        }
        if (!info.permissions().testFlag(QFile::Permission::WriteUser)) {
            error = "Failed, Fileinfo permission error";
            return false;
        }
        QFile file(info.path() + "/" + fileName);
        if (!file.open(QIODevice::Truncate)) {
            error = "Failed, Create new dir unknown error";
            return false;
        }
    }

    finally.dismiss();
    return true;
}

QString sizeString(const QString &str)
{
    int beginPos = str.indexOf('.');

    if (beginPos < 0)
        return str;

    QString size = str;

    while (size.count() - 1 > beginPos) {
        if (!size.endsWith('0'))
            return size;

        size = size.left(size.count() - 1);
    }

    return size.left(size.count() - 1);
}

QString FileUtils::formatSize(qint64 num, bool withUnitVisible, int precision, int forceUnit, QStringList unitList)
{
    if (num < 0) {
        qWarning() << "Negative number passed to formatSize():" << num;
        num = 0;
    }

    bool isForceUnit = (forceUnit >= 0);
    QStringList list;
    qreal fileSize(num);

    if (unitList.size() == 0) {
        list << " B"
             << " KB"
             << " MB"
             << " GB"
             << " TB";   // should we use KiB since we use 1024 here?
    } else {
        list = unitList;
    }

    QStringListIterator i(list);
    QString unit = i.hasNext() ? i.next() : QStringLiteral(" B");

    int index = 0;
    while (i.hasNext()) {
        if (fileSize < 1024 && !isForceUnit) {
            break;
        }

        if (isForceUnit && index == forceUnit) {
            break;
        }

        unit = i.next();
        fileSize /= 1024;
        index++;
    }
    QString unitString = withUnitVisible ? unit : QString();
    return QString("%1%2").arg(sizeString(QString::number(fileSize, 'f', precision)), unitString);
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

int FileUtils::supportedMaxLength(const QString &fileSystem)
{
    static QMap<QString, int> datas {
        std::pair<QString, int>("VFAT", 11),
        std::pair<QString, int>("EXFAT", 11),
        std::pair<QString, int>("EXT2", 16),
        std::pair<QString, int>("EXT3", 16),
        std::pair<QString, int>("EXT4", 16),
        std::pair<QString, int>("NTFS", 40),   // can be more than 40
    };

    const int DefaultLength = 11;
    return datas.value(fileSystem.toUpper(), DefaultLength);
}

DFMBASE_END_NAMESPACE
