// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKWIDGET_H
#define TASKWIDGET_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <DWaterProgress>
#include <DIconButton>
#include <DPushButton>
#include <DWarningButton>

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QLayout>

class QCheckBox;
class QVBoxLayout;

namespace dfmbase {

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
    void setTaskHandle(const JobHandlePointer handle);
    void resetPauseStute();

Q_SIGNALS:
    void buttonClicked(AbstractJobHandler::SupportActions actions);
    void heightChanged(int height);
public Q_SLOTS:
    void parentClose();
private Q_SLOTS:
    void onButtonClicked();
    void onShowErrors(const JobInfoPointer JobInfo);
    void onShowConflictInfo(const QUrl source, const QUrl target, const AbstractJobHandler::SupportActions action);
    void onShowPermanentlyDelete(const QUrl source, const AbstractJobHandler::SupportActions action);
    void onHandlerTaskStateChange(const JobInfoPointer JobInfo);
    void onShowTaskInfo(const JobInfoPointer JobInfo);
    void onShowTaskProccess(const JobInfoPointer JobInfo);
    void onShowSpeedUpdatedInfo(const JobInfoPointer JobInfo);
    void onInfoTimer();

private:
    void initUI();
    void initConnection();
    QWidget *createConflictWidget();
    QWidget *createBtnWidget();
    QWidget *createBaseWidget();
    void showBtnByAction(const AbstractJobHandler::SupportActions &actions);
    void showConflictButtons(bool showBtns = true, bool showConflict = true);
    void onMouseHover(const bool hover);
    QString formatTime(qint64 second) const;
    bool showFileInfo(const FileInfoPointer info, const bool isOrg);

protected:
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    virtual void enterEvent(QEnterEvent *event);
#else
    virtual void enterEvent(QEvent *event);
#endif
    virtual void leaveEvent(QEvent *event);
    virtual void paintEvent(QPaintEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

private:
    DTK_WIDGET_NAMESPACE::DWaterProgress *progress { nullptr };   // 左侧水球动画
    ElidedLable *lbSrcPath { nullptr };   // 左第一个label
    ElidedLable *lbDstPath { nullptr };   // 左第二个label
    QLabel *lbSpeed { nullptr };   // 右第一个label
    QLabel *lbRmTime { nullptr };   // 右第二个label
    ElidedLable *lbErrorMsg { nullptr };   // 错误信息label
    DTK_WIDGET_NAMESPACE::DIconButton *btnStop { nullptr };   // 停止按钮
    DTK_WIDGET_NAMESPACE::DIconButton *btnPause { nullptr };   // 暂停按钮
    QHBoxLayout *hLayout4 { nullptr };
    QHBoxLayout *hLayout5 { nullptr };

    QWidget *baseWid { nullptr };   // 基本信息widget

    QLabel *lbSrcIcon { nullptr };   // 冲突widget上的源文件图标
    QLabel *lbDstIcon { nullptr };   // 冲突widget上的目标文件图标
    bool createDestLabels { true };   // 冲突widget上创建目标文件的labels
    ElidedLable *lbSrcTitle { nullptr };   // 冲突widget上的源文件文件显示名称
    ElidedLable *lbDstTitle { nullptr };   // 冲突widget上的目标文件显示名称
    ElidedLable *lbSrcModTime { nullptr };   // 冲突widget上的源文件文件修改时间
    ElidedLable *lbDstModTime { nullptr };   // 冲突widget上的目标文件修改时间
    ElidedLable *lbSrcFileSize { nullptr };   // 冲突widget上的源文件文件文件大小
    ElidedLable *lbDstFileSize { nullptr };   // 冲突widget上的目标文件文件文件大小
    QWidget *widConfict { nullptr };   // 冲突widget

    QCheckBox *chkboxNotAskAgain { nullptr };   // 不在询问按钮
    DTK_WIDGET_NAMESPACE::DPushButton *btnCoexist { nullptr };   // 共存按钮
    DTK_WIDGET_NAMESPACE::DPushButton *btnSkip { nullptr };   // 跳过按钮
    DTK_WIDGET_NAMESPACE::DPushButton *btnReplace { nullptr };   // 替换、合并按钮
    DTK_WIDGET_NAMESPACE::DWarningButton *btnDelete { nullptr };   // 彻底删除文件按钮
    QWidget *widButton { nullptr };   // 按钮界面

    QVBoxLayout *rVLayout { nullptr };
    QVBoxLayout *mainLayout { nullptr };

    QAtomicInteger<bool> isPauseState { false };   // 是否是暂停状态
    QAtomicInteger<bool> isBtnHidden { false };   // 暂停、停止按钮隐藏
    QAtomicInteger<bool> isShowError { false };   // 处于错误提示状态
    QAtomicInteger<bool> isHover { false };   // 处于错误提示状态

    QTimer infoTimer;
    FileInfoPointer originInfo { nullptr };
    FileInfoPointer targetInfo { nullptr };

    QString preHoverSpeedStr;
    QString preHoverRmTimeStr;
};

}

#endif   // TASKWIDGET_H
