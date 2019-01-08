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

#include "circleprogressanimatepad.h"
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

DFM_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class ErrorHandle;
class MoveCopyTaskWidget : public QFrame
{
    Q_OBJECT
public:
    explicit MoveCopyTaskWidget(const QMap<QString, QString>& jobDetail, QWidget *parent = 0);
    explicit MoveCopyTaskWidget(DFileCopyMoveJob *job, QWidget *parent = 0);
    ~MoveCopyTaskWidget();

    void initUI();
    void initConnect();
    QString getTargetObj();
    QString getDestinationObj();
    int getProgress();
    float getSpeed();
    int getTimeLeft();
    QString getMessage();
    QString getTipMessage();

    void initConflictDetailFrame();
    void initButtonFrame();

    DFileCopyMoveJob::Handle *errorHandle() const;

signals:
    void closed(const QMap<QString, QString>& jobDetail);
    void conflictResponseConfirmed(const QMap<QString, QString>& jobDetail, const QMap<QString, QVariant>& response);
    void heightChanged();
    void conflictShowed(const QMap<QString, QString>& jobDetail);
    void conflictHided(const QMap<QString, QString>& jobDetail);
    void hovereChanged(const bool& hover);

public slots:
    void setTargetObj(QString targetObj);
    void setDestinationObj(QString destinationObj);
    void setProgress(int value);
    void setProgress(QString value);
    void setSpeed(float speed);
    void setTimeLeft(int time);
    void setMessage(const QString &operateStr, const QString &destinateStr);
    void setTipMessage(const QString &speedStr, const QString &remainStr);
    void handleClose();
    void handleResponse();
    void updateMessageByJob();
    void updateMessage(const QMap<QString, QString>& data);
    void updateTipMessage();
    void handleLineDisplay(const int& row, const bool &hover, const int &taskNum);

    void showConflict();
    void hideConflict();

    void showButtonFrame();
    void hideButtonFrame();

    void updateConflictDetailFrame(const DUrl originFilePath, const DUrl targetFilePath);

    //　链接到其它线程的信号时尽量不使用引用接收参数，有时会发生此引用对象无效的诡异问题
    void onJobCurrentJobChanged(const DUrl from, const DUrl to);
    void onJobSpeedChanged(qint64 speed);
    void onJobProgressChanged(qreal progress);
    void disposeJobError(DFileCopyMoveJob::Action action);
protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE;

private:
    int m_progress = 0;
    float m_speed = 0.0;
    int m_timeLeft;
    QString m_targetObj;
    QString m_destinationObj;
    QString m_speedMessage;
    QString m_remainMessage;
    QString m_operateMessage;
    QString m_destinationMessage;
    QString m_sourcePath;
    QString m_targetPath;
    DCircleProgress* m_cirleWidget=NULL;
    QLabel* m_speedLabel;
    QLabel* m_remainLabel;
    QLabel* m_msg1Label;
    QLabel* m_msg2Label;
    QLabel *m_errorLabel = nullptr;

    QPushButton* m_closeButton;
    QPushButton* m_pauseBuuton;
    QPushButton* m_keepBothButton;
    QPushButton* m_skipButton;
    QPushButton* m_replaceButton;
    QMap<QString, QString> m_jobDetail;
    QMap<QString, QVariant> m_response;
    QButtonGroup* m_buttonGroup;
    QFrame* m_buttonFrame=NULL;
    QFrame* m_conflictFrame=NULL;
    QLabel* m_originIconLabel = NULL;
    QLabel* m_originTitleLabel = NULL;
    QLabel* m_originTimeLabel = NULL;
    QLabel* m_originSizeLabel = NULL;
    QLabel* m_targetIconLabel = NULL;
    QLabel* m_targetTitleLabel = NULL;
    QLabel* m_targetTimeLabel = NULL;
    QLabel* m_targetSizeLabel = NULL;

    QCheckBox* m_checkBox=NULL;
    QPushButton* m_enterButton=NULL;
    CircleProgressAnimatePad* m_animatePad;
    QLabel* m_bgLabel;
    QFrame* m_lineLabel = NULL;

    DFileCopyMoveJob *m_fileJob = nullptr;
    ErrorHandle *m_errorHandle = nullptr;

    struct JobInfo {
        QPair<DUrl, DUrl> currentJob;
        qint64 speed;
        qreal progress;
        qint64 totalDataSize = -1;
    };

    JobInfo *m_jobInfo = nullptr;
};


class DTaskDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit DTaskDialog(QWidget *parent = 0);
    void initUI();
    void initConnect();
    static int MaxHeight;
    QListWidget* getTaskListWidget();

signals:
    void abortTask(const QMap<QString, QString>& jobDetail);
    void conflictRepsonseConfirmed(const QMap<QString, QString>& jobDetail, const QMap<QString, QVariant>& response);
    void conflictShowed(const QMap<QString, QString>& jobDetail);
    void conflictHided(const QMap<QString, QString>& jobDetail);
    void closed();
    void currentHoverRowChanged(const int& row, const bool& hover, const int& taskNum);

public slots:
    void setTitle(QString title);
    void setTitle(int taskCount);
    void addTask(const QMap<QString, QString>& jobDetail);
    MoveCopyTaskWidget *addTaskJob(DFileCopyMoveJob *job);
    void addConflictTask(const QMap<QString, QString>& jobDetail);
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

    void showConflictDiloagByJob(const QMap<QString, QString>& jobDetail);
    void handleConflictResponse(const QMap<QString, QString>& jobDetail, const QMap<QString, QVariant>& response);

    void handleMinimizeButtonClick();
    void onItemHovered(const bool &hover);



protected:
    void closeEvent(QCloseEvent* event);
    void keyPressEvent(QKeyEvent *event);


private:
    int m_defaultWidth = 525;
    int m_defaultHeight = 120;
    QLabel* m_titleLabel=NULL;
    QPushButton* m_titlebarMinimizeButton;
    QPushButton* m_titlebarCloseButton;
    QListWidget* m_taskListWidget=NULL;
    QMap<QString, QListWidgetItem*> m_jobIdItems;
    DTitlebar* m_titlebar;
};

#endif // DTASKDIALOG_H
