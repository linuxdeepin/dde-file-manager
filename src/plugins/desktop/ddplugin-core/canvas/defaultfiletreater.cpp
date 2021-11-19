/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "defaultfiletreater.h"
#include "private/defaultfiletreater_p.h"
#include "defaultcanvasgridmanager.h"
#include "dfm-base/base/schemefactory.h"

#include <QDir>
#include <QtConcurrent>
#include <QStandardPaths>

DSB_D_BEGIN_NAMESPACE

DefaultFileTreater::DefaultFileTreater(QObject *parent)
    : QObject(parent), d(new DefaultFileTreaterPrivate(this))
{
}

DefaultFileTreater *DefaultFileTreater::instance()
{
    static DefaultFileTreater fileTreater;
    return &fileTreater;
}

/*!
 * \brief 根据url获取对应的文件对象, \a url 文件路径
 * \return
 */
DFMDesktopFileInfoPointer DefaultFileTreater::getFileByUrl(const QString &url)
{
    Q_UNUSED(url)
    return nullptr;
}

/*!
 * \brief 根据index获取对应的文件对象, \a index 文件路径
 * \return
 */
DFMDesktopFileInfoPointer DefaultFileTreater::getFileByIndex(int index)
{
    Q_UNUSED(index)
    return nullptr;
}

/*!
 * \brief 排序列表与方式，排序方式暂不确定
 * \param fileInfoLst 待排序列表
 * \param str 排序方式
 * \return 排序后的列表
 */
QList<DFMDesktopFileInfoPointer> &DefaultFileTreater::sortFiles(QList<AbstractFileInfo *> &fileInfoLst, QString &str)
{
    Q_UNUSED(fileInfoLst)
    Q_UNUSED(str)
    return d->fileList;
}

QList<DFMDesktopFileInfoPointer> &DefaultFileTreater::getFiles()
{
    return d->fileList;
}

int DefaultFileTreater::indexOfChild(DFMDesktopFileInfoPointer info)
{
    return d->fileList.indexOf(info, 0);
}

int DefaultFileTreater::fileCount()
{
    return d->fileList.size();
}

/*!
 * \brief 初始化数据
 */
void DefaultFileTreater::init()
{
    d->isDone = false;
    d->homePath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    QDir rootDir(d->homePath);
    if (rootDir.path().isEmpty()) {
        qInfo() << "rootDir is faild to get";
    } else {
        loadData(rootDir);
    }
}

QString &DefaultFileTreater::homePath()
{
    return d->homePath;
}

bool DefaultFileTreater::isDone()
{
    return d->isDone;
}

void DefaultFileTreater::loadData(const QDir &url)
{
    connect(&futureWatcher, &QFutureWatcher<void>::finished, this, &DefaultFileTreater::onFileWatcher);
    future = QtConcurrent::run(this, &DefaultFileTreater::asyncFunc, url);

    futureWatcher.setFuture(future);
}

void DefaultFileTreater::initconnection()
{
}

bool DefaultFileTreater::startWatch()
{
    return false;
}

bool DefaultFileTreater::stopWatch()
{
    return false;
}

void DefaultFileTreater::updateFile(const QString &)
{
}

void DefaultFileTreater::asyncFunc(const QDir &url)
{
    QMutexLocker lk(&mutex);
    QDir dir(url);
    bool isHidden = DefaultCanvasGridManager::instance()->getShowHiddenFiles();
    for (auto info : dir.entryInfoList()) {
        if ((!info.isHidden()) || (info.isHidden() && isHidden)) {
            qDebug() << info.filePath();
            auto routeU = UrlRoute::pathToReal(info.filePath());
            QString errString;
            auto itemInfo = InfoFactory::create<DefaultDesktopFileInfo>(routeU, &errString);
            if (!itemInfo)
                qInfo() << "asyncFunc error: " << errString;
            d->fileList.append(itemInfo);
        }
    }
    qDebug() << "file count " << d->fileList.size();
}

void DefaultFileTreater::onAddFile(const QString &)
{
}

void DefaultFileTreater::onDeleteFile(const QString &)
{
}

void DefaultFileTreater::onMoveFile(const QString &, const QString &)
{
}

void DefaultFileTreater::onFileWatcher()
{
    emit fileFinished();
    d->isDone = true;
}

DSB_D_END_NAMESPACE
