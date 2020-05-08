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

#ifndef DTASKDIALOG_H
#define DTASKDIALOG_H

#include "dfmgenericplugin.h"
#include "dfilecopymovejob.h"

#include <dcircleprogress.h>
#include <QLabel>
#include <QListWidget>
#include <QResizeEvent>
#include <QListWidgetItem>
#include <QButtonGroup>
#include <QCheckBox>
#include <ddialog.h>
#include <dplatformwindowhandle.h>
#include <DTitlebar>
#include <QtDBus/QtDBus>
#include <DWaterProgress>
#include <DIconButton>
DFM_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class ErrorHandle : public QObject, public DFileCopyMoveJob::Handle
{
    Q_OBJECT
public:
    ErrorHandle(QObject *parent):QObject(parent){}
    virtual ~ErrorHandle() override;
    DFileCopyMoveJob::Action handleError(DFileCopyMoveJob *job, DFileCopyMoveJob::Error error,
                                         const DAbstractFileInfo *sourceInfo,  const DAbstractFileInfo *targetInfo) override;

    void setActionOfError(DFileCopyMoveJob::Action action){ m_actionOfError = action; }
Q_SIGNALS:
    void onConflict(const DUrl &src, const DUrl &dst);
    void onError(const QString &err);

private:
    DFileCopyMoveJob::Action m_actionOfError = DFileCopyMoveJob::NoAction;
};

class DFMTaskWidget;
class DTaskDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit DTaskDialog(QWidget *parent = nullptr);
    void initUI();
    void initConnect();
    static int MaxHeight;
    QListWidget* getTaskListWidget();

    void updateData(DFMTaskWidget *wid, const QMap<QString, QString> &data);

signals:
    void abortTask(const QMap<QString, QString>& jobDetail);
    void conflictRepsonseConfirmed(const QMap<QString, QString>& jobDetail, const QMap<QString, QVariant>& response);
    void closed();

public slots:
    void setTitle(QString title);
    void setTitle(int taskCount);
    void addTask(const QMap<QString, QString>& jobDetail);
    DFileCopyMoveJob::Handle *addTaskJob(DFileCopyMoveJob *job);
    void handleTaskClose(const QMap<QString, QString>& jobDetail);
    void removeTask(const QMap<QString, QString>& jobDetail, bool adjustSize = true);
    void removeTaskJob(void *job);
    void removeTaskImmediately(const QMap<QString, QString>& jobDetail);
    void delayRemoveTask(const QMap<QString, QString>& jobDetail);
    void removeTaskByPath(QString jobId);
    void handleUpdateTaskWidget(const QMap<QString, QString>& jobDetail,
                                const QMap<QString, QString>& data);
    void adjustSize();
    void moveYCenter();

protected:
    void closeEvent(QCloseEvent* event);
    void keyPressEvent(QKeyEvent *event);

    void blockShutdown();
    void addTaskWidget(DFMTaskWidget *wid);

private:
    int m_defaultWidth = 700;
    int m_defaultHeight = 120;
    //QLabel* m_titleLabel=NULL;
    //QPushButton* m_titlebarMinimizeButton;
    //QPushButton* m_titlebarCloseButton;
    QListWidget* m_taskListWidget=NULL;
    QMap<QString, QListWidgetItem*> m_jobIdItems;
    DTitlebar* m_titlebar;
    QDBusReply<QDBusUnixFileDescriptor> m_reply; // ~QDBusUnixFileDescriptor() will disposes of the Unix file descriptor that it contained.
};

#endif // DTASKDIALOG_H
