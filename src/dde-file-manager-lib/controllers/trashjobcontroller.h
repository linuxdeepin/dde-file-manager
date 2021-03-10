/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#ifndef TRASHJOBCONTROLLER_H
#define TRASHJOBCONTROLLER_H

#include <QtCore>
#include <QtDBus>

class TrashMonitorInterface;
class TrashJobInterface;
class EmptyTrashJobInterface;
class FileConflictController;

class TrashJobController : public QObject
{
    Q_OBJECT
public:
    explicit TrashJobController(QObject *parent = 0);
    ~TrashJobController();

    void initConnect();

signals:

public slots:
    void setTrashEmptyFlag(bool flag);
    void connectTrashSignal();
    void disconnectTrashSignal();
    void trashJobExcute(const QStringList& files);
    void trashJobExcuteFinished();
    void trashJobAbort();
    void trashJobAbortFinished();
    void onTrashingFile(QString file);
    void onDeletingFile(QString file);
    void onProcessAmount(qlonglong progress, ushort info);
    void updateTrashIconByCount(uint count);

    void confirmDelete();
    void handleTrashAction(int index);
    void createEmptyTrashJob();
    void emptyTrashJobExcuteFinished();

    void asyncRequestTrashCount();

private:
    TrashMonitorInterface* m_trashMonitorInterface=NULL;
    TrashJobInterface* m_trashJobInterface = NULL;
    EmptyTrashJobInterface* m_emptyTrashJobInterface = NULL;
    FileConflictController* m_conflictController = NULL;
    bool m_isTrashEmpty = true;
};

#endif // TRASHJOBCONTROLLER_H
