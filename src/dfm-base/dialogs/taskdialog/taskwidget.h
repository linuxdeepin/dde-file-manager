/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef TASKWIDGET_H
#define TASKWIDGET_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"

#include <DWaterProgress>
#include <DIconButton>

#include <QWidget>
#include <QLabel>

class QPushButton;
class QCheckBox;
class QVBoxLayout;

DFMBASE_BEGIN_NAMESPACE

class ElidedLable : public QLabel
{
    friend class TaskWidget;
    Q_OBJECT
    explicit ElidedLable(QWidget *parent = nullptr);
    virtual ~ElidedLable();
    void setText(const QString &text);
};

class TaskWidget : public QWidget
{
    friend class TaskDialog;
    Q_OBJECT
    explicit TaskWidget(QWidget *parent = nullptr);
    ~TaskWidget();
    void setTaskHandle(const JobHandlePointer &handle);

signals:
    void buttonClicked(AbstractJobHandler::SupportActions actions);
    void heightChanged();
private slots:
    void onButtonClicked();
    void onShowErrors(const JobInfoPointer JobInfo);
    void onShowConflictInfo(const QUrl source, const QUrl target, const AbstractJobHandler::SupportActions action);
    void onHandlerTaskStateChange(const JobInfoPointer JobInfo);
    void onShowTaskInfo(const JobInfoPointer JobInfo);
    void onShowTaskProccess(const JobInfoPointer JobInfo);
    void onShowSpeedUpdatedInfo(const JobInfoPointer JobInfo);

private:
    void initUI();
    void initConnection();
    QWidget *createConflictWidget();
    QWidget *createBtnWidget();
    void showBtnByAction(const AbstractJobHandler::SupportActions &actions);
    void showConflictButtons(bool showBtns = true, bool showConflict = true);
    void onMouseHover(const bool hover);
    QString formatTime(qint64 second) const;

protected:
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void paintEvent(QPaintEvent *event);

private:
    DTK_WIDGET_NAMESPACE::DWaterProgress *progress { nullptr };   // 左侧水球动画
    ElidedLable *lbSrcPath { nullptr };   // 左第一个label
    ElidedLable *lbDstPath { nullptr };   // 左第二个label
    QLabel *lbSpeed { nullptr };   // 右第一个label
    QLabel *lbRmTime { nullptr };   // 右第二个label
    ElidedLable *lbErrorMsg { nullptr };   // 错误信label
    QLabel *lbSrcIcon { nullptr };   // 冲突widget上的源文件图标
    QLabel *lbDstIcon { nullptr };   // 冲突widget上的目标文件图标
    ElidedLable *lbSrcTitle { nullptr };   // 冲突widget上的源文件文件显示名称
    ElidedLable *lbDstTitle { nullptr };   // 冲突widget上的目标文件显示名称
    ElidedLable *lbSrcModTime { nullptr };   // 冲突widget上的源文件文件修改时间
    ElidedLable *lbDstModTime { nullptr };   // 冲突widget上的目标文件修改时间
    ElidedLable *lbSrcFileSize { nullptr };   // 冲突widget上的源文件文件文件大小
    ElidedLable *lbDstFileSize { nullptr };   // 冲突widget上的目标文件文件文件大小
    QWidget *widConfict { nullptr };   // 冲突widget
    QWidget *widButton { nullptr };   // 按钮界面

    QCheckBox *chkboxNotAskAgain { nullptr };   // 不在询问按钮
    DTK_WIDGET_NAMESPACE::DIconButton *btnStop { nullptr };   // 停止按钮
    DTK_WIDGET_NAMESPACE::DIconButton *btnPause { nullptr };   // 暂停按钮
    QPushButton *btnCoexist { nullptr };   // 共存按钮
    QPushButton *btnSkip { nullptr };   // 跳过按钮
    QPushButton *btnReplace { nullptr };   // 替换、合并按钮

    QVBoxLayout *rVLayout { nullptr };
    QVBoxLayout *mainLayout { nullptr };

    QAtomicInteger<bool> isPauseState { false };   // 是否是暂停状态
    QAtomicInteger<bool> isBtnHidden { false };   // 暂停、停止按钮隐藏
};

DFMBASE_END_NAMESPACE

#endif   // TASKWIDGET_H
