/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#include "taskwidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QTimer>

#include <DWaterProgress>
#include <DIconButton>

DWIDGET_USE_NAMESPACE

DSC_USE_NAMESPACE
const static int kMsgLabelWidth = 350;
const static int kSpeedLabelWidth = 100;
static const char *const kBtnPropertyName = "btnType";

ElidedLable::ElidedLable(QWidget *parent)
    : QLabel(parent)
{
}

ElidedLable::~ElidedLable() {}
/*!
 * \brief ElidedLable::setText 设置当前文字的内容
 * \param text
 */
void ElidedLable::setText(const QString &text)
{
    QFontMetrics metrics(font());
    Qt::TextElideMode em = Qt::TextElideMode::ElideMiddle;

    if (!property("TextElideMode").isNull()) {
        int iem = property("TextElideMode").toInt();
        em = static_cast<Qt::TextElideMode>(iem);
    }

    QLabel::setText(metrics.elidedText(text, em, width()));
}

TaskWidget::TaskWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    showConflictButtons(false);
}

TaskWidget::~TaskWidget() {}

void TaskWidget::onButtonClicked()
{
    QObject *obj = sender();
    if (!obj)
        return;
    BUTTON button = obj->property(kBtnPropertyName).value<BUTTON>();
    showConflictButtons(button == BUTTON::kPause);
    emit butonClicked(button);
}

void TaskWidget::onTimerTimeOut()
{
    isSettingValue = false;
}

void TaskWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    progress = new DWaterProgress(this);
    progress->setFixedSize(64, 64);
    progress->setValue(1);   // fix：使一开始就有一个进度显示
    progress->setValue(0);
    QHBoxLayout *normalLayout = new QHBoxLayout;
    normalLayout->setContentsMargins(20, 10, 20, 10);
    normalLayout->addWidget(progress, Qt::AlignLeft);
    normalLayout->addSpacing(20);

    lbSrcPath = new ElidedLable;
    lbSpeed = new QLabel;
    lbDstPath = new ElidedLable;
    lbRmTime = new QLabel;
    lbSrcPath->setFixedWidth(kMsgLabelWidth);
    lbDstPath->setFixedWidth(kMsgLabelWidth);
    lbSpeed->setFixedWidth(kSpeedLabelWidth);
    lbRmTime->setFixedWidth(kSpeedLabelWidth);

    QVBoxLayout *rVLayout = new QVBoxLayout;
    QHBoxLayout *hLayout1 = new QHBoxLayout;
    hLayout1->addWidget(lbSrcPath, Qt::AlignLeft);
    hLayout1->addSpacing(10);
    hLayout1->addWidget(lbSpeed, Qt::AlignRight);
    hLayout1->addStretch();

    QHBoxLayout *hLayout2 = new QHBoxLayout;
    hLayout2->addWidget(lbDstPath, Qt::AlignLeft);
    hLayout2->addSpacing(10);
    hLayout2->addWidget(lbRmTime, Qt::AlignRight);
    hLayout2->addStretch();

    rVLayout->addLayout(hLayout1);
    rVLayout->addLayout(hLayout2);

    lbErrorMsg = new ElidedLable;
    lbErrorMsg->setFixedWidth(kMsgLabelWidth + kSpeedLabelWidth);
    rVLayout->addWidget(lbErrorMsg);

    widConfict = createConflictWidget();
    rVLayout->addWidget(widConfict);

    normalLayout->addLayout(rVLayout, 1);

    btnStop = new DIconButton(this);
    btnStop->setObjectName("TaskWidgetStopButton");
    QVariant variantStop;
    variantStop.setValue<BUTTON>(BUTTON::kStop);
    btnStop->setProperty(kBtnPropertyName, variantStop);
    btnStop->setIcon(QIcon::fromTheme("dfm_task_stop"));
    btnStop->setFixedSize(24, 24);
    btnStop->setIconSize({ 24, 24 });
    btnStop->setFlat(true);
    btnStop->setAttribute(Qt::WA_NoMousePropagation);

    btnPause = new DIconButton(this);
    btnPause->setObjectName("TaskWidgetPauseButton");
    QVariant variantPause;
    variantPause.setValue<BUTTON>(BUTTON::kPause);
    btnPause->setProperty(kBtnPropertyName, variantPause);
    btnPause->setIcon(QIcon::fromTheme("dfm_task_pause"));
    btnPause->setIconSize({ 24, 24 });
    btnPause->setFixedSize(24, 24);
    btnPause->setFlat(true);

    normalLayout->addStretch();
    normalLayout->addWidget(btnPause, Qt::AlignRight);
    normalLayout->addSpacing(10);
    normalLayout->addWidget(btnStop, Qt::AlignRight);

    widButton = createBtnWidget();

    mainLayout->addLayout(normalLayout);
    mainLayout->addWidget(widButton);

    lbErrorMsg->setVisible(false);
    btnPause->setVisible(false);
    btnStop->setVisible(false);
    widConfict->setVisible(false);
    widButton->setVisible(false);

    timer = new QTimer(this);
    isSettingValue = false;
    isEnableHover = true;
}

void TaskWidget::initConnection()
{
    connect(btnSkip, &QPushButton::clicked, this, &TaskWidget::onButtonClicked);
    connect(btnReplace, &QPushButton::clicked, this, &TaskWidget::onButtonClicked);
    connect(btnCoexist, &QPushButton::clicked, this, &TaskWidget::onButtonClicked);
    connect(btnPause, &QPushButton::clicked, this, &TaskWidget::onButtonClicked);
    connect(btnStop, &QPushButton::clicked, this, &TaskWidget::onButtonClicked,
            Qt::DirectConnection);

    QObject::connect(timer, &QTimer::timeout, this, &TaskWidget::onTimerTimeOut);
}

QWidget *TaskWidget::createConflictWidget()
{
    QWidget *conflictWidget = new QWidget;
    QPalette labelPalette = palette();
    QColor text_color = labelPalette.text().color();
    labelPalette.setColor(QPalette::Text, text_color);

    lbSrcIcon = new QLabel();
    lbSrcIcon->setFixedSize(48, 48);
    lbSrcIcon->setScaledContents(true);

    lbSrcTitle = new ElidedLable();
    lbSrcModTime = new ElidedLable();
    lbSrcModTime->setPalette(labelPalette);

    lbSrcFileSize = new ElidedLable();
    lbSrcFileSize->setFixedWidth(kSpeedLabelWidth);
    lbSrcFileSize->setPalette(labelPalette);

    lbDstIcon = new QLabel();
    lbDstIcon->setFixedSize(48, 48);
    lbDstIcon->setScaledContents(true);

    lbDstTitle = new ElidedLable();
    lbDstModTime = new ElidedLable();
    lbDstModTime->setPalette(labelPalette);

    lbDstFileSize = new ElidedLable();
    lbDstFileSize->setFixedWidth(kSpeedLabelWidth);
    lbDstFileSize->setPalette(labelPalette);

    QGridLayout *conflictMainLayout = new QGridLayout();

    conflictMainLayout->addWidget(lbSrcIcon, 0, 0, 2, 1, Qt::AlignVCenter);
    conflictMainLayout->addWidget(lbSrcTitle, 0, 1, 1, 2, Qt::AlignVCenter);
    conflictMainLayout->addWidget(lbSrcModTime, 1, 1, Qt::AlignVCenter);
    conflictMainLayout->addWidget(lbSrcFileSize, 1, 2, Qt::AlignVCenter);

    conflictMainLayout->addWidget(lbDstIcon, 2, 0, 2, 1, Qt::AlignVCenter);
    conflictMainLayout->addWidget(lbDstTitle, 2, 1, 1, 2, Qt::AlignVCenter);
    conflictMainLayout->addWidget(lbDstModTime, 3, 1, Qt::AlignVCenter);
    conflictMainLayout->addWidget(lbDstFileSize, 3, 2, Qt::AlignVCenter);

    conflictMainLayout->setHorizontalSpacing(4);
    conflictMainLayout->setVerticalSpacing(4);
    conflictMainLayout->setContentsMargins(0, 0, 0, 0);

    conflictMainLayout->setColumnMinimumWidth(1, kMsgLabelWidth - 100);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addLayout(conflictMainLayout);
    hLayout->addStretch();
    conflictWidget->setLayout(hLayout);
    return conflictWidget;
}

QWidget *TaskWidget::createBtnWidget()
{
    QWidget *buttonWidget = new QWidget;
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);

    QVariant variantCoexit;
    variantCoexit.setValue<BUTTON>(BUTTON::kCoexist);
    btnCoexist = new QPushButton(TaskWidget::tr("Keep both", "button"));
    btnCoexist->setProperty(kBtnPropertyName, variantCoexit);
    btnSkip = new QPushButton(TaskWidget::tr("Skip", "button"));

    QVariant variantSkip;
    variantSkip.setValue<BUTTON>(BUTTON::kSkip);
    btnSkip->setProperty(kBtnPropertyName, variantSkip);
    btnReplace = new QPushButton(TaskWidget::tr("Replace", "button"));
    QVariant variantReplace;
    variantReplace.setValue<BUTTON>(BUTTON::kReplace);
    btnReplace->setProperty(kBtnPropertyName, variantReplace);
    btnSkip->setFocusPolicy(Qt::NoFocus);
    btnReplace->setFocusPolicy(Qt::NoFocus);

    btnCoexist->setCheckable(true);
    btnCoexist->setChecked(true);

    btnSkip->setFixedWidth(80);
    btnReplace->setFixedWidth(80);
    btnCoexist->setFixedWidth(160);

    buttonLayout->addStretch(1);
    buttonLayout->addWidget(btnSkip);
    buttonLayout->addWidget(btnReplace);
    buttonLayout->addWidget(btnCoexist);

    buttonLayout->setContentsMargins(0, 0, 0, 0);

    chkboxNotAskAgain = new QCheckBox(TaskWidget::tr("Do not ask again"));
    QVBoxLayout *btnMainLayout = new QVBoxLayout;
    btnMainLayout->addSpacing(0);
    btnMainLayout->addWidget(chkboxNotAskAgain);
    btnMainLayout->addSpacing(0);
    btnMainLayout->addLayout(buttonLayout);
    buttonWidget->setLayout(btnMainLayout);
    return buttonWidget;
}
void TaskWidget::showConflictButtons(bool showBtns, bool showConflict)
{
    if (!widConfict) {
        return;
    }

    int h = 100;
    if (showBtns) {
        h += widButton->sizeHint().height();
        if (showConflict) {
            h += widConfict->sizeHint().height();
        }
    }

    setFixedHeight(h);
    emit heightChanged();
}
