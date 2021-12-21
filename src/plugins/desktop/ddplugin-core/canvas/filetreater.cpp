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
#include "filetreater.h"
#include "private/filetreater_p.h"
#include "grid/canvasgrid.h"
#include "dfm-base/base/schemefactory.h"

#include <QDir>
#include <QtConcurrent>
#include <QStandardPaths>

DSB_D_BEGIN_NAMESPACE

class FileTreaterGlobal : public FileTreater{};
Q_GLOBAL_STATIC(FileTreaterGlobal, fileTreater)

FileTreater::FileTreater(QObject *parent)
    : QObject(parent)
    , d(new FileTreaterPrivate(this))
{
    Q_ASSERT(thread() == qApp->thread());
}

FileTreater::~FileTreater()
{
    delete d;
    d = nullptr;
}

FileTreater *FileTreater::instance()
{
    return fileTreater;
}

/*!
 * \brief 根据url获取对应的文件对象, \a url 文件路径
 * \return
 */
DFMDesktopFileInfoPointer FileTreater::file(const QString &url)
{
    return d->fileHashTable.value(url);
}

/*!
 * \brief 根据index获取对应的文件对象, \a index 文件路径
 * \return
 */
DFMDesktopFileInfoPointer FileTreater::file(int index)
{
    Q_UNUSED(index)
    if (index >= 0 && index < fileCount()) {
        return d->fileList.at(index);
    }

    return nullptr;
}

/*!
 * \brief 排序列表与方式，排序方式暂不确定
 * \param fileInfoLst 待排序列表
 * \param str 排序方式
 * \return 排序后的列表
 */
QList<DFMDesktopFileInfoPointer> &FileTreater::sortFiles(QList<AbstractFileInfo *> &fileInfoLst, QString &str)
{
    Q_UNUSED(fileInfoLst)
    Q_UNUSED(str)
    return d->fileList;
}

QList<DFMDesktopFileInfoPointer> &FileTreater::getFiles()
{
    return d->fileList;
}

int FileTreater::indexOfChild(DFMDesktopFileInfoPointer info)
{
    return d->fileList.indexOf(info, 0);
}

int FileTreater::fileCount()
{
    return d->fileList.size();
}

/*!
 * \brief 初始化数据
 */
void FileTreater::init()
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

QString FileTreater::homePath()
{
    return d->homePath;
}

bool FileTreater::isDone()
{
    return d->isDone;
}

void FileTreater::loadData(const QDir &url)
{
    connect(&futureWatcher, &QFutureWatcher<void>::finished, this, &FileTreater::onFileWatcher);
    future = QtConcurrent::run(this, &FileTreater::asyncFunc, url);

    futureWatcher.setFuture(future);
}

void FileTreater::initconnection()
{
}

bool FileTreater::startWatch()
{
    return false;
}

bool FileTreater::stopWatch()
{
    return false;
}

void FileTreater::updateFile(const QString &)
{
}

void FileTreater::asyncFunc(const QDir &url)
{
    QMutexLocker lk(&mutex);
    QDir dir(url);
    bool isHidden = false; //todo(lq)
    for (auto info : dir.entryInfoList()) {
        if ((!info.isHidden()) || (info.isHidden() && isHidden)) {
            qDebug() << info.filePath();
            auto routeU = UrlRoute::pathToReal(info.filePath());
            QString errString;
            auto itemInfo = InfoFactory::create<DefaultDesktopFileInfo>(routeU, &errString);
            if (!itemInfo)
                qInfo() << "asyncFunc error: " << errString;
            d->fileList.append(itemInfo);
            d->fileHashTable.insert(itemInfo->url().toString(), itemInfo);
        }
    }
    qDebug() << "file count " << d->fileList.size();
}

void FileTreater::onAddFile(const QString &)
{
}

void FileTreater::onDeleteFile(const QString &)
{
}

void FileTreater::onMoveFile(const QString &, const QString &)
{
}

void FileTreater::onFileWatcher()
{
    emit fileFinished();
    d->isDone = true;
}

DSB_D_END_NAMESPACE
