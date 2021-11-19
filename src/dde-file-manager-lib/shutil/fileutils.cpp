/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

//fixed:CD display size error
#include "gvfs/gvfsmountmanager.h"

#include "fileutils.h"

#include "views/windowmanager.h"

#include "app/define.h"
#include "singleton.h"
#include "mimetypedisplaymanager.h"
#include "dfmstandardpaths.h"
#include "dfileservices.h"
#include "dmimedatabase.h"
#include "mimesappsmanager.h"
#include "interfaces/dfmstandardpaths.h"
#include "controllers/appcontroller.h"
#include "dbusinterface/startmanager_interface.h"
#include "views/dfmopticalmediawidget.h"
#include "controllers/vaultcontroller.h"
#include "models/avfsfileinfo.h"
#include "dfmapplication.h"
#include "dfmsettings.h"
#include "usershare/usersharemanager.h"
#include "usershare/shareinfo.h"

#include <dstorageinfo.h>
#include <dgiovolumemanager.h>
#include <dgiomount.h>
#include <dgiofile.h>
#include <dgiovolume.h>

#include <QDirIterator>
#include <QUrl>
#include <QProcess>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QDebug>
#include <QDesktopServices>
#include <QtMath>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QX11Info>
#include <dabstractfilewatcher.h>
#include <QApplication>
#include <QScreen>
#include <QNetworkInterface>
#include <DRecentManager>

#include <sys/vfs.h>
#include <sys/stat.h>
#include <fts.h>
#include <unistd.h>
#include <sys/utsname.h>

#undef signals
extern "C" {
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
}
#define signals public

DFM_USE_NAMESPACE
DCORE_USE_NAMESPACE

QString FileUtils::XDG_RUNTIME_DIR = "";
QStringList FileUtils::CURRENT_ISGVFSFILE_PATH;

/**
 * @brief Recursive removes file or directory
 * @param path path to file
 * @param name name of file
 * @return true if file/directory was successfully removed
 */
bool FileUtils::removeRecurse(const QString &path, const QString &name)
{
    // File location
    QString url = path + QDir::separator() + name;

    // Check whether file or directory exists
    QFileInfo file(url);
    if (!file.exists()) {
        return false;
    }

    // List of files that will be deleted
    QStringList files;

    // If given file is a directory, collect all children of given directory
    if (file.isDir()) {
        QDirIterator it(url, QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot | QDir::Hidden, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            files.prepend(it.next());
        }
    }

    // Append given file to the list of files and delete all
    files.append(url);
    foreach (QString file, files) {
        QFile(file).remove();
    }
    return true;
}
//---------------------------------------------------------------------------

/**
 * @brief Check ancestor
 * @param
 * @param
 * @param
 */
bool FileUtils::isAncestorUrl(const DUrl &ancestor, const DUrl &url)
{
    DUrl parent = url;
    while (parent.isValid()) {
        if (parent == ancestor) {
            return true;
        }
        parent = parent.parentUrl();
    }
    return false;
}
/**
 * @brief Check isNetworkAncestorUrl 检查挂载的目录是否是本机目录，是本地挂载转换为本地url，在判读是否是同一个目录
 * 两个参数一个是本地，一个网络挂载文件，必须是目标文件是原文的父文件，就返回true
 * @param ancestor
 * @param
 * @param
 */
bool FileUtils::isNetworkAncestorUrl(const DUrl &dest, const bool isDestGvfs, const DUrl &source, const bool isSourceGvfs)
{
    if ((isDestGvfs && isSourceGvfs) || (!isDestGvfs && !isSourceGvfs))
        return false;

    QString gvfsUrlStr = isDestGvfs ? dest.path() : source.path();

    static QRegularExpression regExp("^/run/user/\\d+/gvfs/smb-share:server=(?<host>.*),share=(?<shareName>[^/]+)(?<path>.*)",
                                     QRegularExpression::DotMatchesEverythingOption
                                     | QRegularExpression::DontCaptureOption
                                     | QRegularExpression::OptimizeOnFirstUsageOption);

    const QRegularExpressionMatch &match = regExp.match(gvfsUrlStr, 0, QRegularExpression::NormalMatch,
                                                        QRegularExpression::DontCheckSubjectStringMatchOption);

    if (!match.hasMatch())
        return false;

    const QString &host = match.captured("host");
    const QString &shareName = match.captured("shareName");
    const QString &path = match.captured("path");
    // 获取本机ip判断是否是自己
    bool selfIp = false;
    const auto &allAddresses = QNetworkInterface::allAddresses();
    for (const auto &address : allAddresses) {
        const QString &ipAddr = address.toString();
        if (ipAddr == host) {
            selfIp = true;
            break;
        }
    }

    if (!selfIp)
        return false;
    //获取自己的共享路径
    QString realPath;
    if (!userShareManager->getsShareInfoByShareName(shareName).isValid())
        return false;

    realPath = userShareManager->getsShareInfoByShareName(shareName).path() + path;
    DUrl realUrl = DUrl::fromLocalFile(realPath);
    if (isDestGvfs && realUrl.parentUrl().path().contains(source.path()))
        return true;

    if (isSourceGvfs && dest.path().contains(realUrl.parentUrl().path()))
        return true;

    return false;
}

/**
 * @brief Collects all file names in given path (recursive)
 * @param path path
 * @param parent parent path
 * @param list resulting list of files
 */
void FileUtils::recurseFolder(const QString &path, const QString &parent,
                              QStringList *list)
{
    // Get all files in this path
    QDir dir(path);
    //删出都是系统的链接文件时，这里统计不到，使用QDir::System标志
    QStringList files = dir.entryList(QDir::AllEntries | QDir::System
                                      | QDir::NoDotAndDotDot | QDir::Hidden);

    // Go through all files in current directory
    for (int i = 0; i < files.count(); i++) {
        // If current file is folder perform this method again. Otherwise add file
        // to list of results
        QString current = parent + QDir::separator() + files.at(i);
        QString next = path + QDir::separator() + files.at(i);
        list->append(current);

        // fix bug#52386 【x86】【robot】【文件管理器】删除~/.deepinwine目录下的文件夹导致系统内存泄露
        if (QFileInfo(next).isDir() && !QFileInfo(next).isSymLink()) {
            recurseFolder(next, current, list);
        }
    }
}

int FileUtils::filesCount(const QString &dir)
{
    static QMutex mutex;
    QMutexLocker lk(&mutex);
    QDir d(dir);
    QStringList entryList = d.entryList(QDir::AllEntries | QDir::System
                                        | QDir::NoDotAndDotDot | QDir::Hidden);
    return entryList.size();
}

QStringList FileUtils::filesList(const QString &dir)
{
    QStringList appNames;
    QDirIterator it(dir,
                    QDir::Files | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        appNames.append(it.filePath());
    }
    return appNames;
}

/**
 * @brief 获取一个目录文件的大小
 *
 * 通常情况下，一个空目录文件的大小是 4k （有文件的目录的大小随文件数量的增加而增加）
 * 但是不同的文件系统下，目录的大小是不同的 （如 exfat 是 32k，vfat 是 8k）
 * 跑CI时发现, 空文件夹size返回了64B
 *
 * @return fts_* 函数获取的结果，默认为 4096
 */
qint64 FileUtils::singleDirSize(const DUrl &url)
{
    qint64 size = 0;
    char *paths[2] = {nullptr, nullptr};
    QByteArray urlBytes = url.path().toUtf8();
    paths[0] = strdup(urlBytes.data());
    FTS *fts = fts_open(paths, 0, nullptr);

    if (fts) {
        FTSENT *ent = fts_read(fts);
        if (ent && ent->fts_info == FTS_D)
            size = ent->fts_statp->st_size <= 0 ? 4096 : ent->fts_statp->st_size;
        fts_close(fts);
    }

    if (paths[0])
        free(paths[0]);
    return size;
}

qint64 FileUtils::totalSize(const QString &targetFile)
{
    qint64 total = 0;
    QFileInfo targetInfo(targetFile);
    if (targetInfo.exists()) {
        if (targetInfo.isDir()) {
            QDir d(targetFile);
            QFileInfoList entryInfoList = d.entryInfoList(QDir::AllEntries | QDir::System
                                                          | QDir::NoDotAndDotDot | QDir::NoSymLinks
                                                          | QDir::Hidden);
            foreach (QFileInfo file, entryInfoList) {
                if (file.isFile()) {
                    total += file.size();
                } else {
                    QDirIterator it(file.absoluteFilePath(), QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden, QDirIterator::Subdirectories);
                    while (it.hasNext()) {
                        it.next();
                        total += it.fileInfo().size();
                    }
                }
            }
        } else {
            total += targetInfo.size();
        }
    }
    return total;
}
//---------------------------------------------------------------------------

/**
 * @brief Returns size of all given files/dirs (including nested files/dirs)
 * @param files
 * @return total size
 */
qint64 FileUtils::totalSize(const DUrlList &files)
{
    qint64 total = 1;
    foreach (QUrl url, files) {
        QFileInfo file = url.path();
        if (file.isFile())
            total += file.size();
        else if (!file.isSymLink()) {
            QDirIterator it(url.path(), QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                total += it.fileInfo().size();
            }
        }
    }
    return total;
}
/**
 * @brief Returns size of all given files/dirs (including local) by ftd,so fast in local
 * @param files
 * @param dirSize outputparam,dir size.
 * @param fileCount outputparam,all file count
 * @return total size
 */
qint64 FileUtils::totalSize(const DUrlList &files, qint32 &dirSize, qint32 &fileCount)
{
    qint64 total = 0;
    dirSize = 0;
    fileCount = 0;
    for (auto url : files) {
        char *paths[2] = {nullptr, nullptr};
        paths[0] = strdup(url.path().toUtf8().toStdString().data());
        FTS *fts = fts_open(paths, 0, nullptr);
        if (paths[0])
            free(paths[0]);
        if (nullptr == fts) {
            perror("fts_open");
            continue;
        }
        while (1) {
            FTSENT *ent = fts_read(fts);
            if (ent == nullptr) {
                break;
            }
            unsigned short flag = ent->fts_info;
            if (flag != FTS_DP)
                total += ent->fts_statp->st_size <= 0 ? getMemoryPageSize() : ent->fts_statp->st_size;
            if (dirSize == 0 && flag == FTS_D)
                dirSize = ent->fts_statp->st_size <= 0 ? getMemoryPageSize() :
                                                                  static_cast<qint32>(ent->fts_statp->st_size);
            if (flag == FTS_F)
                fileCount++;
        }
        fts_close(fts);
    }

    dirSize = dirSize <= 0 ? getMemoryPageSize() : dirSize;
    return total;
}

qint64 FileUtils::totalSize(const DUrlList &files, const qint64 &maxLimit, bool &isInLimit)
{
    qint64 total = 1;
    foreach (QUrl url, files) {
        QFileInfo file = url.path();
        if (file.isFile())
            total += file.size();
        if (total > maxLimit) {
            isInLimit = false;
            return total;
        } else {
            QDirIterator it(url.path(), QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                total += it.fileInfo().size();
                if (total > maxLimit) {
                    isInLimit = false;
                    return total;
                }
            }
        }
    }
    return total;
}

bool FileUtils::isArchive(const QString &path)
{
    QFileInfo f(path);
    if (f.exists()) {
        return mimeTypeDisplayManager->supportArchiveMimetypes().contains(DMimeDatabase().mimeTypeForFile(f).name());
    } else {
        return false;
    }
}

//---------------------------------------------------------------------------

/**
 * @brief Returns names of available applications
 * @return application name list
 */
QStringList FileUtils::getApplicationNames()
{
    QStringList appNames;
    QDirIterator it("/usr/share/applications", QStringList("*.desktop"),
                    QDir::Files | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        appNames.append(it.fileName());
    }
    return appNames;
}

//---------------------------------------------------------------------------

/**
 * @brief Returns real suffix for given file
 * @param name
 * @return suffix
 */
QString FileUtils::getRealSuffix(const QString &name)
{
    // Strip version suffix
    QStringList tmp = name.split(".");
    bool ok;
    while (tmp.size() > 1) {
        tmp.last().toInt(&ok);
        if (!ok) {
            return tmp.last();
        }
        tmp.removeLast();
    }
    return "";
}
//---------------------------------------------------------------------------

/**
 * @brief Returns mime icon
 * @param mime
 * @return icon
 */
QIcon FileUtils::searchMimeIcon(QString mime, const QIcon &defaultIcon)
{
    QIcon icon = QIcon::fromTheme(mime.replace("/", "-"), defaultIcon);
    return icon;
}
//---------------------------------------------------------------------------

/**
 * @brief Searches for generic icon
 * @param category
 * @return icon
 */
QIcon FileUtils::searchGenericIcon(const QString &category,
                                   const QIcon &defaultIcon)
{
    QIcon icon = QIcon::fromTheme(category + "-generic");
    if (!icon.isNull()) {
        return icon;
    }
    icon = QIcon::fromTheme(category + "-x-generic");
    return icon.isNull() ? defaultIcon : icon;
}
//---------------------------------------------------------------------------

/**
 * @brief Searches for application icon in the filesystem
 * @param app
 * @param defaultIcon
 * @return icon
 */
QIcon FileUtils::searchAppIcon(const DesktopFile &app,
                               const QIcon &defaultIcon)
{
    // Resulting icon
    QIcon icon;

    // First attempt, check whether icon is a valid file
    if (QFile(app.getIcon()).exists()) {
        icon = QIcon(app.getIcon());
        if (!icon.isNull()) {
            return icon;
        }
    }

    // Second attempt, try load icon from theme
    icon = QIcon::fromTheme(app.getIcon());
    if (!icon.isNull()) {
        return icon;
    }

    // Next, try luck with application name
    QString name = app.getFileName().remove(".desktop").split("/").last();
    icon = QIcon::fromTheme(name);
    if (!icon.isNull()) {
        return icon;
    }

    // Last chance
    QDir appIcons("/usr/share/pixmaps", "", nullptr, QDir::Files | QDir::NoDotAndDotDot);
    QStringList iconFiles = appIcons.entryList();
    QStringList searchIcons = iconFiles.filter(name);
    if (searchIcons.count() > 0) {
        return QIcon("/usr/share/pixmaps/" + searchIcons.at(0));
    }

    // Default icon
    return defaultIcon;
}
//---------------------------------------------------------------------------

QString sizeString(const QString &str)
{
    int begin_pos = str.indexOf('.');

    if (begin_pos < 0)
        return str;

    QString size = str;

    while (size.count() - 1 > begin_pos) {
        if (!size.endsWith('0'))
            return size;

        size = size.left(size.count() - 1);
    }

    return size.left(size.count() - 1);
}

qreal dRound64(qreal num, int count = 1)
{
    if (count <= 0)
        return qRound64(num);

    qreal base = qPow(10, count);

    return qRound64(num * base) / base;
}

/*!
 * \brief Display human readable file size.
 *
 * by default, will display size unit. like `1.2 GB` or `616 MB`. By using
 * \a forceUnit argument we can also force to display a unit size with the
 * required unit. 0 for bytes, 1 for KiB, 2 for MiB, 3 for GiB, etc.
 *
 * \param num The file size number.
 * \param withUnitVisible Size unit visible or not.
 * \param precision Precision for float number after the dot.
 * \param forceUnit Force to use a unit.
 * \return
 */
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
        list << " B" << " KB" << " MB" << " GB" << " TB"; // should we use KiB since we use 1024 here?
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

/*!
 * \brief 计算机页面磁盘大小数据
 * \param usedSize 使用引用的原因是光驱的数据可能重置，若不重置则会影响进度条异常
 * \param totalSize 使用引用的原因是光驱的数据可能重置，若不重置则会影响进度条异常
 * \param strVolTag 这个标识符加入的用于判定是否为光驱
 * \return
 */
QString FileUtils::diskUsageString(quint64 &usedSize, quint64 &totalSize, QString strVolTag)
{
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const QStringList unitDisplayText = {"B", "K", "M", "G", "T"};

    if (!~usedSize) {
        return FileUtils::formatSize(static_cast<qint64>(totalSize), true, 0, totalSize < mb ? 2 : -1, unitDisplayText);
    }

    // todo: too ugly! should be beaufify
    // fix: 探测光盘推进,弹出和挂载状态机标识
    bool bVolFlag = strVolTag.startsWith("sr") // 避免因传入非光驱挂载点而插入 CdStatusInfo 对象
                    ? DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].bVolFlag
                    : false;
    bool bMntFlag = strVolTag.startsWith("sr")
                    ? DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].bMntFlag
                    : false;
    if (bVolFlag && (totalSize == 0)) { //CD/DVD
        return QObject::tr("Unknown");
    } else if (!bVolFlag && !bMntFlag  && (totalSize == 0)) {
        if (strVolTag.startsWith("sr")) { //CD/DVD
            return FileUtils::defaultOpticalSize(strVolTag, usedSize, totalSize);
        } else { // special: totalsize == 0
            return QString("0M");
        }
    } else if (!bVolFlag && !bMntFlag  && (totalSize > 0) && (usedSize == 0)) { //blank CD/DVD
        return QString("0M");
    } else {
        return QString("%1/%2").arg(FileUtils::formatSize(static_cast<qint64>(usedSize), true, 1, usedSize < mb ? 2 : -1, unitDisplayText),
                                    FileUtils::formatSize(static_cast<qint64>(totalSize), true, 1, totalSize < mb ? 2 : -1, unitDisplayText));
    }
}


/*!
 * \brief 解析配置文件，获取光盘大小数据
 * 光盘打开后可以显示大小，并挂载然而关闭文管后再重新启动会导致之前的状态机失效（未持久化存储）
 * 因此会显示为0M，且状态为已挂载的状态，而右下角插件则因为读取了配置而显示正确的大小
 *
 * \return 挂载状态时返回配置文件中存储的光盘大小
 */
QString FileUtils::defaultOpticalSize(const QString &tagName, quint64 &usedSize, quint64 &totalSize)
{
    QString size{"0M"};
    int burnStatus{DFMOpticalMediaWidget::BCSA_BurnCapacityStatusEjct};
    quint64 curTotalSize{0}; // 光盘总大小
    quint64 curUsedSize{0};  // 光盘已使用的大小

    if (DFMApplication::genericSetting()->keys(BURN_CAPACITY_ATTRIBUTE).contains(tagName)) {
        const QMap<QString, QVariant> &info = DFMApplication::genericSetting()->value(BURN_CAPACITY_ATTRIBUTE, tagName).toMap();
        burnStatus = info.value(BURN_CAPACITY_STATUS).toInt();
        curTotalSize = static_cast<quint64>(info.value(BURN_CAPACITY_TOTAL_SIZE).toDouble());
        curUsedSize = static_cast<quint64>(info.value(BURN_CAPACITY_USED_SIZE).toDouble());

        if (burnStatus == DFMOpticalMediaWidget::BCSA_BurnCapacityStatusAddMount && curUsedSize > 0 && curTotalSize > 8) {
            const qint64 kb = 1024;
            const qint64 mb = 1024 * kb;
            const QStringList unitDisplayText = {"B", "K", "M", "G", "T"};
            // 重置原始的size数据
            usedSize = curUsedSize;
            totalSize = curTotalSize;
            size = QString("%1/%2").arg(FileUtils::formatSize(static_cast<qint64>(usedSize), true, 1, usedSize < mb ? 2 : -1, unitDisplayText),
                                        FileUtils::formatSize(static_cast<qint64>(totalSize), true, 1, totalSize < mb ? 2 : -1, unitDisplayText));

        }
    }

    return size;
}

DUrl FileUtils::newDocumentUrl(const DAbstractFileInfoPointer targetDirInfo, const QString &baseName, const QString &suffix)
{
    if (targetDirInfo->isVirtualEntry()) {
        return DUrl();
    }

    int i = 0;
    QString fileName = suffix.isEmpty() ? QString("%1").arg(baseName) : QString("%1.%2").arg(baseName, suffix);
    DUrl fileUrl = targetDirInfo->getUrlByChildFileName(fileName);
    while (true) {
        DAbstractFileInfoPointer newInfo = DFileService::instance()->createFileInfo(nullptr, fileUrl);
        if (newInfo && newInfo->exists()) {
            ++i;
            fileName = suffix.isEmpty()
                       ? QString("%1%2").arg(baseName, QString::number(i))
                       : QString("%1%2.%3").arg(baseName, QString::number(i), suffix);
            fileUrl = targetDirInfo->getUrlByChildFileName(fileName);
        } else {
            return fileUrl;
        }
    }
}

QString FileUtils::newDocmentName(QString targetdir, const QString &baseName, const QString &suffix)
{
    if (targetdir.isEmpty())
        return QString();

    if (targetdir.endsWith(QDir::separator()))
        targetdir.chop(1);

    int i = 0;
    QString filePath = suffix.isEmpty() ? QString("%1/%2").arg(targetdir, baseName) : QString("%1/%2.%3").arg(targetdir, baseName, suffix);
    while (true) {
        if (QFile(filePath).exists()) {
            ++i;
            filePath = suffix.isEmpty()
                       ? QString("%1/%2 %3").arg(targetdir, baseName, QString::number(i))
                       : QString("%1/%2 %3.%4").arg(targetdir, baseName, QString::number(i), suffix);
        } else {
            return filePath;
        }
    }
}

bool FileUtils::cpTemplateFileToTargetDir(const QString &targetdir, const QString &baseName, const QString &suffix, WId windowId)
{
    QString templateFile;
    QDirIterator it(DFMStandardPaths::location(DFMStandardPaths::TemplatesPath), QDir::Files);
    while (it.hasNext()) {
        it.next();
        if (it.fileInfo().suffix() == suffix) {
            templateFile = it.filePath();
            break;
        }
    }

    if (templateFile.isEmpty())
        return false;

//    QString targetFile = FileUtils::newDocmentName(targetdir, baseName, suffix);

//    if (targetFile.isEmpty())
//        return false;

//    return QFile::copy(templateFile, targetFile);
    return !AppController::createFile(templateFile, targetdir, baseName, windowId).isEmpty();
}

bool FileUtils::openFile(const QString &filePath)
{
    bool result = false;
    if (QFileInfo(filePath).suffix() == "desktop") {
        result = FileUtils::launchApp(filePath);
        return result;
    }

    /*********************************************************/
    //解决空文本文件转其他非文本格式时打开仍然是文本方式打开的问题
    //QString mimetype = getFileMimetype(filePath);
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, DUrl(FILE_ROOT + filePath));
    QString mimetype;
    if (info && info->size() == 0 && info->exists()) {
        mimetype = info->mimeType().name();
    } else {
        mimetype = getFileMimetype(filePath);
    }
    /*********************************************************/
    QAtomicInteger<bool> isOpenNow = false;
    QString defaultDesktopFile = MimesAppsManager::getDefaultAppDesktopFileByMimeType(mimetype);
    if (defaultDesktopFile.isEmpty()) {
        if (isSmbUnmountedFile(DUrl::fromLocalFile(filePath))) {
            mimetype = QString("inode/directory");
            defaultDesktopFile = MimesAppsManager::getDefaultAppDesktopFileByMimeType(mimetype);
            isOpenNow = true;
            mimetype = QString();
        } else {
            qDebug() << "no default application for" << filePath;
            return false;
        }
    }
    //此处会排除执行段(Exec=)包含dde-file-manager的desktop文件，需要对目录(inode/directory)类型及dde-open.desktop例外处理
    if (!isOpenNow && isFileManagerSelf(defaultDesktopFile) && mimetype != "inode/directory" && !defaultDesktopFile.contains("/dde-open.desktop")) {
        QStringList recommendApps = mimeAppsManager->getRecommendedApps(DUrl::fromLocalFile(filePath));
        recommendApps.removeOne(defaultDesktopFile);
        if (recommendApps.count() > 0) {
            defaultDesktopFile = recommendApps.first();
        } else {
            qDebug() << "no default application for" << filePath;
            return false;
        }
    }
    result = launchApp(defaultDesktopFile, QStringList() << DUrl::fromLocalFile(filePath).toString());
    if (result) {
        // workaround since DTK apps doesn't support the recent file spec.
        // spec: https://www.freedesktop.org/wiki/Specifications/desktop-bookmark-spec/
        // the correct approach: let the app add it to the recent list.
        // addToRecentFile(DUrl::fromLocalFile(filePath), mimetype);
        DesktopFile df(defaultDesktopFile);
        addRecentFile(filePath, df, mimetype);
        return result;
    }

    if (mimeAppsManager->getDefaultAppByFileName(filePath) == "org.gnome.font-viewer.desktop") {
        QProcess::startDetached("gio", QStringList() << "open" << filePath);
        QTimer::singleShot(200, [ = ] {
            QProcess::startDetached("gio", QStringList() << "open" << filePath);
        });
        return true;
    }

    result = QProcess::startDetached("gio", QStringList() << "open" << filePath);

    if (!result)
        return QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    return result;
}

bool FileUtils::openFiles(const QStringList &filePaths)
{
    QStringList rePath = filePaths;
    bool ret = false;
    for (const QString &filePath : filePaths) {
        if (QFileInfo(filePath).suffix() == "desktop") {
            ret = FileUtils::launchApp(filePath) || ret; //有一个成功就成功
            rePath.removeOne(filePath);
            continue;
        }
    }
    if (rePath.isEmpty())
        return ret;

    const QString filePath = rePath.first();

    /*********************************************************/
    //解决空文本文件转其他非文本格式时打开仍然是文本方式打开的问题
    //QString mimetype = getFileMimetype(filePath);
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, DUrl(FILE_ROOT + filePath));
    QString mimetype;
    if (info && info->size() == 0 && info->exists()) {
        mimetype = info->mimeType().name();
    } else {
        mimetype = getFileMimetype(filePath);
    }
    /*********************************************************/
    bool isOpenNow = false;
    QString defaultDesktopFile = MimesAppsManager::getDefaultAppDesktopFileByMimeType(mimetype);
    if (defaultDesktopFile.isEmpty()) {
        if (isSmbUnmountedFile(DUrl::fromLocalFile(filePath))) {
            mimetype = QString("inode/directory");
            defaultDesktopFile = MimesAppsManager::getDefaultAppDesktopFileByMimeType(mimetype);
            isOpenNow = true;
            mimetype = QString();
        } else {
            qDebug() << "no default application for" << filePath;
            return false;
        }
    }

    if (!isOpenNow && isFileManagerSelf(defaultDesktopFile) && mimetype != "inode/directory") {
        QStringList recommendApps = mimeAppsManager->getRecommendedApps(DUrl::fromLocalFile(filePath));
        recommendApps.removeOne(defaultDesktopFile);
        if (recommendApps.count() > 0) {
            defaultDesktopFile = recommendApps.first();
        } else {
            qDebug() << "no default application for" << rePath;
            return false;
        }
    }

    QStringList appAgrs;
    for (const QString &tmp : rePath)
        appAgrs << DUrl::fromLocalFile(tmp).toString();
    bool result = launchApp(defaultDesktopFile, appAgrs);
    if (result) {
        // workaround since DTK apps doesn't support the recent file spec.
        // spec: https://www.freedesktop.org/wiki/Specifications/desktop-bookmark-spec/
        // the correct approach: let the app add it to the recent list.
        // addToRecentFile(DUrl::fromLocalFile(filePath), mimetype);
        for (const QString &tmp : rePath) {
            QString file_path = DUrl::fromLocalFile(tmp).toLocalFile();
            DesktopFile df(defaultDesktopFile);
            addRecentFile(file_path, df, mimetype);
        }
        return result;
    } else if (isSmbUnmountedFile(DUrl::fromLocalFile(rePath[0]))) {
        return false;
    }

    if (mimeAppsManager->getDefaultAppByFileName(filePath) == "org.gnome.font-viewer.desktop") {
        QProcess::startDetached("gio", QStringList() << "open" << rePath);
        QTimer::singleShot(200, [ = ] {
            QProcess::startDetached("gio", QStringList() << "open" << rePath);
        });
        return true;
    }

    result = QProcess::startDetached("gio", QStringList() << "open" << rePath);

    if (!result) {
        result = false;
        for (const QString &tmp : rePath)
            result = QDesktopServices::openUrl(QUrl::fromLocalFile(tmp)) || result; //有一个成功就成功
    }
    return result;
}

bool FileUtils::openEnterFiles(const QStringList &filePaths)
{
    QStringList rePath = filePaths;
    bool ret = false;
    for (const QString &filePath : filePaths) {
        if (QFileInfo(filePath).suffix() == "desktop") {
            ret = FileUtils::launchApp(filePath) || ret; //有一个成功就成功
            rePath.removeOne(filePath);
            continue;
        }
    }
    if (rePath.isEmpty())
        return ret;
    //fix bug 33136 在选中3过文件，mimetype有3种不同，但是用enter键打开，这里只处理了
    //第一个文件的mimetype，根据mimetype，选定desktop，传入了3个文件地址，所以后面两个
    //打开失败，处理所有的文件的mimetype，相同的就用同一个desktop启动
    QHash<QString, QStringList> openinfo;
    QHash<QString, QString> mimetypeinfo;
    foreach (const QString &filePath, rePath) {
        /*********************************************************/
        //解决空文本文件转其他非文本格式时打开仍然是文本方式打开的问题
        //QString mimetype = getFileMimetype(filePath);
        DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, DUrl(FILE_ROOT + filePath));
        QString mimetype;
        if (info && info->size() == 0 && info->exists()) {
            mimetype = info->mimeType().name();
        } else {
            mimetype = getFileMimetype(filePath);
        }
        mimetypeinfo.insert(DUrl::fromLocalFile(filePath).toString(), mimetype);
        /*********************************************************/
        bool isOpenNow = false;
        QString defaultDesktopFile = MimesAppsManager::getDefaultAppDesktopFileByMimeType(mimetype);
        if (defaultDesktopFile.isEmpty()) {
            if (isSmbUnmountedFile(DUrl::fromLocalFile(filePath))) {
                mimetype = QString("inode/directory");
                defaultDesktopFile = MimesAppsManager::getDefaultAppDesktopFileByMimeType(mimetype);
                isOpenNow = true;
                mimetype = QString();
            } else {
                qDebug() << "no default application for" << filePath;
                continue;
            }
        }

        if (!isOpenNow && isFileManagerSelf(defaultDesktopFile) && mimetype != "inode/directory") {
            QStringList recommendApps = mimeAppsManager->getRecommendedApps(DUrl::fromLocalFile(filePath));
            recommendApps.removeOne(defaultDesktopFile);
            if (recommendApps.count() > 0) {
                defaultDesktopFile = recommendApps.first();
            } else {
                qDebug() << "no default application for" << rePath;
                continue;
            }
        }
        if (!defaultDesktopFile.isEmpty()) {
            QStringList value = openinfo.contains(defaultDesktopFile) ? openinfo.value(defaultDesktopFile) : QStringList();
            value << DUrl::fromLocalFile(filePath).toString();
            openinfo.insert(defaultDesktopFile, value);
        }
    }

    //未找到默认打开应用，需要返回false，让上层逻辑走选择默认打开程序的流程
    if (openinfo.isEmpty()) {
        return false;
    } else if (isSmbUnmountedFile(DUrl::fromLocalFile(rePath[0]))) {
        return false;
    }

    QStringList appAgrs;
    for (const QString &tmp : rePath)
        appAgrs << DUrl::fromLocalFile(tmp).toString();
    bool result = false;
    foreach (const QString &defaultDesktopFile, openinfo.keys()) {
        bool temresult = launchApp(defaultDesktopFile, openinfo.value(defaultDesktopFile));
        if (temresult) {
            // workaround since DTK apps doesn't support the recent file spec.
            // spec: https://www.freedesktop.org/wiki/Specifications/desktop-bookmark-spec/
            // the correct approach: let the app add it to the recent list.
            // addToRecentFile(DUrl::fromLocalFile(filePath), mimetype);
            for (const QString &tmp : openinfo.value(defaultDesktopFile)) {
                QString filePath = DUrl::fromUserInput(tmp).toLocalFile();
                DesktopFile df(defaultDesktopFile);
                addRecentFile(filePath, df, mimetypeinfo.value(tmp));
            }
            result = true;
        }
    }
    //只要一次成功就返回
    if (result) {
        return result;
    }

    const QString filePath = rePath.first();
    if (mimeAppsManager->getDefaultAppByFileName(filePath) == "org.gnome.font-viewer.desktop") {
        QProcess::startDetached("gio", QStringList() << "open" << rePath);
        QTimer::singleShot(200, [ = ] {
            QProcess::startDetached("gio", QStringList() << "open" << rePath);
        });
        return true;
    }

    result = QProcess::startDetached("gio", QStringList() << "open" << rePath);

    if (!result) {
        result = false;
        for (const QString &tmp : rePath)
            result = QDesktopServices::openUrl(QUrl::fromLocalFile(tmp)) || result; //有一个成功就成功
    }
    return result;
}

bool FileUtils::launchApp(const QString &desktopFile, const QStringList &filePaths)
{
    QStringList newList(filePaths);
    if (isFileManagerSelf(desktopFile) && filePaths.count() > 1) {
        foreach (const QString &filePath, filePaths) {
            // fix bug#33577在桌面上，多选文件夹不能打开
            DUrl t_file(filePath);
            QString t_filePath = t_file.toString();
            if (t_file.isLocalFile()) {
                t_filePath = t_file.toLocalFile();
            }
            openFile(t_filePath);
        }
        return true;
    }

    if (isFileManagerSelf(desktopFile) && filePaths.count() == 1) {
        DUrl fileUrl(filePaths[0]);
        if (isSmbUnmountedFile(fileUrl)) {
            newList.clear();
            newList << smbFileUrl(filePaths[0]).toString();
        }
    }

    bool ok = launchAppByDBus(desktopFile, newList);
    if (!ok) {
        ok = launchAppByGio(desktopFile, newList);
    }
    return ok;
}

bool FileUtils::launchAppByDBus(const QString &desktopFile, const QStringList &filePaths)
{
    if (appController->checkLaunchAppInterface()) {
        qDebug() << "launchApp by dbus:" << desktopFile << filePaths;
        //多个wps文件同时打开应用会报出文件不存在的错误，而单个打开不会
        //对wps文件做特殊处理，一个一个分别打开
//        if (desktopFile.endsWith("wps-office-wps.desktop")) {
//            DesktopFile deskfile(desktopFile);
//            if (deskfile.getExec().contains("%U")) { //exc标志 为%F时也可以打开多个 %U不行
//                if (!filePaths.isEmpty())
//                {
//                    for (const QString &path : filePaths) {
//                        appController->startManagerInterface()->LaunchApp(desktopFile, static_cast<uint>(QX11Info::getTimestamp()), {path});
//                    }
//                    return true;
//                }
//            }
//        }
        //以上改动有问题，暂时撤销

        appController->startManagerInterface()->LaunchApp(desktopFile, static_cast<uint>(QX11Info::getTimestamp()), filePaths);
        return true;
    }
    return false;
}

bool FileUtils::launchAppByGio(const QString &desktopFile, const QStringList &filePaths)
{
    qDebug() << "launchApp by gio:" << desktopFile << filePaths;

    std::string stdDesktopFilePath = desktopFile.toStdString();
    const char *cDesktopPath = stdDesktopFilePath.data();

    GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(cDesktopPath);
    if (!appInfo) {
        qDebug() << "Failed to open desktop file with gio: g_desktop_app_info_new_from_filename returns NULL. Check PATH maybe?";
        return false;
    }

    GList *g_files = nullptr;
    foreach (const QString &filePath, filePaths) {
        std::string stdFilePath = filePath.toStdString();
        const char *cFilePath = stdFilePath.data();
        GFile *f = g_file_new_for_uri(cFilePath);
        g_files = g_list_append(g_files, f);
    }

    GError *gError = nullptr;
    gboolean ok = g_app_info_launch(reinterpret_cast<GAppInfo *>(appInfo), g_files, nullptr, &gError);

    if (gError) {
        qWarning() << "Error when trying to open desktop file with gio:" << gError->message;
        g_error_free(gError);
    }

    if (!ok) {
        qWarning() << "Failed to open desktop file with gio: g_app_info_launch returns false";
    }
    g_object_unref(appInfo);
    g_list_free(g_files);

    return ok;
}

bool FileUtils::openFilesByApp(const QString &desktopFile, const QStringList &filePaths)
{
    bool ok = false;

    if (desktopFile.isEmpty()) {
        qDebug() << "Failed to open desktop file with gio: app file path is empty";
        return ok;
    }

    if (filePaths.isEmpty()) {
        qDebug() << "Failed to open desktop file with gio: file path is empty";
        return ok;
    }

    qDebug() << desktopFile << filePaths;

    GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(desktopFile.toLocal8Bit().constData());
    if (!appInfo) {
        qDebug() << "Failed to open desktop file with gio: g_desktop_app_info_new_from_filename returns NULL. Check PATH maybe?";
        return false;
    }

    QString terminalFlag = QString(g_desktop_app_info_get_string(appInfo, "Terminal"));
    if (terminalFlag == "true") {
        QString exec = QString(g_desktop_app_info_get_string(appInfo, "Exec"));
        QStringList args;
        args << "-e" << exec.split(" ").at(0) << filePaths;
        QString termPath = defaultTerminalPath();
        qDebug() << termPath << args;
        ok = QProcess::startDetached(termPath, args);
    } else {
        ok = launchApp(desktopFile, filePaths);
    }
    g_object_unref(appInfo);

    if (ok) {
        // workaround since DTK apps doesn't support the recent file spec.
        // spec: https://www.freedesktop.org/wiki/Specifications/desktop-bookmark-spec/
        // the correct approach: let the app add it to the recent list.
        // addToRecentFile(DUrl::fromLocalFile(filePath), mimetype);
        QString filePath = filePaths.first();
        filePath = DUrl::fromUserInput(filePath).path();
        QString mimetype = getFileMimetype(filePath);
        for (const QString &tmp : filePaths) {
            QString temFilePath = DUrl::fromUserInput(tmp).path();
            DesktopFile df(desktopFile);
            addRecentFile(temFilePath, df, mimetype);
        }
    }

    return ok;
}

bool FileUtils::isFileManagerSelf(const QString &desktopFile)
{
    /*
     *  return true if exec field contains dde-file-manager/file-manager.sh of dde-file-manager desktopFile
    */
    DesktopFile d(desktopFile);
    return d.getExec().contains("dde-file-manager") || d.getExec().contains("file-manager.sh");
}

QString FileUtils::defaultTerminalPath()
{
    const static QString dde_daemon_default_term = QStringLiteral("/usr/lib/deepin-daemon/default-terminal");
    const static QString debian_x_term_emu = QStringLiteral("/usr/bin/x-terminal-emulator");

    if (QFileInfo::exists(dde_daemon_default_term)) {
        return dde_daemon_default_term;
    } else if (QFileInfo::exists(debian_x_term_emu)) {
        return debian_x_term_emu;
    }

    return QStandardPaths::findExecutable("xterm");
}

bool FileUtils::setBackground(const QString &pictureFilePath)
{
    QDBusMessage msgIntrospect = QDBusMessage::createMethodCall("com.deepin.daemon.Appearance", "/com/deepin/daemon/Appearance", "org.freedesktop.DBus.Introspectable", "Introspect");
    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msgIntrospect);
    call.waitForFinished();
    if (call.isFinished()) {
        QDBusReply<QString> reply = call.reply();
        QString value = reply.value();

        if (value.contains("SetMonitorBackground")) {
            QDBusMessage msg = QDBusMessage::createMethodCall("com.deepin.daemon.Appearance", "/com/deepin/daemon/Appearance", "com.deepin.daemon.Appearance", "SetMonitorBackground");
            if (DesktopInfo().waylandDectected()) {
                QDBusInterface interface("com.deepin.daemon.Display", "/com/deepin/daemon/Display", "com.deepin.daemon.Display");
                QString screenname = qvariant_cast< QString >(interface.property("Primary"));
                msg.setArguments({screenname, pictureFilePath});
            }

            else {
                msg.setArguments({qApp->primaryScreen()->name(), pictureFilePath});
            }

            QDBusConnection::sessionBus().asyncCall(msg);
            qDebug() << "FileUtils::setBackground call Appearance SetMonitorBackground";
            return true;
        }
    }

    QDBusMessage msg = QDBusMessage::createMethodCall("com.deepin.daemon.Appearance", "/com/deepin/daemon/Appearance", "com.deepin.daemon.Appearance", "Set");
    msg.setArguments({"Background", pictureFilePath});
    QDBusConnection::sessionBus().asyncCall(msg);
    qDebug() << "FileUtils::setBackground call Appearance Set";

    return true;
}

QString FileUtils::md5(const QString &fpath)
{
    QFile file(fpath);

    if (file.open(QIODevice::ReadOnly)) {
        return QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
    }

    return QCryptographicHash::hash(fpath.toLocal8Bit(), QCryptographicHash::Md5).toHex();
}

QByteArray FileUtils::md5(QFile *file, const QString &filePath)
{
    QByteArray md5 = filePath.toLocal8Bit() + QByteArray::number(file->size());

    if (file->open(QIODevice::ReadOnly)) {
        if (file->size() < 8192) {
            md5 += file->readAll();
        } else {
            char data[4097] = {};

            file->read(data, 4096);

            md5.append(data);

            file->seek(file->size() - 4096);

            file->read(data, 4096);

            md5.append(data);
        }

        file->close();
    }

    return QCryptographicHash::hash(md5, QCryptographicHash::Md5).toHex();;
}

bool FileUtils::isFileExecutable(const QString &path)
{
    QFile file(path);

    // regard these type as unexecutable.
    const static QStringList noValidateType {"txt", "md"};
    if (noValidateType.contains(QFileInfo(file).suffix()))
        return false;

    bool isExeUser = false;

    // Vault file need to use stat function to read file permission.
    if (VaultController::isVaultFile(path)) {
        struct stat buf;
        std::string stdStr = path.toStdString();
        stat(stdStr.c_str(), &buf);
        if ((buf.st_mode & S_IXUSR)) {
            isExeUser = true;
        }
    } else {
        isExeUser = file.permissions() & QFile::ExeUser;
    }

    return (file.permissions() & QFile::ReadUser) && isExeUser;
}

// sort of.. duplicate with FileUtils::isFileRunnable
bool FileUtils::shouldAskUserToAddExecutableFlag(const QString &path)
{
    QString _path = path;
    QFileInfo info(path);
    QString mimetype = getFileMimetype(path);
    if (info.isSymLink()) {
        _path = QFile(path).symLinkTarget();
        mimetype = getFileMimetype(path);
    }

    if (mimetype == "application/x-executable"
            || mimetype == "application/x-sharedlib"
            || mimetype == "application/x-iso9660-appimage"
            || mimetype == "application/vnd.appimage") {
        return !isFileExecutable(_path);
    }

    return false;
}

bool FileUtils::isFileRunnable(const QString &path)
{
    QString _path = path;
    QFileInfo info(path);
    QString mimetype = getFileMimetype(path);
    qDebug() << info.isSymLink() << mimetype;
    if (info.isSymLink()) {
        _path = QFile(path).symLinkTarget();
        mimetype = getFileMimetype(path);
    }

    // blumia: about AppImage mime type, please refer to:
    //         https://cgit.freedesktop.org/xdg/shared-mime-info/tree/freedesktop.org.xml.in
    //         btw, consider using MimeTypeDisplayManager::ExecutableMimeTypes(private) ?
    if (mimetype == "application/x-executable"
            || mimetype == "application/x-sharedlib"
            || mimetype == "application/x-iso9660-appimage"
            || mimetype == "application/vnd.appimage") {
        return isFileExecutable(_path);
    }

    return false;
}


bool FileUtils::isFileWindowsUrlShortcut(const QString &path)
{
    QString mimetype = getFileMimetype(path);
    qDebug() << mimetype;
    if (mimetype == "application/x-mswinurl")
        return true;
    return false;
}

QString FileUtils::getInternetShortcutUrl(const QString &path)
{
    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup("InternetShortcut");
    QString url = settings.value("URL").toString();
    settings.endGroup();
    return url;
}


QString FileUtils::getFileMimetype(const QString &path)
{
    GFile *file;
    GFileInfo *info;
    QString result;

    file = g_file_new_for_path(path.toUtf8());
    info = g_file_query_info(file, "standard::content-type", G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
    result = g_file_info_get_content_type(info);

    g_object_unref(file);

    return result;
}

bool FileUtils::isExecutableScript(const QString &path)
{
    QString _path = path;
    QFileInfo info(path);
    QString mimetype = getFileMimetype(path);
    qDebug() << info.isSymLink() << mimetype;

    // anke requirement
//    if (info.size() == 0) {
//        return false;
//    }

    if (info.isSymLink()) {
        _path = QFile(path).symLinkTarget();
        mimetype = getFileMimetype(path);
    }

    // blumia: it's not a good idea to check if it is a executable script by just checking
    //         mimetype.startsWith("text/"), should be fixed later.
    if (mimetype.startsWith("text/") ||
            (mimetype == "application/x-shellscript")) {
        return isFileExecutable(_path);
    }

    return false;
}

bool FileUtils::openExcutableScriptFile(const QString &path, int flag)
{
    bool result = false;
    switch (flag) {
    case 0:

        break;
    case 1:
        result = runCommand(path, QStringList(), QUrl(path).adjusted(QUrl::RemoveFilename).toString());
        break;
    case 2: {
        QStringList args;
        args << "-e" << path;
        result = runCommand(FileUtils::defaultTerminalPath(), args, QUrl(path).adjusted(QUrl::RemoveFilename).toString());
        break;
    }
    case 3:
        result = openFile(path);
        break;
    default:
        break;
    }

    return result;
}

bool FileUtils::addExecutableFlagAndExecuse(const QString &path, int flag)
{
    bool result = false;
    QFile file(path);
    switch (flag) {
    case 0:
        break;
    case 1:
        file.setPermissions(file.permissions() | QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther);
        result = runCommand(path, QStringList());
        break;
    default:
        break;
    }

    return result;
}

bool FileUtils::openExcutableFile(const QString &path, int flag)
{
    bool result = false;
    switch (flag) {
    case 0:
        break;
    case 1: {
        QStringList args;
        args << "-e" << path;
        result = runCommand(FileUtils::defaultTerminalPath(), args, QUrl(path).adjusted(QUrl::RemoveFilename).toString());
        break;
    }
    case 2:
        result = runCommand(path, QStringList(), QUrl(path).adjusted(QUrl::RemoveFilename).toString());
        break;
    default:
        break;
    }

    return result;
}

bool FileUtils::runCommand(const QString &cmd, const QStringList &args, const QString &wd)
{
    bool result = false;
    if (appController->checkLaunchAppInterface()) {
        qDebug() << "launch cmd by dbus:" << cmd << args;
        if (wd.length()) {
            QVariantMap opt = {{"dir", wd}};
            appController->startManagerInterface()->RunCommandWithOptions(cmd, args, opt);
        } else
            appController->startManagerInterface()->RunCommand(cmd, args);
        result = true;
    } else {
        qDebug() << "launch cmd by qt:" << cmd << args;
        result = QProcess::startDetached(cmd, args, wd);
    }
    return result;
}

void FileUtils::mkpath(const DUrl &path)
{
    if (path.parentUrl() == path) {
        return;
    }
    if (fileService->createFileInfo(nullptr, path)->isDir()) {
        return;
    }
    if (fileService->mkdir(nullptr, path)) {
        return;
    }
    mkpath(path.parentUrl());
    fileService->mkdir(nullptr, path);
}

QString FileUtils::displayPath(const QString &pathStr)
{
    QString devicePath = pathStr;
    QString homeDir = QDir::homePath();
    if (devicePath.startsWith(homeDir)) {
        devicePath.replace(0, homeDir.length(), "~");
    }

    return devicePath;
}

QByteArray FileUtils::imageFormatName(QImage::Format f)
{
    switch (f) {
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
    case QImage::Format_ARGB8565_Premultiplied:
    case QImage::Format_ARGB6666_Premultiplied:
    case QImage::Format_ARGB8555_Premultiplied:
    case QImage::Format_ARGB4444_Premultiplied:
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied:
    case QImage::Format_A2BGR30_Premultiplied:
    case QImage::Format_A2RGB30_Premultiplied:
    case QImage::Format_Alpha8:
        return "png";
    default:
        break;
    }

    return "jpeg";
}


QString FileUtils::getFileContent(const QString &file)
{
    QFile f(file);
    QString fileContent = "";
    if (f.open(QFile::ReadOnly)) {
        fileContent = QString(f.readAll());
        f.close();
    } else {
        qDebug() << "Could not read file " << file << ":" << f.errorString();
    }
    return fileContent;
}

bool FileUtils::writeTextFile(const QString &filePath, const QString &content)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in << content << endl;
        file.close();
        return true;
    } else {
        qDebug() << "Failed to write content to file " << filePath << ":" << file.errorString();
    }
    return false;
}

void FileUtils::migrateConfigFileFromCache(const QString &key)
{
    bool ret = false;
    QString oldPath = QString("%1/%2/%3.%4").arg(QDir().homePath(), ".cache/dde-file-manager", key, "json");
    QString newPath = QString("%1/%2.%3").arg(DFMStandardPaths::location(DFMStandardPaths::ApplicationConfigPath), key.toLower(), "json");
    QFile srcFile(oldPath);
    ret = srcFile.open(QIODevice::ReadOnly);
    if (ret) {
        QByteArray data = srcFile.readAll();
        srcFile.close();

        QFile desFile(newPath);
        ret = desFile.open(QIODevice::WriteOnly);
        if (ret) {
            qint64 code = desFile.write(data);
            if (code < 0) {
                qDebug() << "Error occurred when writing data";
                ret = false;
            } else {
                ret = srcFile.remove();
                if (!ret) {
                    qDebug() << "Failed to remove source file " << oldPath;
                }
            }
            desFile.close();

        } else {
            qDebug() << "Failed to write data :" << desFile.errorString();
        }

    } else {
        qDebug() << "Could not read source file " << oldPath << ":" << srcFile.errorString();
    }

    if (!ret) {
        qDebug() << "Failed to migrate config file from cache";
    }
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

DFMGlobal::MenuExtension FileUtils::getMenuExtension(const DUrlList &urlList)
{
    int fileCount = 0;
    int dirCount = 0;
    foreach (DUrl url, urlList) {
        QFileInfo info(url.toLocalFile());
        if (info.isDir()) {
            dirCount += 1;
        } else if (info.isFile()) {
            fileCount += 1;
        }
    }
    if (urlList.count() == 0) {
        return DFMGlobal::MenuExtension::EmptyArea;
    } else if (fileCount == 1 && dirCount == 0 && fileCount == urlList.count()) {
        return DFMGlobal::MenuExtension::SingleFile;
    } else if (fileCount > 1 && dirCount == 0 && fileCount == urlList.count()) {
        return DFMGlobal::MenuExtension::MultiFiles;
    } else if (fileCount == 0 && dirCount == 1 && dirCount == urlList.count()) {
        return DFMGlobal::MenuExtension::SingleDir;
    } else if (fileCount == 0 && dirCount > 1 && dirCount == urlList.count()) {
        return DFMGlobal::MenuExtension::MultiDirs;
    } else if (urlList.count() > 1) {
        return DFMGlobal::MenuExtension::MultiFileDirs;
    } else {
        return DFMGlobal::MenuExtension::UnknowMenuExtension;
    }
}

bool FileUtils::isGvfsMountFile(const QString &filePath, const bool &isEx)
{
    static QMutex mutex;
    QMutexLocker lk(&mutex);
    if (filePath.isEmpty())
        return false;

    static const QString gvfsPath = QString("/run/user/") + QString::number(getuid()) + QString("/gvfs");

    if (filePath.startsWith(gvfsPath) && filePath != gvfsPath)
        return true;
    if (filePath == gvfsPath)
        return false;

    bool isgvfsfile = !DStorageInfo::isLocalDevice(filePath, isEx);

    return isgvfsfile;
}

bool FileUtils::isFileExists(const QString &filePath)
{
    GFile *file;
    std::string fstdPath = filePath.toStdString();
    file = g_file_new_for_path(fstdPath.data());
    bool isExists = g_file_query_exists(file, nullptr);
    g_object_unref(file);
    return isExists;
}

QJsonObject FileUtils::getJsonObjectFromFile(const QString &filePath)
{
    QJsonObject obj;
    QJsonDocument doc;
    if (!QFile::exists(filePath))
        return obj;

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "cannot read file " << filePath << ":" << file.errorString();
        file.close();
        return obj;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError *jsError = nullptr;
    doc = QJsonDocument::fromJson(data, jsError);

    if (jsError) {
        qDebug() << "cache data pase error:" << jsError->errorString();
        return obj;
    }

    obj = doc.object();
    return obj;
}

QJsonArray FileUtils::getJsonArrayFromFile(const QString &filePath)
{
    QJsonArray array;
    QJsonDocument doc;
    if (!QFile::exists(filePath))
        return array;

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "cannot read file " << filePath << ":" << file.errorString();
        file.close();
        return array;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError *jsError = nullptr;
    doc = QJsonDocument::fromJson(data, jsError);

    if (jsError) {
        qDebug() << "cache data pase error:" << jsError->errorString();
        return array;
    }

    array = doc.array();
    return array;
}

bool FileUtils::writeJsonObjectFile(const QString &filePath, const QJsonObject &obj)
{
    QJsonDocument doc;
    doc.setObject(obj);
    return writeTextFile(filePath, doc.toJson().data());
}

bool FileUtils::writeJsonnArrayFile(const QString &filePath, const QJsonArray &array)
{
    QJsonDocument doc;
    doc.setArray(array);
    return writeTextFile(filePath, doc.toJson().data());
}

void FileUtils::mountAVFS()
{
    QProcess p;
    p.start("/usr/bin/umountavfs");
    p.waitForFinished();
    QProcess::startDetached("/usr/bin/mountavfs");
}

void FileUtils::umountAVFS()
{
    QProcess::startDetached("/usr/bin/umountavfs");
}

void FileUtils::addRecentFile(const QString &filePath, const DesktopFile &desktopFile, const QString &mimetype)
{
    if (filePath.isEmpty()) {
        return;
    }
    DRecentData recentData;
    recentData.appName = desktopFile.getName();
    recentData.appExec = desktopFile.getExec();
    recentData.mimeType = mimetype;
    DRecentManager::removeItem(filePath);
    DRecentManager::addItem(filePath, recentData);
}

bool FileUtils::appendCompress(const DUrl &toUrl, const DUrlList &fromUrlList)
{
    if (!fromUrlList.isEmpty()) {
        QStringList arguments {toUrl.toLocalFile()};
        foreach (const DUrl &url, fromUrlList) {
            // 如果是avfs文件，将路径转换为真实路径
            if (url.isAVFSFile()) {
                arguments << AVFSFileInfo::realFileUrl(url).toLocalFile();
            } else {
                arguments << url.toLocalFile();
            }
        }
        arguments << "dragdropadd";
        return QProcess::startDetached("deepin-compressor", arguments);
    }
    return false;
}
//获取当前內存页大小
int FileUtils::getMemoryPageSize()
{
    static const int memoryPageSize = getpagesize();
    return memoryPageSize > 0 ? memoryPageSize : 4096;
}

bool FileUtils::isFtpFile(const DUrl &url)
{
    static QRegularExpression regExp("^/run/user/\\d+/gvfs/.+$",
                                     QRegularExpression::DotMatchesEverythingOption
                                     | QRegularExpression::DontCaptureOption
                                     | QRegularExpression::OptimizeOnFirstUsageOption);

    if (!regExp.match(url.path(), 0, QRegularExpression::NormalMatch, QRegularExpression::DontCheckSubjectStringMatchOption).hasMatch()) {
        return false;
    }
    if (url.path().contains("/ftp:host="))
        return true;
    return false;
}
/**
 * @brief jugment file is local file(in system disk file)(使用gio判断挂载点是否可以卸载)
 * @param url file url
 */
bool FileUtils::isFileOnDisk(const QString &path)
{
    if (path.isEmpty())
        return false;
    bool isLocal = true;
    GFile *dest_dir_file = g_file_new_for_path(path.toUtf8().constData());
    GMount *dest_dir_mount = g_file_find_enclosing_mount(dest_dir_file, nullptr, nullptr);
    if (dest_dir_mount) {
        isLocal = !g_mount_can_unmount(dest_dir_mount);
        g_object_unref(dest_dir_mount);
    }
    g_object_unref(dest_dir_file);
    return isLocal;
}
//获取cpu的核个数
qint32 FileUtils::getCpuProcessCount()
{
    static const int cpuProcessCount = sysconf(_SC_NPROCESSORS_CONF) < 4 ?
                4 : static_cast<int>(sysconf(_SC_NPROCESSORS_CONF));
    return cpuProcessCount;
}
/**
 * @brief jugment file is mount 通过gioqt获取所有的挂载点，对比当前的url是否在挂载点中，只判断了smb和ftp
 * @param url file url
 */
bool FileUtils::isNetworkUrlMounted(const DUrl &url)
{
    for (auto gvfsmp : DGioVolumeManager::getMounts()) {
        auto rootFile = gvfsmp->getRootFile();
        if (!rootFile || (!rootFile->uri().contains("smb")
                && !rootFile->uri().contains("ftp")))
            continue;

        DUrl mountUrl;
        mountUrl.setScheme(DFMROOT_SCHEME);
        mountUrl.setPath("/" + QUrl::toPercentEncoding(rootFile->path()) + "." SUFFIX_GVFSMP);
        if (mountUrl == url)
            return true;
    }
    return false;
}

bool FileUtils::isSambaServiceRunning()
{
    QDBusInterface iface("org.freedesktop.systemd1",
                         "/org/freedesktop/systemd1/unit/smbd_2eservice",
                         "org.freedesktop.systemd1.Unit",
                         QDBusConnection::systemBus());

    if (iface.isValid()) {
        const QVariant &variantStatus = iface.property("SubState"); // 获取属性 SubState，等同于 systemctl status smbd 结果 Active 值
        if (variantStatus.isValid())
            return "running" == variantStatus.toString();
    }
    return false;
}

DUrl FileUtils::smbFileUrl(const QString &filePath)
{
    static QRegularExpression regExp("file:///run/user/\\d+/gvfs/smb-share:server=(?<host>.*),share=(?<path>.*)",
                                     QRegularExpression::DotMatchesEverythingOption
                                     | QRegularExpression::DontCaptureOption
                                     | QRegularExpression::OptimizeOnFirstUsageOption);

    const QRegularExpressionMatch &match = regExp.match(filePath, 0, QRegularExpression::NormalMatch,
                                                        QRegularExpression::DontCheckSubjectStringMatchOption);

    if (!match.hasMatch())
        return DUrl::fromLocalFile(filePath);

    const QString &host = match.captured("host");
    const QString &path = match.captured("path");

    DUrl newUrl;
    newUrl.setScheme("smb");
    newUrl.setHost(host);
    newUrl.setPath("/" + path.mid(0, path.lastIndexOf("/")));
    return newUrl;
}

bool FileUtils::isSmbUnmountedFile(const DUrl &url)
{
    return url.path().startsWith("/run/user/")
            && url.path().contains("/gvfs/smb-share:server=")
            && DFileService::instance()->checkGvfsMountfileBusy(url, false);
}

//优化苹果文件不卡显示，存在判断错误的可能，只能临时优化，需系统提升ios传输效率
bool FileUtils::isDesktopFile(const QString &filePath)
{
    QMimeType mt = DMimeDatabase().mimeTypeForFile(filePath);
    return mt.name() == "application/x-desktop" && mt.suffixes().contains("desktop", Qt::CaseInsensitive);
}

bool FileUtils::isDesktopFile(const QFileInfo &fileInfo)
{
    QMimeType mt = DMimeDatabase().mimeTypeForFile(fileInfo);
    return mt.name() == "application/x-desktop" && mt.suffixes().contains("desktop", Qt::CaseInsensitive);
}

bool FileUtils::isDesktopFile(const QString &filePath, QMimeType &mimetype)
{
    QMimeType mt = DMimeDatabase().mimeTypeForFile(filePath, QMimeDatabase::MatchExtension);
    mimetype = mt;
    return mt.name() == "application/x-desktop" &&
           mt.suffixes().contains("desktop", Qt::CaseInsensitive);
}

bool FileUtils::isDesktopFile(const QFileInfo &fileInfo, QMimeType &mimetyp)
{
    QMimeType mt = DMimeDatabase().mimeTypeForFile(fileInfo, QMimeDatabase::MatchExtension);
    mimetyp = mt;
    return mt.name() == "application/x-desktop" &&
           mt.suffixes().contains("desktop", Qt::CaseInsensitive);

}
