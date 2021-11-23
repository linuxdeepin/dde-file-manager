/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<zhangsheng@uniontech.com>
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

#include "dfm_common_service_global.h"

#include <QWidget>
#include <QLabel>

#include <DWaterProgress>
#include <DIconButton>
class QPushButton;
class QCheckBox;
DWIDGET_USE_NAMESPACE
DSC_BEGIN_NAMESPACE
class ElidedLable : public QLabel
{
    Q_OBJECT
public:
    explicit ElidedLable(QWidget *parent = nullptr);
    virtual ~ElidedLable();
    void setText(const QString &text);
};

class TaskWidget : public QWidget
{
    Q_OBJECT
public:
    enum class BUTTON : uint8_t {
        kPause,   // 暂停
        kStop,   // 停止
        kSkip,   // 跳过
        kReplace,   // 替换
        kCoexist,   // 共存
        kCheckBoxNoAsk   // 记住选项
    };
    Q_ENUM(BUTTON)
    explicit TaskWidget(QWidget *parent = nullptr);
    ~TaskWidget();

signals:
    void butonClicked(BUTTON btn);
    void heightChanged();
    void hoverChanged(bool);
private slots:
    void onButtonClicked();
    void onTimerTimeOut();

protected:
    void initUI();
    void initConnection();

    QWidget *createConflictWidget();
    QWidget *createBtnWidget();

    void showConflictButtons(bool showBtns = true, bool showConflict = true);

private:
    DWaterProgress *progress = nullptr;
    ElidedLable *lbSrcPath = nullptr;
    ElidedLable *lbDstPath = nullptr;
    QLabel *lbSpeed = nullptr;
    QLabel *lbRmTime = nullptr;
    ElidedLable *lbErrorMsg = nullptr;
    QLabel *lbSrcIcon = nullptr;
    QLabel *lbDstIcon = nullptr;
    ElidedLable *lbSrcTitle = nullptr;
    ElidedLable *lbDstTitle = nullptr;
    ElidedLable *lbSrcModTime = nullptr;
    ElidedLable *lbDstModTime = nullptr;
    ElidedLable *lbSrcFileSize = nullptr;
    ElidedLable *lbDstFileSize = nullptr;
    QWidget *widConfict = nullptr;
    QWidget *widButton = nullptr;

    QCheckBox *chkboxNotAskAgain = nullptr;
    DIconButton *btnStop = nullptr;
    DIconButton *btnPause = nullptr;
    QPushButton *btnCoexist = nullptr;
    QPushButton *btnSkip = nullptr;
    QPushButton *btnReplace = nullptr;

    QTimer *timer = nullptr;
    bool isSettingValue { false };
    bool isEnableHover { false };
};

DSC_END_NAMESPACE
#endif   // TASKWIDGET_H
