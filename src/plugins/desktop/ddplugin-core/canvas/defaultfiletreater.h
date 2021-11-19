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
#ifndef DEFAULTFILETREATER_H
#define DEFAULTFILETREATER_H

#include "dfm-base/widgets/abstractfiletreater.h"
#include "defaultdesktopfileinfo.h"

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>

DFMBASE_USE_NAMESPACE
DSB_D_BEGIN_NAMESPACE

class DefaultFileTreaterPrivate;
class DefaultFileTreater : public QObject
{
    Q_OBJECT
public:
    static DefaultFileTreater *instance();
    void init();
    DFMDesktopFileInfoPointer getFileByUrl(const QString &url);
    DFMDesktopFileInfoPointer getFileByIndex(int index);

    QList<DFMDesktopFileInfoPointer> &sortFiles(QList<AbstractFileInfo *> &fileInfoLst, QString &str);
    QList<DFMDesktopFileInfoPointer> &getFiles();
    int indexOfChild(DFMDesktopFileInfoPointer info);
    int fileCount();
    QString &homePath();
    bool isDone();

protected:
    void loadData(const QDir &url);
    void initconnection();
    bool startWatch();
    bool stopWatch();
    void updateFile(const QString &);
    void asyncFunc(const QDir &url);

signals:
    void fileMove(const QStringList &oldLst, const QStringList &newLst);
    void fileAdd(const QString &);
    void fileDelete(const QString &);
    void fileFinished();

protected slots:
    void onAddFile(const QString &);
    void onDeleteFile(const QString &);
    void onMoveFile(const QString &, const QString &);
    void onFileWatcher();

private:
    explicit DefaultFileTreater(QObject *parent = nullptr);

private:
    QFuture<void> future;
    QFutureWatcher<void> futureWatcher;
    DefaultFileTreaterPrivate *d;
    QMutex mutex;
};

#define DefaultFileTreaterCt DefaultFileTreater::instance()

DSB_D_END_NAMESPACE
#endif   // DEFAULTFILETREATER_H
