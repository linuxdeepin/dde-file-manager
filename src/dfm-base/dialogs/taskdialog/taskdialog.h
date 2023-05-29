// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <DDialog>
#include <DTitlebar>

#include <QObject>
#include <QtDBus/QtDBus>

class QListWidget;
class QListWidgetItem;
class QMutex;

DWIDGET_USE_NAMESPACE
namespace dfmbase {
class TaskWidget;
class TaskDialog : public DAbstractDialog
{
    Q_OBJECT

public:
    explicit TaskDialog(QObject *parent = nullptr);
    ~TaskDialog() override;
    void addTask(const JobHandlePointer taskHandler);
    void initUI();
    void blockShutdown();
    void addTaskWidget(const JobHandlePointer taskHandler, TaskWidget *wid);
    void setTitle(int taskCount);

Q_SIGNALS:
    /*!
     * \brief closed 当前进度窗口关闭时，发送关闭信号
     */
    void closed();
private Q_SLOTS:
    void moveYCenter();
    void removeTask();
    void adjustSize(int hight = 0);

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QListWidget *taskListWidget { nullptr };
    QMap<JobHandlePointer, QListWidgetItem *> taskItems;
    DTitlebar *titlebar { nullptr };
    QDBusReply<QDBusUnixFileDescriptor> replyBlokShutDown;
    static int kMaxHeight;
};

}

#endif   // TASKDIALOG_H
