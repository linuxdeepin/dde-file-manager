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

#include "filejob.h"

#include "shutil/fileutils.h"
#include "interfaces/dfmstandardpaths.h"
#include "../deviceinfo/udiskdeviceinfo.h"
#include "../deviceinfo/udisklistener.h"
#include "../app/define.h"
#include "singleton.h"
#include "../app/filesignalmanager.h"
#include "interfaces/dfmglobal.h"
#include "dfmstandardpaths.h"
#include "partman/partition.h"
#include "dfmevent.h"
#include "dfmeventdispatcher.h"
#include "dabstractfilewatcher.h"
#include "ddiskmanager.h"
#include "dblockdevice.h"
#include "ddiskdevice.h"
#include "disomaster.h"

#include "tag/tagmanager.h"

#ifdef SW_LABEL
#include "sw_label/llsdeepinlabellibrary.h"
#endif

#include <functional>
#include <QFile>
#include <QThread>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QElapsedTimer>
#include <QDirIterator>
#include <QProcess>
#include <QCryptographicHash>
#include <QMetaEnum>
#include <QStorageInfo>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include "sort.h"

#include <qplatformdefs.h>

DFM_USE_NAMESPACE

int FileJob::FileJobCount = 0;
DUrlList FileJob::CopyingFiles = {};
qint64 FileJob::Msec_For_Display = 1000;
qint64 FileJob::Data_Block_Size = 139624;
qint64 FileJob::Data_Flush_Size = 16777216;

#define IS_IO_ERROR(__error, KIND) (((__error)->domain == G_IO_ERROR && (__error)->code == G_IO_ERROR_ ## KIND))


static inline char * ptr_align (char const *ptr, size_t alignment)
{
  char const *p0 = ptr;
  char const *p1 = p0 + alignment - 1;
  return (char *) (p1 - (size_t) p1 % alignment);
}


bool FileJob::setDirPermissions(const QString &scrPath, const QString& tarDirPath)
{
    struct stat buf;
    std::string stdSrcPath = scrPath.toStdString();
    stat(stdSrcPath.data(), &buf);
    std::string stdTarDirPath = tarDirPath.toStdString();
    /*07777 represent permission of Linux*/
    bool success = ::chmod(stdTarDirPath.data(), buf.st_mode & 07777) == 0;
    return success;
}

FileJob::FileJob(JobType jobType, QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QMap<QString, QString>>();
    qRegisterMetaType<DISOMasterNS::DISOMaster::JobStatus>(QT_STRINGIFY(DISOMasterNS::DISOMaster::JobStatus));
    FileJobCount += 1;
    m_status = FileJob::Started;

    m_abortGCancellable = g_cancellable_new();

    m_trashLoc = QString("%1/.local/share/Trash").arg(QDir::homePath());
    m_id = QString::number(FileJobCount);
    m_jobType = jobType;
    m_jobDetail.insert("jobId", m_id);
    QMetaEnum metaEnum = QMetaEnum::fromType<JobType>();
    QString type = metaEnum.valueToKey(m_jobType);
    m_jobDetail.insert("type", type.toLower());
    connect(this, &FileJob::finished, this, &FileJob::handleJobFinished);

#ifdef SPLICE_CP
    if(pipe(m_filedes) < 0) {
       qDebug() << "Create  pipe fail";
    }
    qDebug() << "Create  pipe successfully";
#endif
}

FileJob::~FileJob()
{
#ifdef SPLICE_CP
    close(m_filedes[0]);
    close(m_filedes[1]);
    qDebug() << "close pipe";
#endif
    free(m_buffer);
}

void FileJob::setStatus(FileJob::Status status)
{
    m_status = status;
}

void FileJob::setJobId(const QString &id)
{
    m_id = id;
}

QString FileJob::getJobId()
{
    return m_id;
}

QString FileJob::checkDuplicateName(const QString &name)
{
    QString destUrl = name;
    QFile file(destUrl);
    QFileInfo startInfo(destUrl);
    int num = 1;
    QString cpy = tr("copy");
    while (file.exists())
    {
        if(num == 1)
        {
            if(startInfo.isDir()){
                destUrl = QString("%1/%2(%3)").arg(startInfo.absolutePath(),
                                                   startInfo.fileName(),
                                                   cpy);
            }
            else{
                if (startInfo.completeSuffix().isEmpty()){
                    destUrl = QString("%1/%2(%3)").arg(startInfo.absolutePath(),
                                                          startInfo.baseName(),
                                                          cpy);
                }else{
                    destUrl = QString("%1/%2(%3).%4").arg(startInfo.absolutePath(),
                                                          startInfo.baseName(),
                                                          cpy,
                                                          startInfo.completeSuffix());
                }
            }
        }
        else
        {
            if(startInfo.isDir()){
                destUrl = QString("%1/%2(%3 %4)").arg(startInfo.absolutePath(),
                                                      startInfo.fileName(),
                                                      cpy,
                                                      QString::number(num));
            }
            else{
                if (startInfo.completeSuffix().isEmpty()){
                    destUrl = QString("%1/%2(%3 %4)").arg(startInfo.absolutePath(),
                                                             startInfo.baseName(),
                                                             cpy,
                                                             QString::number(num));
                }else{
                    destUrl = QString("%1/%2(%3 %4).%5").arg(startInfo.absolutePath(),
                                                             startInfo.baseName(),
                                                             cpy,
                                                             QString::number(num),
                                                             startInfo.completeSuffix());
                }
            }
        }
        num++;
        file.setFileName(destUrl);
    }
    return destUrl;
}

void FileJob::setApplyToAll(bool v)
{
    m_applyToAll = v;
}

void FileJob::setReplace(bool v)
{
    m_isReplaced = v;
}

QString FileJob::getTargetDir()
{
    return m_tarPath;
}

void FileJob::adjustSymlinkPath(QString &scrPath, QString &tarDirPath)
{
    QFileInfo srcFileInfo(scrPath);
    QFileInfo tarDirFileInfo(tarDirPath);
    if (!srcFileInfo.canonicalFilePath().isEmpty()){
        scrPath = srcFileInfo.canonicalFilePath();
    }
    if (!tarDirFileInfo.canonicalFilePath().isEmpty()){
        tarDirPath = tarDirFileInfo.canonicalFilePath();
    }
}

DUrlList FileJob::doCopy(const DUrlList &files, const DUrl &destination)
{
    m_noPermissonUrls.clear();
    DUrlList result = doMoveCopyJob(files, destination);
    if (!m_noPermissonUrls.isEmpty()){
        DFMUrlListBaseEvent noPermissionEvent(nullptr, m_noPermissonUrls);
        noPermissionEvent.setWindowId(getWindowId());
        emit fileSignalManager->requestShowNoPermissionDialog(noPermissionEvent);
    }
    m_noPermissonUrls.clear();
    return result;
}

DUrlList FileJob::doMove(const DUrlList &files, const DUrl &destination)
{
    m_noPermissonUrls.clear();
    DUrlList new_list;

    for (const DUrl &url : files) {
        if (canMove(url.toLocalFile()))
            new_list << url;
        else
            m_noPermissonUrls << url;
    }

    DUrlList result;

    if (!new_list.isEmpty())
        result = doMoveCopyJob(new_list, destination);

    if (!m_noPermissonUrls.isEmpty()){
        DFMUrlListBaseEvent noPermissionEvent(nullptr, m_noPermissonUrls);
        noPermissionEvent.setWindowId(getWindowId());
        emit fileSignalManager->requestShowNoPermissionDialog(noPermissionEvent);
    }

    m_noPermissonUrls.clear();

    return result;
}

DUrlList FileJob::doMoveCopyJob(const DUrlList &files, const DUrl &destination)
{
    qDebug() << "Do file operation is started" << m_jobDetail;
    jobPrepared();

    m_isGvfsFileOperationUsed = checkUseGvfsFileOperation(files, destination);

    DUrlList list;
    QString tarDirPath = destination.toLocalFile();
    QDir tarDir(tarDirPath);
    QStorageInfo tarStorageInfo = getStorageInfo(tarDirPath);
    if (files.count() > 0){
        QStorageInfo srcStorageInfo = getStorageInfo(files.at(0).toLocalFile());
        if (srcStorageInfo.rootPath() != tarStorageInfo.rootPath()){
            m_isInSameDisk = false;
        }else if (FileUtils::isGvfsMountFile(destination.toLocalFile())){
            UDiskDeviceInfoPointer pSrc = deviceListener->getDeviceByFilePath(files.at(0).toLocalFile());
            UDiskDeviceInfoPointer pDes = deviceListener->getDeviceByFilePath(destination.toLocalFile());
            if (pSrc && pDes){
                qDebug() << pSrc->getMountPointUrl() << pDes->getMountPointUrl();
                if (pSrc->getMountPointUrl() != pDes->getMountPointUrl()){
                    m_isInSameDisk = false;
                }
            }
        }
    }

    qDebug() << "m_isInSameDisk" << m_isInSameDisk;
    qDebug() << "mountpoint" << tarStorageInfo.rootPath();

    //No need to check dist usage for moving job in same disk
    if(!((m_jobType == Move || m_jobType == Trash || m_jobType == Restore) && m_isInSameDisk)){
        const bool diskSpaceAvailable = checkDiskSpaceAvailable(files, destination);
        m_isCheckingDisk = false;
        if(!diskSpaceAvailable){
            emit requestNoEnoughSpaceDialogShowed();
            emit requestJobRemovedImmediately(m_jobDetail);
            return DUrlList();
        }
    } else{
        m_totalSize = FileUtils::totalSize(files);
    }

    qDebug() << "m_totalSize" << FileUtils::formatSize(m_totalSize);

    if(!tarDir.exists())
    {
        qDebug() << "Destination must be directory";
        return list;
    }

    for(int i = 0; i < files.size(); i++)
    {
        QUrl url = files.at(i);
        QString srcPath = url.toLocalFile();
        if (srcPath.isEmpty()){
            continue;
        }
        QFileInfo srcInfo(srcPath);

        if (!srcInfo.exists() && !srcInfo.isSymLink())
            continue;

        QString targetPath;

        if (m_isAborted)
            break;

        if (srcInfo.isSymLink()){
            handleSymlinkFile(srcPath, tarDirPath, &targetPath);
        }else if (srcInfo.isDir()){
            adjustSymlinkPath(srcPath, tarDirPath);
            if (m_jobType == Copy){
                copyDir(srcPath, tarDirPath, false,  &targetPath);
            }else if (m_jobType == Move || m_jobType == Restore){
                if(m_isInSameDisk)
                {
                    if (!moveDir(srcPath, tarDirPath, &targetPath)) {
                        if(copyDir(srcPath, tarDirPath, true, &targetPath))
                            deleteDir(srcPath);
                    }
                }
                else
                {
                    if(copyDir(srcPath, tarDirPath, true, &targetPath))
                        if (!m_isSkip)
                            deleteDir(srcPath);
                }
            }else if (m_jobType == Trash){
                bool canTrash = moveDirToTrash(srcPath, &targetPath);
                if(m_isInSameDisk)
                {
                    if (canTrash){
                        QDir sourceDir(srcPath);
                        if (!sourceDir.rename(srcPath, targetPath)) {
                            if (QProcess::execute("mv -T \"" + srcPath.toUtf8() + "\" \"" + targetPath.toUtf8() + "\"") != 0) {
                                qDebug() << "Unable to trash dir:" << srcPath;
                            }
                        }
                    }
                }
                else
                {
                    if(copyDir(srcPath, tarDirPath, true, &targetPath))
                        if (!m_isSkip)
                            deleteDir(srcPath);
                }
            }
        }else{
            adjustSymlinkPath(srcPath, tarDirPath);
            if (m_jobType == Copy){
                copyFile(srcPath, tarDirPath, false,  &targetPath);
            }else if (m_jobType == Move || m_jobType == Restore){
                if(m_isInSameDisk)
                {
                    if (!moveFile(srcPath, tarDirPath, &targetPath)) {
#ifndef SW_LABEL
                        if(copyFile(srcPath, tarDirPath, true, &targetPath))
                            deleteFile(srcPath);
#endif
                    }
                }
                else
                {
                    if(copyFile(srcPath, tarDirPath, true, &targetPath)){
                        if (!m_isSkip)
                            deleteFile(srcPath);
                    }
                }
            }else if (m_jobType == Trash){
                bool canTrash = moveFileToTrash(srcPath, &targetPath);
                if(m_isInSameDisk)
                {
                    if (canTrash){
                        QFile localFile(srcPath);
                        if (!localFile.rename(targetPath))
                        {
                            if (QProcess::execute("mv -T \"" + srcPath.toUtf8() + "\" \"" + targetPath.toUtf8() + "\"") != 0) {
                                //Todo: find reason
                                qDebug() << "Unable to trash file:" << localFile.fileName();
//                                emit fileSignalManager->requestShowNoPermissionDialog(DUrl::fromLocalFile(srcPath));
                                m_noPermissonUrls << DUrl::fromLocalFile(srcPath);
                            }
                        }
                    }
                }
                else
                {
                    if(copyFile(srcPath, tarDirPath, true, &targetPath))
                        if (!m_isSkip)
                            deleteFile(srcPath);
                }
            }
        }

        if (!targetPath.isEmpty())
            list << DUrl::fromLocalFile(targetPath);
        else
            list << DUrl();
    }
    if(m_isJobAdded)
        jobRemoved();
    emit finished();
    qDebug() << "Do file operation is done" << m_jobDetail;

    foreach (DUrl url, list) {
        CopyingFiles.removeOne(url);
    }

    return list;
}

void FileJob::doDelete(const DUrlList &files)
{
    qDebug() << "Do delete is started";
    m_noPermissonUrls.clear();
    for(int i = 0; i < files.size(); i++)
    {
        QUrl url = files.at(i);
        QFileInfo info(url.path());

        if (!info.exists() && !info.isSymLink())
            continue;

        if (info.isFile() || info.isSymLink()){
            deleteFile(url.path());
        }else{
            if (!deleteDir(url.path())) {
                QProcess::execute("rm -r \"" + url.path().toUtf8() + "\"");
            }
        }
    }
    if(m_isJobAdded)
        jobRemoved();
    emit finished();
    qDebug() << "Do delete is done!";
    if (!m_noPermissonUrls.isEmpty()){
        DFMUrlListBaseEvent noPermissionEvent(nullptr, m_noPermissonUrls);
        noPermissionEvent.setWindowId(getWindowId());
        emit fileSignalManager->requestShowNoPermissionDialog(noPermissionEvent);
    }
    m_noPermissonUrls.clear();
}

DUrlList FileJob::doMoveToTrash(const DUrlList &files)
{
    QDir trashDir;
    DUrlList list;

    if(!trashDir.mkpath(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath))) {
        qDebug() << "mk" << DFMStandardPaths::location(DFMStandardPaths::TrashInfosPath) << "failed!";
        /// TODO

        return list;
    }

    if(!trashDir.mkpath(DFMStandardPaths::location(DFMStandardPaths::TrashInfosPath))) {
        qDebug() << "mk" << DFMStandardPaths::location(DFMStandardPaths::TrashInfosPath) << "failed!";
        /// TODO

        return list;
    }

    if (files.count() > 0){
        QStorageInfo storageInfo = getStorageInfo(files.at(0).toLocalFile());
        QStorageInfo trashStorageInfo = getStorageInfo(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath));
        if(storageInfo.rootPath() != trashStorageInfo.rootPath()){
            m_isInSameDisk = false;
        }
    }

    //store url list whom cannot be moved to trash
    DUrlList canMoveToTrashList;
    DUrlList canNotMoveToTrashList;

    for(int i = 0; i < files.size(); i++)
    {
        DUrl url = files.at(i);
        if (!m_isInSameDisk){
            //check if is target file in the / disk
            bool canMoveToTrash = checkTrashFileOutOf1GB(url);
            if(!canMoveToTrash){
                canNotMoveToTrashList << url;
                continue;
            }
        }
        canMoveToTrashList << url;
    }

    if(canNotMoveToTrashList.size() > 0){
        emit requestCanNotMoveToTrashDialogShowed(canNotMoveToTrashList);
    }else{
        list = doMove(files, DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath)));
    }

    if(m_isJobAdded)
        jobRemoved();

    emit finished();


    qDebug() << "Move to Trash is done!";
    return list;
}

bool FileJob::doTrashRestore(const QString &srcFilePath, const QString &tarFilePath)
{
//    qDebug() << srcFile << tarFile;
    qDebug() << "Do restore trash file is started";
    DUrlList files;
    files << QUrl::fromLocalFile(srcFilePath);
    m_totalSize = FileUtils::totalSize(files);
    jobPrepared();

    QStorageInfo srcStorageInfo = getStorageInfo(srcFilePath);
    QString tarDir = DUrl::fromLocalFile(tarFilePath).parentUrl().toLocalFile();
    QStorageInfo tarStorageInfo = getStorageInfo(tarDir);
    if (srcStorageInfo.rootPath() != tarStorageInfo.rootPath()){
        m_isInSameDisk = false;
    }

    bool ok = false;

    if (m_isInSameDisk){
        ok = restoreTrashFile(srcFilePath, tarFilePath);
    }else{

        QString _tarFilePath = tarFilePath;
        QFileInfo srcInfo(srcFilePath);

        if (srcInfo.isSymLink()){
            DUrlList urls;
            DUrl url =DUrl::fromLocalFile(srcFilePath);
            urls << url;
//            qDebug() << srcInfo.symLinkTarget() << DUrl::fromLocalFile(srcInfo.symLinkTarget()).parentUrl() << tarFilePath;
            const DUrlList &result = doMove(urls, DUrl::fromLocalFile(_tarFilePath).parentUrl());

            ok = !result.isEmpty();
        }else if (srcInfo.isDir()){
            if (copyDir(srcFilePath, tarDir, true, &_tarFilePath)) {
                deleteDir(srcFilePath);
                ok = QFile::rename(_tarFilePath, tarFilePath);
            }
        }else if (srcInfo.isFile() || srcInfo.isSymLink()){
            if (copyFile(srcFilePath, tarDir, true, &_tarFilePath)) {
                deleteFile(srcFilePath);
                ok = QFile::rename(_tarFilePath, tarFilePath);
            }
        }
    }

    if (ok) {
        QFile::remove(DFMStandardPaths::location(DFMStandardPaths::TrashInfosPath) + QDir::separator() + QFileInfo(srcFilePath).fileName() + ".trashinfo");
    }

    if(m_isJobAdded)
        jobRemoved();
    emit finished();
    qDebug() << "Do restore trash file is done!";

    return ok;
}

void FileJob::doOpticalBlank(const DUrl &device)
{
    QString dev = device.path();
    m_tarPath = dev;
    dev.replace("/dev/", "/org/freedesktop/UDisks2/block_devices/");
    QScopedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(dev));
    blkdev->unmount({});
    m_opticalOpSpeed.clear();
    jobPrepared();

    DISOMasterNS::DISOMaster *job_isomaster = new DISOMasterNS::DISOMaster(this);
    connect(job_isomaster, &DISOMasterNS::DISOMaster::jobStatusChanged, this, std::bind(&FileJob::opticalJobUpdated, this, job_isomaster, std::placeholders::_1, std::placeholders::_2));
    job_isomaster->acquireDevice(device.path());
    job_isomaster->erase();
    job_isomaster->releaseDevice();

    blkdev->rescan({});
    ISOMaster->nullifyDevicePropertyCache(device.path());

    if (m_isJobAdded)
        jobRemoved();
    emit finished();
    delete job_isomaster;
}

/*
 * flag:
 * 1: close session?
 * 2: eject?
 * 4: check media?
 */
void FileJob::doOpticalBurn(const DUrl &device, QString volname, int speed, int flag)
{
    QString dev = device.path();
    m_tarPath = dev;
    dev.replace("/dev/", "/org/freedesktop/UDisks2/block_devices/");
    QScopedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(dev));
    blkdev->unmount({});
    m_opticalJobPhase = 1;
    m_opticalOpSpeed.clear();
    jobPrepared();

    DISOMasterNS::DISOMaster *job_isomaster = new DISOMasterNS::DISOMaster(this);
    connect(job_isomaster, &DISOMasterNS::DISOMaster::jobStatusChanged, this, std::bind(&FileJob::opticalJobUpdated, this, job_isomaster, std::placeholders::_1, std::placeholders::_2));
    job_isomaster->acquireDevice(device.path());
    job_isomaster->getDeviceProperty();
    QUrl stagingurl(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                    + "/diskburn/" + device.path().replace('/','_') + "/");
    job_isomaster->stageFiles({{stagingurl, QUrl("/")}});
    job_isomaster->commit(speed, flag & 1, volname);
    double gud, slo, bad;
    if (flag & 4) {
        m_opticalJobPhase = 2;
        job_isomaster->checkmedia(&gud, &slo, &bad);
    }
    bool rst = ! ((flag & 4) && bad > 1e-6);
    job_isomaster->releaseDevice();

    if (flag & 2) {
        QScopedPointer<DDiskDevice> diskdev(DDiskManager::createDiskDevice(blkdev->drive()));
        diskdev->eject({});
    } else {
        blkdev->rescan({});
        ISOMaster->nullifyDevicePropertyCache(device.path());
    }

    doDelete({DUrl::fromLocalFile(stagingurl.path())});

    if (m_isJobAdded)
        jobRemoved();
    emit finished();
    if (m_opticalJobStatus == DISOMasterNS::DISOMaster::JobStatus::Finished) {
        if (flag & 4) {
            emit requestOpticalJobCompletionDialog(rst ? tr("Data verification successful.") : tr("Data verification failed."), rst ? "dialog-ok" : "dialog-error");
        } else {
            emit requestOpticalJobCompletionDialog(tr("Burn process completed"), "dialog-ok");
        }
    }
    delete job_isomaster;
}

/*
 * flag:
 * 1: unused
 * 2: eject?
 * 4: check media?
 */
void FileJob::doOpticalImageBurn(const DUrl &device, const DUrl &image, int speed, int flag)
{
    QString dev = device.path();
    m_tarPath = dev;
    dev.replace("/dev/", "/org/freedesktop/UDisks2/block_devices/");
    QScopedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(dev));
    blkdev->unmount({});
    m_opticalJobPhase = 0;
    m_opticalOpSpeed.clear();
    jobPrepared();

    DISOMasterNS::DISOMaster *job_isomaster = new DISOMasterNS::DISOMaster(this);
    connect(job_isomaster, &DISOMasterNS::DISOMaster::jobStatusChanged, this, std::bind(&FileJob::opticalJobUpdated, this, job_isomaster, std::placeholders::_1, std::placeholders::_2));
    job_isomaster->acquireDevice(device.path());
    DISOMasterNS::DeviceProperty dp = job_isomaster->getDeviceProperty();
    if (dp.formatted) {
        m_opticalJobPhase = 1;
    }
    job_isomaster->writeISO(image, speed);
    double gud, slo, bad;
    if (flag & 4) {
        m_opticalJobPhase = 2;
        job_isomaster->checkmedia(&gud, &slo, &bad);
    }
    bool rst = ! ((flag & 4) && bad > 1e-6);
    job_isomaster->releaseDevice();

    if (flag & 2) {
        QScopedPointer<DDiskDevice> diskdev(DDiskManager::createDiskDevice(blkdev->drive()));
        diskdev->eject({});
    } else {
        blkdev->rescan({});
        ISOMaster->nullifyDevicePropertyCache(device.path());
    }

    if (m_isJobAdded)
        jobRemoved();
    emit finished();
    if (m_opticalJobStatus == DISOMasterNS::DISOMaster::JobStatus::Finished) {
        if (flag & 4) {
        emit requestOpticalJobCompletionDialog(rst ? tr("Data verification successful.") : tr("Data verification failed."), rst ? "dialog-ok" : "dialog-error");
        } else {
        emit requestOpticalJobCompletionDialog(tr("Burn process completed"), "dialog-ok");
        }
    }
    delete job_isomaster;
}

void FileJob::opticalJobUpdated(DISOMasterNS::DISOMaster *jobisom, int status, int progress)
{
    if (status == DISOMasterNS::DISOMaster::JobStatus::Failed) {
        QStringList msg = jobisom->getInfoMessages();
        emit requestOpticalJobFailureDialog(m_jobType, FileJob::getXorrisoErrorMsg(msg), msg);
        return;
    }
    if (m_jobType == JobType::OpticalImageBurn && m_opticalJobStatus == DISOMasterNS::DISOMaster::JobStatus::Finished
        && status != DISOMasterNS::DISOMaster::JobStatus::Finished) {
        ++m_opticalJobPhase;
    }
    m_opticalJobStatus = status;
    m_opticalJobProgress = progress;
    if (status == DISOMasterNS::DISOMaster::JobStatus::Running) {
        m_opticalOpSpeed = jobisom->getCurrentSpeed();
    } else {
        m_opticalOpSpeed.clear();
    }
}

void FileJob::paused()
{
    m_status = FileJob::Paused;
}

void FileJob::started()
{
    m_status = FileJob::Started;
}

void FileJob::cancelled()
{
    g_cancellable_cancel(m_abortGCancellable);
    m_status = FileJob::Cancelled;
}

void FileJob::handleJobFinished()
{
    qDebug() << m_status;
    m_bytesCopied = m_totalSize;
    m_bytesPerSec = -1;
    m_isFinished = true;
    jobUpdated();
}

void FileJob::jobUpdated()
{

    if (m_status == FileJob::Paused){
        return;
    }

    if (m_isCheckingDisk){
        emit requestJobDataUpdated(m_jobDetail, m_checkDiskJobDataDetail);
    }

    QMap<QString, QString> jobDataDetail;

    if (m_jobType >= OpticalBurn && m_jobType <= OpticalImageBurn) {
        jobDataDetail["optical_op_type"] = QString::number(m_jobType);
        jobDataDetail["optical_op_status"] = QString::number(m_opticalJobStatus);
        jobDataDetail["optical_op_progress"] = QString::number(m_opticalJobProgress);
        jobDataDetail["optical_op_phase"] = QString::number(m_opticalJobPhase);
        jobDataDetail["optical_op_speed"] = m_opticalOpSpeed;
        jobDataDetail["optical_op_dest"] = m_tarPath;
    }
    else if (m_jobType == Restore && m_isInSameDisk){
        jobDataDetail.insert("file", m_srcFileName);
        jobDataDetail.insert("destination", m_tarDirName);
        if (!m_isFinished){
            if (m_status == Run){
                jobDataDetail.insert("file", m_srcFileName);
                jobDataDetail.insert("destination", m_tarDirName);
                jobDataDetail.insert("status", "restoring");
            }else{
                return;
            }
        }else{
            if (m_status != Cancelled){
                jobDataDetail.insert("progress", "100");
            }
        }
    }else{
        if (!m_isFinished){

            qint64 currentMsec = m_timer.elapsed();

            m_factor = (currentMsec - m_lastMsec) / 1000;

            if (m_factor <= 0)
                return;

            m_bytesPerSec /= m_factor;

            if (m_bytesPerSec == 0){
                return;
            }

            if(m_bytesPerSec > 0)
            {
                if (m_totalSize < m_bytesCopied){
                    qDebug() << "error copying file by growing" << m_totalSize << m_bytesCopied;
                    m_totalSize = m_bytesCopied;
                    cancelled();
                }else{
                    int remainTime = (m_totalSize - m_bytesCopied) / m_bytesPerSec;

                    if (remainTime < 60){
                        jobDataDetail.insert("remainTime", tr("%1 s").arg(QString::number(remainTime)));
                    }else if (remainTime >=60  && remainTime < 3600){
                        int min = remainTime / 60;
                        int second = remainTime % 60;
                        jobDataDetail.insert("remainTime", tr("%1 m %2 s").arg(QString::number(min),
                                                                                    QString::number(second)));
                    }else if (remainTime >=3600  && remainTime < 86400){
                        int hour = remainTime / 3600;
                        int min = (remainTime % 3600) / 60;
                        int second = (remainTime % 3600) % 60;
                        jobDataDetail.insert("remainTime", tr("%1 h %2 m %3 s").arg(QString::number(hour),
                                                                                         QString::number(min),
                                                                                         QString::number(second)));
                    }else{
                        int day = remainTime / 86400;
                        int left = remainTime % 86400;
                        int hour = left / 3600;
                        int min = (left % 3600) / 60;
                        int second = (left % 3600) % 60;
                        jobDataDetail.insert("remainTime", tr("%1 d %2 h %3 m %4 s").arg(QString::number(day),
                                                                                              QString::number(hour),
                                                                                              QString::number(min),
                                                                                              QString::number(second)));
                    }
                }
            }
        }
        QString speed;

        if (m_bytesCopied == m_totalSize){
            speed = QString("0 MB/s");
        }else{
            if(m_bytesPerSec > ONE_MB_SIZE)
            {
                m_bytesPerSec = m_bytesPerSec / ONE_MB_SIZE;
                speed = QString("%1 MB/s").arg(QString::number(m_bytesPerSec));
            }
            else
            {
                m_bytesPerSec = m_bytesPerSec / ONE_KB_SIZE;
                speed = QString("%1 KB/s").arg(QString::number(m_bytesPerSec));
            }
        }

        jobDataDetail.insert("speed", speed);
        jobDataDetail.insert("file", m_srcFileName);
        jobDataDetail.insert("progress", QString::number(m_bytesCopied * 100/ m_totalSize));
        jobDataDetail.insert("destination", m_tarDirName);
        m_progress = jobDataDetail.value("progress");
    }
//    qDebug() << m_jobDetail << jobDataDetail;
    emit requestJobDataUpdated(m_jobDetail, jobDataDetail);

    m_lastMsec = m_timer.elapsed();
    m_bytesPerSec = 0;
}

void FileJob::jobAdded()
{
    if(m_isJobAdded)
        return;
    emit requestJobAdded(m_jobDetail);
    m_isJobAdded = true;
}

void FileJob::jobRemoved()
{
    emit requestJobRemoved(m_jobDetail);
}

void FileJob::jobAborted()
{
    m_isAborted = true;
    emit requestAbortTask(m_jobDetail);
}

void FileJob::jobPrepared()
{
    m_bytesCopied = 0;
    m_bytesPerSec = 0;
    m_isFinished = false;
    m_timer.start();
    m_lastMsec = m_timer.elapsed();
}

void FileJob::jobConflicted()
{
    jobAdded();
    QMap<QString, QString> jobDataDetail;
    jobDataDetail.insert("remainTime", "");
    jobDataDetail.insert("speed", "");
    jobDataDetail.insert("file", m_srcFileName);
    jobDataDetail.insert("progress", m_progress);
    jobDataDetail.insert("destination", m_tarDirName);
    jobDataDetail.insert("sourcePath",m_srcPath);
    jobDataDetail.insert("targetPath", m_tarPath);
    jobDataDetail.insert("status", "conflict");

    emit requestJobDataUpdated(m_jobDetail, jobDataDetail);
    emit requestConflictDialogShowed(m_jobDetail);
    m_status = Paused;
}

bool FileJob::getIsFinished() const
{
    return m_isFinished;
}

bool FileJob::getIsGvfsFileOperationUsed() const
{
    return m_isGvfsFileOperationUsed;
}

int FileJob::getWindowId()
{
    return m_windowId;
}

void FileJob::setWindowId(int windowId)
{
    m_windowId = windowId;
}

bool FileJob::getIsCoExisted() const
{
    return m_isCoExisted;
}

void FileJob::setIsCoExisted(bool isCoExisted)
{
    m_isCoExisted = isCoExisted;
}

bool FileJob::getIsSkip() const
{
    return m_isSkip;
}

void FileJob::setIsSkip(bool isSkip)
{
    m_isSkip = isSkip;
}
bool FileJob::isAborted() const
{
    return m_isAborted;
}

void FileJob::setIsAborted(bool isAborted)
{
    m_isAborted = isAborted;
}

bool FileJob::copyFile(const QString &srcFile, const QString &tarDir, bool isMoved, QString *targetPath)
{
    if (m_isGvfsFileOperationUsed) {
        QString targetFile;

        bool ok = copyFileByGio(srcFile, tarDir, isMoved, &targetFile);

        if (targetPath)
            *targetPath = targetFile;

        return ok;
    }
#ifdef SW_LABEL
    if (LlsDeepinLabelLibrary::instance()->isCompletion()){
        QString tarFile = tarDir + "/" + QFileInfo(srcFile).fileName();
        bool isSrcLabelFileFlag = isLabelFile(srcFile);
        bool isDstLabelFileFlag = isLabelFile(srcFile);
        qDebug() << "srcFile: " << srcFile;
        qDebug() << "tarFile: "<< tarFile;
        qDebug() << "isSrcLabelFileFlag: " << isSrcLabelFileFlag;
        qDebug() << "isDstLabelFileFlag: " << isDstLabelFileFlag;
        qDebug() << tarDir << deviceListener->isInRemovableDeviceFolder(tarDir);
        int nRet = 0;
        if (deviceListener->isInRemovableDeviceFolder(tarDir)){
            nRet = checkStoreInRemovableDiskPrivilege(srcFile, tarFile);
            if (nRet != 0){
                emit fileSignalManager->jobFailed(nRet, QString(QMetaEnum::fromType<JobType>().valueToKey(m_jobType)), srcFile);
                return false;
            }
        }else{
            nRet = checkCopyJobPrivilege(srcFile, tarFile);
            if (nRet != 0){
                emit fileSignalManager->jobFailed(nRet, QString(QMetaEnum::fromType<JobType>().valueToKey(m_jobType)), srcFile);
                return false;
            }
        }
    }
#endif


    if (checkFat32FileOutof4G(srcFile, tarDir))
        return false;

    if (m_isAborted)
        return false;
    if(m_applyToAll && m_status == FileJob::Cancelled){
        m_skipandApplyToAll = true;
    }else if(!m_applyToAll && m_status == FileJob::Cancelled){
        m_status = Started;
    }
    QFileInfo srcFileInfo(srcFile);
    QFileInfo tarDirInfo(tarDir);
    m_srcFileName = srcFileInfo.fileName();
    m_tarDirName = tarDirInfo.fileName();
    m_srcPath = srcFile;

    if (m_jobType != Trash && m_jobType != Restore){
        m_tarPath = tarDir + "/" + m_srcFileName;
    }else{
        if (targetPath){
            m_tarPath = *targetPath;
        }else{
            m_tarPath = tarDir + "/" + m_srcFileName;
        }
    }
    QFile from(srcFile);
    QFile to(m_tarPath);
    QFileInfo targetInfo(m_tarPath);
    m_status = Started;

    //We only check the conflict of the files when
    //they are not in the same folder

    bool isTargetExists = targetInfo.exists();

    if(srcFileInfo.absolutePath() != targetInfo.absolutePath()){
        if(isTargetExists && !m_applyToAll)
        {
            jobConflicted();
        }else if (isTargetExists && m_skipandApplyToAll){
            return false;
        }else{
            m_isSkip = false;
        }
    }

#ifdef SPLICE_CP
    loff_t in_off = 0;
    loff_t out_off = 0;
    int buf_size = 16 * 1024;
    qint64 len = 0;
    ssize_t err = -1;
    int in_fd = 0;
    int out_fd = 0;
#else
    if (!m_bufferAlign){
        m_buffer = (char *) malloc(Data_Block_Size + getpagesize());
        m_bufferAlign = ptr_align(m_buffer, getpagesize());
    }
#endif

    while(true)
    {
        switch(m_status)
        {
            case FileJob::Started:
            {
                if (isTargetExists){

                    if (m_isSkip){

                        if(!m_applyToAll)
                            m_isSkip = false;

                        return false;
                    }

                    if(m_isCoExisted && !m_isReplaced)
                    {
                        m_tarPath = checkDuplicateName(m_tarPath);
                        to.setFileName(m_tarPath);
                        if(!m_applyToAll)
                            m_isCoExisted = true;
                    }

                    if (m_isReplaced){

                        if (targetInfo.isSymLink()){
                            QFile(m_tarPath).remove();
                        }else if (!targetInfo.isSymLink() && targetInfo.isDir()){
                            QDir(m_tarPath).removeRecursively();
                        }else if (!targetInfo.isSymLink() && targetInfo.isFile()){
                            QFile(m_tarPath).remove();
                        }

                        if(!m_applyToAll)
                            m_isReplaced = false;

                        // Clean saved events
                        DFMEventDispatcher::instance()->processEvent<DFMCleanSaveOperatorEvent>(nullptr);
                    }
                }

                if(!from.open(QIODevice::ReadOnly))
                {
                    //Operation failed
                    qDebug() << srcFile << "isn't read only";
                    return false;
                }

                if(!to.open(QIODevice::WriteOnly))
                {
                    //Operation failed
                    qDebug() << to.fileName() << to.error() << to.errorString() << "isn't write only";
                    QString toFileName =  tarDir + "/" + DUrl::toPercentEncoding(m_srcFileName);
                    qDebug() << "toPercentEncoding" <<  toFileName;
                    to.setFileName(toFileName);
                    if (!to.open(QIODevice::WriteOnly)){
                        qDebug() << to.fileName() << to.error() <<to.errorString() << "isn't write only";
                        return false;
                    }
                }
                posix_fadvise (from.handle(), 0, 0, POSIX_FADV_SEQUENTIAL);

                CopyingFiles.append(DUrl::fromLocalFile(m_tarPath));

                m_status = Run;
 #ifdef SPLICE_CP
                in_fd = from.handle();
                out_fd = to.handle();
                len = srcFileInfo.size();
                posix_fadvise(from.handle(), 0, len, POSIX_FADV_SEQUENTIAL);
                posix_fadvise(from.handle(), 0, len, POSIX_FADV_WILLNEED);
 #endif
                break;
            }
            case FileJob::Run:
            {

#ifdef SPLICE_CP
                if(len <= 0)
                {
                    if ((m_totalSize - m_bytesCopied) <= 1){
                        m_bytesCopied = m_totalSize;
                    }

                    to.flush();
//                    fsync(out_fd);
                    from.close();
                    to.close();

                    if (targetPath)
                        *targetPath = m_tarPath;

                    return true;
                }

                if(buf_size > len)
                    buf_size = len;
                /*
                 * move to pipe buffer.
                 */
                err = splice(in_fd, &in_off, m_filedes[1], NULL, buf_size, SPLICE_F_MOVE);
                if(err < 0) {
                    qDebug() << "splice pipe0 fail";
                    return false;
                }

                if (err < buf_size) {
                    qDebug() << QString("copy ret %1 (need %2)").arg(err, buf_size);
                    buf_size = err;
                }
                /*
                 * move from pipe buffer to out_fd
                 */
                err = splice(m_filedes[0], NULL, out_fd, &out_off, buf_size, SPLICE_F_MOVE );
                if(err < 0) {
                    qDebug() << "splice pipe1 fail";
                    return false;
                }
                len -= buf_size;

                m_bytesCopied += buf_size;
                m_bytesPerSec += buf_size;

                if (!m_isInSameDisk){
                    if (m_bytesCopied % (Data_Flush_Size) == 0){
                        fsync(out_fd);
                    }
                }
#else
                qint64 inBytes = from.read(m_bufferAlign, Data_Block_Size);

                if(inBytes == 0)
                {
                    if ((m_totalSize - m_bytesCopied) <= 1){
                        m_bytesCopied = m_totalSize;
                    }
                    to.close();
                    from.close();

                    if (!to.setPermissions(from.permissions())){
                        qDebug() << "Set permissions from " << srcFile << "to" << m_tarPath << "failed";
                    }

                    if (targetPath){
                        *targetPath = m_tarPath;
                    }
                    return true;
                }else if (inBytes == -1 && from.error() == QFileDevice::ReadError){
                    to.close();
                    from.close();
                    if (targetPath){
                        *targetPath = m_tarPath;
                    }
                    return false;
                }

                qint64 availableBytes = inBytes;

                while (true) {
                    qint64 writtenBytes = to.write(m_bufferAlign, availableBytes);
                    availableBytes = availableBytes - writtenBytes;
                    if (writtenBytes == 0 && availableBytes == 0){
                        break;
                    }
                }

                m_bytesCopied += inBytes;
                m_bytesPerSec += inBytes;
#endif
                break;
            }
            case FileJob::Paused:
                QThread::msleep(100);
                m_lastMsec = m_timer.elapsed();
                break;
            case FileJob::Cancelled:
                from.close();
                to.close();
                if (m_isSkip)
                    return true;
                else
                    return false;
            default:
                from.close();
                to.close();
                return false;
         }

    }
    return false;
}

void FileJob::showProgress(goffset current_num_bytes, goffset total_num_bytes, gpointer user_data)
{
    Q_UNUSED(total_num_bytes)

    FileJob* job = static_cast<FileJob*>(user_data);
//    qDebug() << current_num_bytes << total_num_bytes;
    qint64 writtenBytes = current_num_bytes - job->m_last_current_num_bytes;
    job->m_bytesPerSec += writtenBytes;
    job->m_bytesCopied += writtenBytes;
    job->m_last_current_num_bytes = current_num_bytes;

    // 使用gio往远程挂载设备中复制文件时没有notify
    // 需要手动更新文件信息
    if (job->m_isGvfsFileOperationUsed) {
        if (job->m_needGhostFileCreateSignal) {
            DAbstractFileWatcher::ghostSignal(DUrl::fromLocalFile(QFileInfo(job->m_tarPath).absolutePath()),
                                              &DAbstractFileWatcher::subfileCreated, DUrl::fromLocalFile(job->m_tarPath));
            job->m_needGhostFileCreateSignal = false;
        }

        DAbstractFileWatcher::ghostSignal(DUrl::fromLocalFile(QFileInfo(job->m_tarPath).absolutePath()),
                                          &DAbstractFileWatcher::fileModified, DUrl::fromLocalFile(job->m_tarPath));
    }
}

bool FileJob::copyFileByGio(const QString &srcFile, const QString &tarDir, bool isMoved, QString *targetPath)
{
    Q_UNUSED(isMoved);
//    qDebug() << "copy file by gvfs" << srcFile << tarDir;

    if (checkFat32FileOutof4G(srcFile, tarDir))
        return false;

    if (m_isAborted)
        return false;
    if(m_applyToAll && m_status == FileJob::Cancelled){
        m_skipandApplyToAll = true;
    }else if(!m_applyToAll && m_status == FileJob::Cancelled){
        m_status = Started;
    }

    QFileInfo srcFileInfo(srcFile);
    QFileInfo tarDirInfo(tarDir);
    m_srcFileName = srcFileInfo.fileName();
    m_tarDirName = tarDirInfo.fileName();
    m_srcPath = srcFile;
    m_tarPath = tarDir + "/" + m_srcFileName;

    QFileInfo targetInfo(m_tarPath);
    m_status = Started;

    //We only check the conflict of the files when
    //they are not in the same folder
    bool isTargetExists = targetInfo.exists();

    if(srcFileInfo.absolutePath() != targetInfo.absolutePath()){
        if(isTargetExists && !m_applyToAll)
        {
            jobConflicted();
        }else if (isTargetExists && m_skipandApplyToAll){
            return false;
        }else{
            m_isSkip = false;
        }
    }

    GError *error;
    GFile *source = NULL, *target=NULL;
    GFileCopyFlags flags;

    flags = static_cast<GFileCopyFlags>(G_FILE_COPY_NONE | G_FILE_COPY_ALL_METADATA);
    error = NULL;
    bool result = false;
    while(true)
    {
        switch(m_status)
        {
            case FileJob::Started:
            {
                if (isTargetExists){

                    if (m_isSkip){
                        if(!m_applyToAll)
                            m_isSkip = false;
                        return true;
                    }

                    if(m_isCoExisted && !m_isReplaced)
                    {
                        m_tarPath = checkDuplicateName(m_tarPath);
                        if(!m_applyToAll)
                            m_isCoExisted = true;
                    }

                    if (m_isReplaced){

                        if (targetInfo.isSymLink()){
                            QFile(m_tarPath).remove();
                        }else if (!targetInfo.isSymLink() && targetInfo.isDir()){
                            QDir(m_tarPath).removeRecursively();
                        }else{
                            flags = static_cast<GFileCopyFlags>(flags | G_FILE_COPY_OVERWRITE);
                        }

                        if(!m_applyToAll)
                            m_isReplaced = false;
                    }
                }

                std::string std_srcPath = m_srcPath.toStdString();
                source = g_file_new_for_path(std_srcPath.data());

                std::string std_tarPath = m_tarPath.toStdString();
                target = g_file_new_for_path(std_tarPath.data());

                m_last_current_num_bytes = 0;

                m_status = Run;
                break;
            }
            case FileJob::Run:
            {
                GFileProgressCallback progress_callback = FileJob::showProgress;

                // 需要模拟通知文件创建的信号
                m_needGhostFileCreateSignal = true;
                g_cancellable_reset(m_abortGCancellable);

                if (!g_file_copy (source, target, flags, m_abortGCancellable, progress_callback, this, &error)){
                    if (error){
                        qDebug() << error->message << g_file_error_from_errno(error->domain);
                        if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED)){
                            m_noPermissonUrls << DUrl::fromLocalFile(srcFile);
                        } else if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
                            emit fileSignalManager->requestShowErrorDialog(QString::fromUtf8(error->message), QString(" "));
                        }
                        g_error_free (error);
                    }
                    result = false;
                    continue;
                }else{
                    m_last_current_num_bytes = 0;
                    if (error && IS_IO_ERROR (error, CANCELLED)) {
                        qDebug() << error->message;
                        g_error_free (error);
                    }
                    if (targetPath)
                        *targetPath = m_tarPath;
                    result = true;
                }
                goto unref;
            }
            case FileJob::Paused:
                QThread::msleep(100);
                m_lastMsec = m_timer.elapsed();
                break;
            case FileJob::Cancelled:
                goto unref;
            default:
                goto unref;
         }

    }
unref:
    if (source)
        g_object_unref (source);
    if (target)
        g_object_unref (target);
    return result;
}

bool FileJob::copyDir(const QString &srcDir, const QString &tarDir, bool isMoved, QString *targetPath)
{
    DUrl srcUrl(srcDir);
    DUrl tarUrl(tarDir);
    qDebug() << srcUrl<< tarUrl << isMoved;
    if(DUrl::childrenList(tarUrl).contains(srcUrl) && !isMoved){
        emit requestCopyMoveToSelfDialogShowed(m_jobDetail);
        return false;
    }

    if (m_isAborted)
        return false;

    if(m_applyToAll && m_status == FileJob::Cancelled){
        m_skipandApplyToAll = true;
    }else if(!m_applyToAll && m_status == FileJob::Cancelled){
        m_status = Started;
    }
    QDir sourceDir(srcDir);
    QDir targetDir(tarDir);
    QFileInfo srcDirInfo(srcDir);
    m_srcPath = srcDir;
    m_srcFileName = srcDirInfo.fileName();
    m_tarDirName = targetDir.dirName();

    if (m_jobType != Trash){
        m_tarPath = tarDir + "/" + m_srcFileName;
    }else{
        if (targetPath){
            m_tarPath = *targetPath;
        }else{
            m_tarPath = tarDir + "/" + m_srcFileName;
        }
    }
    QFileInfo targetInfo(m_tarPath);

    m_status = Started;

    bool isTargetExists = targetInfo.exists();

    if(srcDirInfo.absolutePath() != targetInfo.absolutePath()){
        if(isTargetExists && !m_applyToAll)
        {
            jobConflicted();
        }else if (isTargetExists && m_skipandApplyToAll){
            return false;
        }else{
            m_isSkip = false;
        }
    }

    while(true)
    {
        switch(m_status)
        {
        case Started:
        {
            if (isTargetExists){

                if (m_isSkip){

                    if(!m_applyToAll)
                        m_isSkip = false;

                    return true;
                }

                if(m_isCoExisted && !m_isReplaced)
                {
                    m_tarPath = checkDuplicateName(m_tarPath);
                    isTargetExists = false;

                    if(!m_applyToAll)
                        m_isCoExisted = true;
                }

                if (m_isReplaced){
                    if (targetInfo.isSymLink()){
                        QFile(m_tarPath).remove();
                    }else if (!targetInfo.isSymLink() && targetInfo.isFile()){
                        QFile(m_tarPath).remove();
                    }else if (!targetInfo.isSymLink() && targetInfo.isDir()){
//                        if (!deleteDir(m_tarPath)) {
//                            QProcess::execute("rm -r \"" + m_tarPath.toUtf8() + "\"");
//                        }
                    }

                    targetDir.mkdir(m_tarPath);

                    if(!m_applyToAll)
                        m_isReplaced = false;
                }
            }

            if(!isTargetExists)
            {
                if(!targetDir.mkdir(m_tarPath))
                    return false;
            }
            targetDir.setPath(m_tarPath);

            m_status = Run;
            break;
        }
        case Run:
        {

            char *name_space;
            char *namep;
            name_space = savedir(srcDir.toStdString().data());
            namep = name_space;

            // 打开目录失败则跳过此目录
            if (!namep)
                return false;

            while (*namep != '\0')
            {
                QString srcFile = QString("%1/%2").arg(srcDir, QString(namep));
                QFileInfo srcFileInfo(srcFile);
                if (srcFileInfo.isSymLink()){
                    handleSymlinkFile(srcFile, targetDir.absolutePath());
                }else if(!srcFileInfo.isSymLink() && srcFileInfo.isDir()){
                    if(!copyDir(srcFile, targetDir.absolutePath(), isMoved)){
                        qDebug() << "coye dir" << srcFile << "failed";
                    }
                }else
                {
                    if(!copyFile(srcFile, targetDir.absolutePath(), isMoved))
                    {
                        qDebug() << "coye file" << srcFile << "failed";
                    }
                }
                namep += strlen (namep) + 1;
            }
           free (name_space);

            if (targetPath)
                *targetPath = targetDir.absolutePath();

            if (!FileUtils::isGvfsMountFile(targetDir.path())){
                bool isSetPermissionsSuccess = setDirPermissions(srcDir, targetDir.path());
                if (!isSetPermissionsSuccess){
                    qWarning() << "Set Permissions of "<< m_tarPath << "same as" <<  srcDir << "failed";
                }
            }

            return true;
        }
        case Paused:
            QThread::msleep(100);
            break;
        case Cancelled:
            return false;
        default: break;
        }
    }

    if (targetPath)
        *targetPath = targetDir.absolutePath();

    return true;
}

bool FileJob::moveFile(const QString &srcFile, const QString &tarDir, QString *targetPath)
{
    /*use fuse to moveFile file if file is samba/mtp/afc which can be monitor by inotify*/
//    if (m_isGvfsFileOperationUsed){
//        return moveFileByGio(srcFile, tarDir, targetPath);
//    }else{
//        qDebug() << "move file by qtio" << srcFile << tarDir;
//    }

#ifdef SW_LABEL
    if (LlsDeepinLabelLibrary::instance()->isCompletion()){
        QString tarFile = tarDir + "/" + QFileInfo(srcFile).fileName();
        bool isSrcLabelFileFlag = isLabelFile(srcFile);
        bool isDstLabelFileFlag = isLabelFile(tarFile);
        qDebug() << "srcFile: " << srcFile;
        qDebug() << "tarFile: "<< tarFile;
        qDebug() << "isSrcLabelFileFlag: " << isSrcLabelFileFlag;
        qDebug() << "isDstLabelFileFlag: " << isDstLabelFileFlag;
        qDebug() << tarDir << deviceListener->isInRemovableDeviceFolder(tarDir);
        int nRet = 0;
        if (deviceListener->isInRemovableDeviceFolder(tarDir)){
            nRet = checkStoreInRemovableDiskPrivilege(srcFile, tarFile);
            if (nRet != 0){
                emit fileSignalManager->jobFailed(nRet, QString(QMetaEnum::fromType<JobType>().valueToKey(m_jobType)), srcFile  );
                return false;
            }
        }else{
            nRet = checkMoveJobPrivilege(srcFile, tarFile);
            if (nRet != 0){
                emit fileSignalManager->jobFailed(nRet, QString(QMetaEnum::fromType<JobType>().valueToKey(m_jobType)), srcFile);
                return false;
            }
        }
    }
#endif

//    qDebug() << "moveFile start:" << srcFile << tarDir << targetPath;
    bool ret = handleMoveJob(srcFile, tarDir, targetPath);
//    qDebug() << "moveFile end:" << srcFile << tarDir << ret << targetPath;
    return ret;
}

bool FileJob::moveFileByGio(const QString &srcFile, const QString &tarDir, QString *targetPath)
{
//    qDebug() << "move file by gvfs" << srcFile << tarDir;
    QString srcPath(srcFile);

    QFileInfo scrFileInfo(srcPath);

    if (scrFileInfo.isDir() && scrFileInfo.exists()){
        DUrl srcUrl(srcPath);
        DUrl tarUrl(tarDir);
        if(DUrl::childrenList(tarUrl).contains(srcUrl)){
            emit requestCopyMoveToSelfDialogShowed(m_jobDetail);
            /*copyDir/deleteDir will excute if return false;*/
            return true;
        }
    }

    if (m_isAborted)
        return false;

    if(m_applyToAll && m_status == FileJob::Cancelled){
        m_skipandApplyToAll = true;
    }else if(!m_applyToAll && m_status == FileJob::Cancelled){
        m_status = Started;
    }

    QDir to(tarDir);
    m_srcFileName = scrFileInfo.fileName();
    m_tarDirName = to.dirName();
    m_srcPath = srcPath;
    m_tarPath = tarDir;
    m_status = Started;

    bool isTargetExists = to.exists(m_srcFileName);

    //We only check the conflict of the files when
    //they are not in the same folder
    if(scrFileInfo.absolutePath()== tarDir && isTargetExists)
        return true;
    else{
        if(isTargetExists && !m_applyToAll)
        {
            jobConflicted();
        }else if (isTargetExists && m_skipandApplyToAll){
            return false;
        }else{
            m_isSkip = false;
        }
    }

    GError *error;
    GFile *source = NULL, *target=NULL;
    GFileCopyFlags flags;

    flags = static_cast<GFileCopyFlags>(G_FILE_COPY_NONE | G_FILE_COPY_ALL_METADATA);
    error = NULL;

    bool result = false;

    while(true)
    {
        switch(m_status)
        {
            case FileJob::Started:
            {
                if (isTargetExists){

                    if (m_isSkip){

                        if(!m_applyToAll)
                            m_isSkip = false;

                        return true;
                    }

                    if(m_isCoExisted && !m_isReplaced)
                    {
                        m_tarPath = checkDuplicateName(m_tarPath + "/" + m_srcFileName);

                        if(!m_applyToAll)
                            m_isCoExisted = true;
                    }

                    if (m_isReplaced){

                        m_tarPath = m_tarPath + "/" + m_srcFileName;

                        flags = static_cast<GFileCopyFlags>(flags | G_FILE_COPY_OVERWRITE);

                        if(!m_applyToAll)
                            m_isReplaced = false;
                    }
                }

                std::string std_srcPath = m_srcPath.toStdString();
                source = g_file_new_for_path(std_srcPath.data());

                std::string std_tarPath = m_tarPath.toStdString();
                target = g_file_new_for_path(std_tarPath.data());

                m_last_current_num_bytes = 0;


                m_status = Run;
                break;
            }
            case FileJob::Run:
            {
                GFileProgressCallback progress_callback = FileJob::showProgress;
                g_cancellable_reset(m_abortGCancellable);

                if (!g_file_move (source, target, flags, m_abortGCancellable, progress_callback, this, &error)){
                    if (error){
                        qDebug() << error->message << g_file_error_from_errno(error->domain);
                        if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED)){
                            m_noPermissonUrls << DUrl::fromLocalFile(srcFile);
                        } else if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
                            emit fileSignalManager->requestShowErrorDialog(QString::fromUtf8(error->message), QString(" "));
                        }
                        g_error_free (error);
                    }
                    result = true;
                }else{
                    m_last_current_num_bytes = 0;
                    if (error && IS_IO_ERROR (error, CANCELLED)) {
                        qDebug() << error->message;
                        g_error_free (error);
                    }
                    if (targetPath)
                        *targetPath = m_tarPath;
                    result = true;
                }
                goto unref;
            }
            case FileJob::Paused:
                QThread::msleep(100);
                break;
            case FileJob::Cancelled:
                goto unref;
            default:
                goto unref;
        }
    }
unref:
    if (source)
        g_object_unref (source);
    if (target)
        g_object_unref (target);
    return result;
}

bool FileJob::moveDir(const QString &srcDir, const QString &tarDir, QString *targetPath)
{
    return handleMoveJob(srcDir, tarDir, targetPath);
}

bool FileJob::handleMoveJob(const QString &srcPath, const QString &tarDir, QString *targetPath)
{
//    qDebug() << srcPath << tarDir;

    m_isSkip = false;

    QFileInfo scrFileInfo(srcPath);

    if (scrFileInfo.isDir() && scrFileInfo.exists()){
        DUrl srcUrl(srcPath);
        DUrl tarUrl(tarDir);
        if(DUrl::childrenList(tarUrl).contains(srcUrl)){
            emit requestCopyMoveToSelfDialogShowed(m_jobDetail);
            /*copyDir/deleteDir will excute if return false;*/
            return true;
        }
    }

    if (m_isAborted)
        return false;

    if(m_applyToAll && m_status == FileJob::Cancelled){
        m_skipandApplyToAll = true;
    }else if(!m_applyToAll && m_status == FileJob::Cancelled){
        m_status = Started;
    }

    QDir to(tarDir);
    m_srcFileName = scrFileInfo.fileName();
    m_tarDirName = to.dirName();
    m_srcPath = srcPath;
    m_tarPath = tarDir + "/" + m_srcFileName;;
    m_status = Started;

    bool isTargetExists = to.exists(m_srcFileName);

    //We only check the conflict of the files when
    //they are not in the same folder
    if(scrFileInfo.absolutePath()== tarDir && isTargetExists)
        return true;
    else{
        if(isTargetExists && !m_applyToAll)
        {
            jobConflicted();
        }else if (isTargetExists && m_skipandApplyToAll){
            return false;
        }else{
            m_isSkip = false;
        }

    }

    while(true)
    {
        switch(m_status)
        {
            case FileJob::Started:
            {
                qDebug() << m_isSkip << m_isCoExisted << m_isReplaced << m_applyToAll;
                if (m_isSkip){

                    if(!m_applyToAll)
                        m_isSkip = false;

                    return true;
                }

                if(m_isCoExisted && !m_isReplaced){
                    m_tarPath = checkDuplicateName(m_tarPath);

                    if(!m_applyToAll)
                        m_isCoExisted = true;

                }

                if (m_isReplaced){

                    QFileInfo tarInfo(m_tarPath);
                    if (tarInfo.isDir()){
                        QDir tarDir(m_tarPath);
                        if(tarDir.exists()){

                        }else{
                            return false;
                        }
                    }else{
                        QFile tarFile(m_tarPath);
                        if(tarFile.exists() || QFileInfo(tarFile).isSymLink()){
                            QFile(m_tarPath).remove();
                        }
                    }

                    if(!m_applyToAll)
                        m_isReplaced = false;
                }

                m_status = Run;
                break;
            }
            case FileJob::Run:
            {
                bool ok(false);
                if (scrFileInfo.isDir()){
                    if (!QDir(m_tarPath).exists()){
                        qDebug() << srcPath << m_tarPath;
                        ok = QDir(srcPath).rename(srcPath, m_tarPath);
                    }else{
                        QDirIterator tmp_iterator(scrFileInfo.absoluteFilePath(),
                                                  QDir::AllEntries | QDir::System
                                                  | QDir::NoDotAndDotDot
                                                  | QDir::Hidden);

                        while (tmp_iterator.hasNext()) {

                            if (m_isAborted)
                                break;

                            tmp_iterator.next();
                            const QFileInfo fileInfo = tmp_iterator.fileInfo();
                            QString srcFilePath = fileInfo.filePath();
                            QString _targetDir = QFileInfo(m_tarPath).absoluteFilePath();
                            if (fileInfo.isSymLink()){
                                handleSymlinkFile(srcFilePath, _targetDir);
                            }else if(!fileInfo.isSymLink() && fileInfo.isDir()){
                                qDebug() << srcFilePath << _targetDir;
                                if(!moveDir(srcFilePath, _targetDir)){
                                    qDebug() << "move dir" << srcFilePath << _targetDir << "failed";
                                }else{
                                    m_tarPath = _targetDir;
                                }
                            }else
                            {
                                if(!moveFile(srcFilePath, _targetDir))
                                {
                                    qDebug() << "move file" << srcFilePath << _targetDir << "failed";
                                }else{
                                    m_tarPath = _targetDir;
                                }
                            }
                        }
                        ok = true;
                    }


                }else{
                    QFile srcFile(srcPath);
                    ok = srcFile.rename(m_tarPath);
                }

                if (ok && targetPath)
                    *targetPath = m_tarPath;

                return ok;
            }
            case FileJob::Paused:
                QThread::msleep(100);
                break;
            case FileJob::Cancelled:
            if (m_isSkip)
                return true;
            else
                return false;
            default:
                return false;
         }

    }
    return false;

}

bool FileJob::handleSymlinkFile(const QString &srcFile, const QString &tarDir, QString *targetPath)
{
    qDebug() << srcFile << tarDir;
    if (m_isAborted)
        return false;
    QDir to(tarDir);
    QFileInfo fromInfo(srcFile);
    m_srcFileName = fromInfo.fileName();
    m_tarDirName = to.dirName();
    m_srcPath = srcFile;
    m_tarPath = tarDir;
    m_status = Started;

    while(true)
    {
        switch(m_status)
        {
            case FileJob::Started:
            {
                if (m_jobType != Trash){
                    m_tarPath = checkDuplicateName(m_tarPath + "/" + m_srcFileName);
                }else{
                    bool canTrash = moveFileToTrash(srcFile, targetPath);
                    if (canTrash && targetPath){
                        m_tarPath = *targetPath;
                    }
                }
                m_status = Run;
                break;
            }
            case FileJob::Run:
            {

                QFile targetFile(fromInfo.symLinkTarget());
                bool ok = targetFile.link(m_tarPath);
                if (ok){
                    if (m_jobType == Move || m_jobType == Trash || m_jobType == Restore){
                        QFile from(srcFile);
                        from.remove();
                    }
                    if (targetPath){
                        *targetPath = m_tarPath;
                    }
                }else{
                    qDebug() << targetFile.errorString();
                }

                if(!m_applyToAll){
                    m_isReplaced = false;
                }

                return ok;
            }
            case FileJob::Paused:
                QThread::msleep(100);
                break;
            case FileJob::Cancelled:
                if (m_isSkip)
                    return true;
                else
                    return false;
            default:
                return false;
         }

    }
    return false;
}

bool FileJob::restoreTrashFile(const QString &srcFile, const QString &tarFile)
{
    QFile from(srcFile);
    QFile to(tarFile);

    QFileInfo toInfo(tarFile);
    m_srcFileName = toInfo.fileName();
    m_srcPath = srcFile;
    m_tarPath = toInfo.absoluteFilePath();
    m_tarDirName = toInfo.absoluteDir().dirName();
    m_status = Started;

    if(toInfo.exists() || toInfo.isSymLink())
    {
        jobConflicted();
    }

    while(true)
    {
        switch(m_status)
        {
            case FileJob::Started:
            {
                if (m_isSkip){
                    return true;
                }

                if(m_isCoExisted && !m_isReplaced)
                {
                    m_tarPath = checkDuplicateName(m_tarPath);
                }

                if (m_isReplaced){
                    if(to.exists() || toInfo.isSymLink()){
                        if (toInfo.isDir()){
//                            bool result = QDir(tarFile).removeRecursively();

//                            if (!result) {
//                                result = QProcess::execute("rm -r \"" + tarFile.toUtf8() + "\"") == 0;
//                            }

//                            if (!result)
//                                return false;
                        }else if (toInfo.isFile() || toInfo.isSymLink()){
                            to.remove();
//                            qDebug() << to.error() << to.errorString();
                        }
                    }
                }

                m_status = Run;
                break;
            }
            case FileJob::Run:
            {
                bool result(false);
                QFileInfo srcFileinfo(srcFile);
                if (srcFileinfo.isSymLink()){
                    result =  QFile(srcFileinfo.symLinkTarget()).link(m_tarPath);
                    if (result){
                        from.remove();
                    }
                }else if (srcFileinfo.isDir() && m_isReplaced){
                    QDir srcDir(srcFile);
                    QFileInfoList infos = srcDir.entryInfoList(QDir::AllEntries | QDir::System
                                     | QDir::NoDotAndDotDot
                                     | QDir::Hidden);
                    DUrlList urls, resultUrls;
                    foreach (const QFileInfo& info, infos) {
                        urls << DUrl::fromLocalFile(info.absoluteFilePath());
                    }
                    qDebug() << urls;
                    m_isReplaced = false;
                    resultUrls = doMove(urls, DUrl::fromLocalFile(tarFile));
                    m_isReplaced = true;
                    qDebug() << resultUrls;
                    if (resultUrls.count() == urls.count()){
                       result = QDir(srcFile).removeRecursively();
                        if (!result) {
                            result = QProcess::execute("rm -r \"" + srcFile.toUtf8() + "\"") == 0;
                        }
                        if (!result)
                            return false;
                    }

                }else{
                    result = from.rename(m_tarPath);
                }

                if (!result) {
                    if (srcFileinfo.isDir() && m_isReplaced){
                        return result;
                    }else{
                        qDebug() << m_tarPath << from.error() << from.errorString();
                        result = (QProcess::execute("mv -T \"" + from.fileName().toUtf8() + "\" \"" + m_srcPath.toUtf8() + "\"") == 0);
                        if (!result){
                            emit fileSignalManager->requestShowRestoreFailedPerssionDialog(srcFile, m_tarPath);
                        }
                    }
                }

                return result;
            }
            case FileJob::Paused:
                QThread::msleep(100);
                break;
            case FileJob::Cancelled:
                return true;
            default:
                return false;
         }

    }
    return false;
}

bool FileJob::deleteFile(const QString &file)
{
    /*use fuse to delete file if file is samba/mtp/afc which can be monitor by inotify*/
//    if (checkUseGvfsFileOperation(file)){
//        return deleteFileByGio(file);
//    }

#ifdef SW_LABEL
    if (LlsDeepinLabelLibrary::instance()->isCompletion()){
        if (isLabelFile(file)){
            int nRet = checkDeleteJobPrivilege(file);
            if (nRet != 0){
                emit fileSignalManager->jobFailed(nRet, QString(QMetaEnum::fromType<JobType>().valueToKey(m_jobType)), file);
                return false;
            }
        }
    }
#endif

//    qDebug() << "delete file by qtio" << file;

    if(QFile::remove(file)){
        return true;
    }
    else
    {
        qDebug() << "unable to delete file:" << file;
//        emit fileSignalManager->requestShowNoPermissionDialog(DUrl::fromLocalFile(file));
        m_noPermissonUrls << DUrl::fromLocalFile(file);
        return false;
    }
}

bool FileJob::deleteFileByGio(const QString &srcFile)
{
//    qDebug() << "delete file by gvfs" << srcFile;
    GFile *source;
    GError* error = NULL;

    std::string std_srcPath = srcFile.toStdString();
    source = g_file_new_for_path(std_srcPath.data());

    bool result = false;
    if (!g_file_delete (source, NULL, &error)){
        if (error){
            qDebug() << error->message;
            g_error_free (error);
        }
    }else{
        result = true;
    }
    if (source)
        g_object_unref (source);
    return result;
}

bool FileJob::deleteDir(const QString &dir)
{
    if (m_status == FileJob::Cancelled) {
        emit result("cancelled");
        return false;
    }

    QDir sourceDir(dir);

    sourceDir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System | QDir::AllDirs);

    QDirIterator iterator(sourceDir, QDirIterator::Subdirectories);

    while (iterator.hasNext()) {
        const QFileInfo &fileInfo = iterator.next();
        if (fileInfo.exists() || fileInfo.isSymLink()){
            if (fileInfo.isFile() || fileInfo.isSymLink()) {
                if (!deleteFile(fileInfo.filePath())) {
                    qDebug() << "Unable to remove file:" << fileInfo.filePath();
                    emit error("Unable to remove file");
                    return false;
                }
            } else {
                qDebug() << fileInfo.filePath() << fileInfo.isDir() << fileInfo.isFile() << fileInfo.isSymLink();
                if (fileInfo.isDir()){
                    if (!deleteDir(fileInfo.filePath()))
                        return false;
                }
            }
        }
    }
//    qDebug() << "delete dir:" <<sourceDir.path();
    if (!sourceDir.rmdir(QDir::toNativeSeparators(sourceDir.path()))) {
        qDebug() << "Unable to remove dir:" << sourceDir.path();
//        emit("Unable to remove dir: " + sourceDir.path());
//        emit fileSignalManager->requestShowNoPermissionDialog(DUrl::fromLocalFile(dir));
        m_noPermissonUrls << DUrl::fromLocalFile(dir);
        return false;
    }

    return true;
}

void FileJob::deleteEmptyDir(const QString &srcPath)
{
    QFlags<QDir::Filter> f = QDir::AllEntries | QDir::System
            | QDir::NoDotAndDotDot
            | QDir::Hidden;
    QDirIterator tmp_iterator(srcPath, f);

    while (tmp_iterator.hasNext()) {
        tmp_iterator.next();
        const QFileInfo fileInfo = tmp_iterator.fileInfo();
        if (fileInfo.isDir()){
            QString _srcPath = fileInfo.absoluteFilePath();
            QDir _srcDir(_srcPath);
            _srcDir.setFilter(f);
            if (_srcDir.count() == 0){
                deleteDir(_srcPath);
            }else{
                deleteEmptyDir(_srcPath);
            }
        }
    }
    QDir srcDir(srcPath);
    srcDir.setFilter(f);
    if (srcDir.count() == 0){
        deleteDir(srcPath);
    }
}

bool FileJob::moveDirToTrash(const QString &dir, QString *targetPath)
{
    if(m_status == FileJob::Cancelled)
    {
        emit result("cancelled");
        return false;
    }

    qDebug() << "moveDirToTrash" << dir;

    QDir sourceDir(dir);


    QString baseName = getNotExistsTrashFileName(sourceDir.dirName());
    QString newName = m_trashLoc + "/files/" + baseName;
    QString delTime = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (!writeTrashInfo(baseName, dir, delTime))
        return false;

    if (targetPath)
        *targetPath = newName;

    return true;
}

QString FileJob::getNotExistsTrashFileName(const QString &fileName)
{
    QByteArray name = fileName.toUtf8();

    int index = name.lastIndexOf('/');

    if (index >= 0)
        name = name.mid(index + 1);

    index = name.lastIndexOf('.');
    QByteArray suffix;

    if (index >= 0)
        suffix = name.mid(index);

    if (suffix.size() > 200)
        suffix = suffix.left(200);

    name.chop(suffix.size());
    name = name.left(200 - suffix.size());

    while (QFile::exists(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath) + "/" + name + suffix)) {
        name = QCryptographicHash::hash(name, QCryptographicHash::Md5).toHex();
    }

    return QString::fromUtf8(name + suffix);
}

bool FileJob::moveFileToTrash(const QString &file, QString *targetPath)
{
#ifdef SW_LABEL
    if (LlsDeepinLabelLibrary::instance()->isCompletion()){
        if (isLabelFile(file)){
            int nRet = checkMoveJobPrivilege(file, "");
            if (nRet != 0){
                emit fileSignalManager->jobFailed(nRet, QString(QMetaEnum::fromType<JobType>().valueToKey(m_jobType)), file);
                return false;
            }
        }
    }
#endif

    if(m_status == FileJob::Cancelled)
    {
        emit result("cancelled");
        return false;
    }

    QFile localFile(file);
    QString path = m_trashLoc + "/files/";
    QString baseName = getNotExistsTrashFileName(localFile.fileName());
    QString newName = path + baseName;
    QString delTime = QDateTime::currentDateTime().toString(Qt::ISODate);

    qDebug() << "moveFileToTrash" << file;

    if (!writeTrashInfo(baseName, file, delTime))
        return false;

    if (targetPath)
        *targetPath = newName;

    return true;
}

bool FileJob::writeTrashInfo(const QString &fileBaseName, const QString &path, const QString &time)
{
    QFile metadata( m_trashLoc + "/info/" + fileBaseName + ".trashinfo" );

    if (!metadata.open( QIODevice::WriteOnly )) {
        qDebug() << metadata.fileName() << "file open error:" << metadata.errorString();

        return false;
    }

    QByteArray data;

    data.append("[Trash Info]\n");
    data.append("Path=").append(path.toUtf8().toPercentEncoding("/")).append("\n");
    data.append("DeletionDate=").append(time).append("\n");

    // save the file tag info
    const QStringList tag_name_list = TagManager::instance()->getTagsThroughFiles({DUrl::fromLocalFile(path)});

    if (!tag_name_list.isEmpty())
        data.append("TagNameList=").append(tag_name_list.join(",")).append("\n");

    qint64 size = metadata.write(data);

    metadata.close();

    if (size < 0) {
        qDebug() << "write file " << metadata.fileName() << "error:" << metadata.errorString();
    }

    return size > 0;
}

bool FileJob::checkDiskSpaceAvailable(const DUrlList &files, const DUrl &destination)
{
//    UDiskDeviceInfoPointer info = deviceListener->getDeviceByPath(destination.path()); // get disk info from mount point
//    if(!info)
//        info = deviceListener->getDeviceByFilePath(destination.path()); // get disk infor from mount mount point sub path
    if (FileUtils::isGvfsMountFile(destination.toLocalFile())){
        m_totalSize = FileUtils::totalSize(files);
        return true;
    }

    qint64 freeBytes;
    freeBytes = getStorageInfo(destination.toLocalFile()).bytesFree();

    m_isCheckingDisk = true;

    bool isInLimit = true;
    QMap<QString, QString> jobDataDetail;

    jobDataDetail.insert("status", "calculating");
    jobDataDetail.insert("file", files.first().fileName());
    jobDataDetail.insert("progress", m_progress);
    jobDataDetail.insert("destination", destination.fileName());

    m_checkDiskJobDataDetail = jobDataDetail;

    //calculate files's sizes
    m_totalSize = FileUtils::totalSize(files, freeBytes, isInLimit);

    jobDataDetail["status"] = "working";

    m_checkDiskJobDataDetail = jobDataDetail;

    if(!isInLimit)
        qDebug() << QString ("Can't copy or move files to target disk, disk free: %1").arg(FileUtils::formatSize(freeBytes));

    return isInLimit;
}

bool FileJob::checkTrashFileOutOf1GB(const DUrl &url)
{
    const QFileInfo &info(url.toLocalFile());

    if (info.isSymLink())
        return true;

    DUrlList list;
    list << url;
    m_isCheckingDisk = true;

    bool isInLimit = true;
    QMap<QString, QString> jobDataDetail;

    jobDataDetail.insert("status", "calculating");
    jobDataDetail.insert("file", url.fileName());
    jobDataDetail.insert("progress", m_progress);
    jobDataDetail.insert("destination", "");

    m_checkDiskJobDataDetail = jobDataDetail;

    //calculate files's sizes
    m_totalSize = FileUtils::totalSize(list, 1024*1024*1024, isInLimit);

    jobDataDetail["status"] = "working";

    m_checkDiskJobDataDetail = jobDataDetail;

    return isInLimit;
}

bool FileJob::checkFat32FileOutof4G(const QString &srcFile, const QString &tarDir)
{
    QFileInfo srcFileInfo(srcFile);

    qint64 threshold = srcFileInfo.size() / 1024;
    threshold = threshold / 1024;
    threshold = threshold / 1024;

    if (threshold >= 4 ){
        UDiskDeviceInfoPointer pDesDevice;
        UDiskDeviceInfoPointer pDesDevice1 = deviceListener->getDeviceByPath(tarDir);
        UDiskDeviceInfoPointer pDesDevice2 = deviceListener->getDeviceByFilePath(tarDir);
        if (pDesDevice1 || pDesDevice2){
            if (pDesDevice1){
                pDesDevice = pDesDevice1;
            }
            if (pDesDevice2){
                pDesDevice = pDesDevice2;
            }
            if (pDesDevice){
                QString devicePath = pDesDevice->getDiskInfo().unix_device();
                QString fstype = PartMan::Partition::getPartitionByDevicePath(devicePath).fs();
                if (fstype == "vfat" ){
                    emit fileSignalManager->requestShow4GFat32Dialog();
                    return true;
                }
            }
        }
    }
    return false;
}

bool FileJob::checkUseGvfsFileOperation(const DUrlList &files, const DUrl &destination)
{
    if (checkUseGvfsFileOperation(destination.path())){
        return true;
    }
    foreach (DUrl url, files) {
        if (checkUseGvfsFileOperation(url.path())){
            return true;
        }
    }
    return false;
}

bool FileJob::checkUseGvfsFileOperation(const QString &path)
{
    return FileUtils::isGvfsMountFile(path);
}

QStorageInfo FileJob::getStorageInfo(const QString &file)
{
    QFileInfo info(file);

    return info.isSymLink()
            ? QStorageInfo(info.absolutePath())
            : QStorageInfo(info.absoluteFilePath());
}

bool FileJob::canMove(const QString &filePath)
{
    QFileInfo file_info(filePath);
    QFileInfo dir_info(file_info.dir().absolutePath());

    if (!dir_info.permission(QFile::WriteUser))
        return false;

#ifdef Q_OS_LINUX
    // 如果是root，则拥有权限
    if (getuid() == 0) {
        return true;
    }

    QT_STATBUF statBuffer;
    if (QT_LSTAT(dir_info.absoluteFilePath().toUtf8().constData(), &statBuffer) == 0) {
        // 如果父目录拥有t权限，则判断当前用户是不是文件的owner，不是则无法操作文件
        if ((statBuffer.st_mode & S_ISVTX) && file_info.ownerId() != getuid()) {
            return false;
        }
    }
#endif

    return true;
}

QString FileJob::getXorrisoErrorMsg(const QStringList &msg)
{
    QRegularExpression ovrex("While grafting '(.*)'");
    for (auto& msgs : msg) {
        auto ovrxm = ovrex.match(msgs);
        if (msgs.contains("file object exists and may not be overwritten") && ovrxm.hasMatch()) {
            return tr("%1 is a duplicate file.").arg(ovrxm.captured(1));
        }
        if (msgs.contains("Lost connection to drive")) {
            return tr("Lost connection to drive.");
        }
        if (msgs.contains("servo failure")) {
            return tr("The CD/DVD drive is not ready. Try another disc.");
        }
        if (msgs.contains("Device or resource busy")) {
            return tr("The CD/DVD drive is busy. Exit the program using the drive, and insert the drive again.");
        }
    }
    return tr("Unknown error");
}

#ifdef SW_LABEL
bool FileJob::isLabelFile(const QString &srcFileName)
{
    if (LlsDeepinLabelLibrary::instance()->isCompletion()){
        std::string path = srcFileName.toStdString();
//        int ret = lls_simplechecklabel(const_cast<char*>(path.c_str()));
        int ret = LlsDeepinLabelLibrary::instance()->lls_simplechecklabel()(const_cast<char*>(path.c_str()));
        qDebug() << ret << srcFileName;
        if (ret == 0){
            return true;
        }else{
            return false;
        }
    }
    return false;
}

int FileJob::checkCopyJobPrivilege(const QString &srcFileName, const QString &dstFileName)
{
    if (LlsDeepinLabelLibrary::instance()->isCompletion()){
        std::string path = srcFileName.toStdString();
        std::string dstpath = dstFileName.toStdString();
//        int nRet =  lls_checkprivilege(const_cast<char*>(path.c_str()), NULL, E_FILE_PRI_COPY);
        int nRet = LlsDeepinLabelLibrary::instance()->lls_checkprivilege()(const_cast<char*>(path.c_str()), const_cast<char*>(dstpath.c_str()), E_FILE_PRI_COPY);
        qDebug() << nRet << srcFileName;
        return nRet;
    }
    return -1;
}

int FileJob::checkMoveJobPrivilege(const QString &srcFileName, const QString &dstFileName)
{
    if (LlsDeepinLabelLibrary::instance()->isCompletion()){
        std::string path = srcFileName.toStdString();
        std::string dstpath = dstFileName.toStdString();
//        int nRet =  lls_checkprivilege(const_cast<char*>(path.c_str()), NULL, E_FILE_PRI_MOVE);
        int nRet =   LlsDeepinLabelLibrary::instance()->lls_checkprivilege()(const_cast<char*>(path.c_str()), const_cast<char*>(dstpath.c_str()), E_FILE_PRI_MOVE);
        qDebug() << nRet << srcFileName;
        return nRet;
    }
    return -1;
}

int FileJob::checkStoreInRemovableDiskPrivilege(const QString &srcFileName, const QString &dstFileName)
{
    if (LlsDeepinLabelLibrary::instance()->isCompletion()){
        std::string path = srcFileName.toStdString();
        std::string dstpath = dstFileName.toStdString();
//        int nRet =  lls_checkprivilege(const_cast<char*>(path.c_str()), NULL, E_FILE_PRI_STORE);
        int nRet =  LlsDeepinLabelLibrary::instance()->lls_checkprivilege()(const_cast<char*>(path.c_str()), const_cast<char*>(dstpath.c_str()), E_FILE_PRI_STORE);
        qDebug() << nRet << srcFileName;
        return nRet;
    }
    return -1;
}

int FileJob::checkDeleteJobPrivilege(const QString &srcFileName)
{
    if (LlsDeepinLabelLibrary::instance()->isCompletion()){
        std::string path = srcFileName.toStdString();
//        int nRet =  lls_checkprivilege(const_cast<char*>(path.c_str()), NULL, E_FILE_PRI_DELETE);
        int nRet =   LlsDeepinLabelLibrary::instance()->lls_checkprivilege()(const_cast<char*>(path.c_str()), NULL, E_FILE_PRI_DELETE);
        qDebug() << nRet << srcFileName;
        return nRet;
    }
    return -1;
}

int FileJob::checkRenamePrivilege(const QString &srcFileName)
{
    if (LlsDeepinLabelLibrary::instance()->isCompletion()){
        std::string path = srcFileName.toStdString();
//        int nRet =  lls_checkprivilege(const_cast<char*>(path.c_str()), NULL, E_FILE_PRI_RENAME);
        int nRet =  LlsDeepinLabelLibrary::instance()->lls_checkprivilege()(const_cast<char*>(path.c_str()), NULL, E_FILE_PRI_RENAME);
        qDebug() << nRet << srcFileName;
        return nRet;
    }
    return -1;
}

int FileJob::checkReadPrivilege(const QString &srcFileName)
{
    if (LlsDeepinLabelLibrary::instance()->isCompletion()){
        std::string path = srcFileName.toStdString();
//        int nRet =  lls_checkprivilege(const_cast<char*>(path.c_str()), NULL, E_FILE_PRI_READ);
        int nRet =  LlsDeepinLabelLibrary::instance()->lls_checkprivilege()(const_cast<char*>(path.c_str()), NULL, E_FILE_PRI_READ);
        qDebug() << nRet << srcFileName;
        return nRet;
    }
    return -1;
}
#endif
