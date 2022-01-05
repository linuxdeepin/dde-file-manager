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
#ifndef FILETREATER_H
#define FILETREATER_H

#include "dfm-base/file/local/localfileinfo.h"
#include "dfm_desktop_service_global.h"

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>

DSB_D_BEGIN_NAMESPACE

class FileTreaterPrivate;
class FileTreater : public QObject
{
    Q_OBJECT
public:
    static FileTreater *instance();
    void init();
    DFMLocalFileInfoPointer file(const QString &url);
    DFMLocalFileInfoPointer file(int index);

    QList<DFMLocalFileInfoPointer> &sortFiles(QList<dfmbase::AbstractFileInfo *> &fileInfoLst, QString &str);
    QList<DFMLocalFileInfoPointer> &getFiles();
    int indexOfChild(DFMLocalFileInfoPointer info);
    int fileCount();
    QString homePath();
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

protected:
    explicit FileTreater(QObject *parent = nullptr);
    ~FileTreater();
private:
    QFuture<void> future;
    QFutureWatcher<void> futureWatcher;
    FileTreaterPrivate *d;
    QMutex mutex;
};

#define FileTreaterCt DSB_D_NAMESPACE::FileTreater::instance()

DSB_D_END_NAMESPACE
#endif   // FILETREATER_H
