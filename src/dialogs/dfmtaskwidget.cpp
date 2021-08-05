/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Mike Chen <kegechen@gmail.com>
 *
 * Maintainer: Mike Chen <chenke_cm@deepin.com>
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
#include "dfmtaskwidget.h"

#include <DWaterProgress>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <DIconButton>
#include <DGuiApplicationHelper>
#include <QTimer>
#include <QException>
#include <QPainterPath>

#include "app/define.h"
#include "dfileservices.h"
#include "dabstractfileinfo.h"
#include "shutil/fileutils.h"

DWIDGET_USE_NAMESPACE

#define MSG_LABEL_WITH 350
#define SPEED_LABEL_WITH 100
#define PausedState 2

DFMElidedLable::DFMElidedLable(QWidget *parent)
    : QLabel(parent)
{

}

DFMElidedLable::~DFMElidedLable()
{

}

void DFMElidedLable::setText(const QString &text)
{
    QFontMetrics metrics(font());
    Qt::TextElideMode em = Qt::TextElideMode::ElideMiddle;

    if (!property("TextElideMode").isNull()) {
        int iem = property("TextElideMode").toInt();
        em = static_cast<Qt::TextElideMode>(iem);
    }

    QLabel::setText(metrics.elidedText(text, em, width()));
}

class DFMTaskWidgetPrivate
{
public:
    explicit DFMTaskWidgetPrivate(DFMTaskWidget *qq): q_ptr(qq)
    {
        initUI();
        initConnection();
    }
    ~DFMTaskWidgetPrivate() {}

protected:
    void initUI();
    void initConnection();

    QWidget *createConflictWidget();
    QWidget *createBtnWidget();

private:
    QString taskId;
    DWaterProgress *m_progress;
    DFMElidedLable *m_lbSrcPath;
    DFMElidedLable *m_lbDstPath;
    QLabel *m_lbSpeed;
    QLabel *m_lbRmTime;
    DFMElidedLable *m_lbErrorMsg;
    QLabel *m_lbSrcIcon;
    QLabel *m_lbDstIcon;
    DFMElidedLable *m_lbSrcTitle;
    DFMElidedLable *m_lbDstTitle;
    DFMElidedLable *m_lbSrcModTime;
    DFMElidedLable *m_lbDstModTime;
    DFMElidedLable *m_lbSrcFileSize;
    DFMElidedLable *m_lbDstFileSize;
    QWidget *m_widConfict;
    QWidget *m_widButton;

    QCheckBox *m_chkboxNotAskAgain;
    DIconButton *m_btnStop;
    DIconButton *m_btnPause;
    QPushButton *m_btnCoexist;
    QPushButton *m_btnSkip;
    QPushButton *m_btnReplace;

    QTimer *m_timer;
    bool m_isSettingValue;
    bool m_isEnableHover;
    QAtomicInteger<bool> m_handlingError = false;
    QAtomicInteger<bool> m_isPauseState = false;

    DFMTaskWidget *q_ptr;
    Q_DECLARE_PUBLIC(DFMTaskWidget)
};

QWidget *DFMTaskWidgetPrivate::createConflictWidget()
{
    Q_Q(DFMTaskWidget);
    QWidget *conflictWidget = new QWidget;
    QPalette labelPalette = q->palette();
    QColor text_color = labelPalette.text().color();
    DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(q->palette().base().color());
    if (ct == DGuiApplicationHelper::LightType) {
        text_color = DGuiApplicationHelper::adjustColor(text_color, 0, 0, 0, +20, +20, +20, 0);
    } else {
        text_color = DGuiApplicationHelper::adjustColor(text_color, 0, 0, 0, -20, -20, -20, 0);
    }
    labelPalette.setColor(QPalette::Text, text_color);

    m_lbSrcIcon = new QLabel();
    m_lbSrcIcon->setFixedSize(48, 48);
    m_lbSrcIcon->setScaledContents(true);

    m_lbSrcTitle = new DFMElidedLable();
    m_lbSrcModTime = new DFMElidedLable();
    m_lbSrcModTime->setPalette(labelPalette);

    m_lbSrcFileSize = new DFMElidedLable();
    m_lbSrcFileSize->setFixedWidth(SPEED_LABEL_WITH);
    m_lbSrcFileSize->setPalette(labelPalette);

    m_lbDstIcon = new QLabel();
    m_lbDstIcon->setFixedSize(48, 48);
    m_lbDstIcon->setScaledContents(true);

    m_lbDstTitle = new DFMElidedLable();
    m_lbDstModTime = new DFMElidedLable();
    m_lbDstModTime->setPalette(labelPalette);

    m_lbDstFileSize = new DFMElidedLable();
    m_lbDstFileSize->setFixedWidth(SPEED_LABEL_WITH);
    m_lbDstFileSize->setPalette(labelPalette);

    QGridLayout *conflictMainLayout = new QGridLayout();

    conflictMainLayout->addWidget(m_lbSrcIcon, 0, 0, 2, 1, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_lbSrcTitle, 0, 1, 1, 2, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_lbSrcModTime, 1, 1, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_lbSrcFileSize, 1, 2, Qt::AlignVCenter);

    conflictMainLayout->addWidget(m_lbDstIcon, 2, 0, 2, 1, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_lbDstTitle, 2, 1, 1, 2, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_lbDstModTime, 3, 1, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_lbDstFileSize, 3, 2, Qt::AlignVCenter);

    conflictMainLayout->setHorizontalSpacing(4);
    conflictMainLayout->setVerticalSpacing(4);
    conflictMainLayout->setContentsMargins(0, 0, 0, 0);

    conflictMainLayout->setColumnMinimumWidth(1, MSG_LABEL_WITH - 100);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addLayout(conflictMainLayout);
    hLayout->addStretch();
    conflictWidget->setLayout(hLayout);
    return conflictWidget;
}

QWidget *DFMTaskWidgetPrivate::createBtnWidget()
{
    QWidget *buttonWidget = new QWidget;
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);
    m_btnCoexist = new QPushButton(DFMTaskWidget::tr("Keep both","button"));
    m_btnSkip = new QPushButton(DFMTaskWidget::tr("Skip","button"));
    m_btnReplace = new QPushButton(DFMTaskWidget::tr("Replace","button"));
    m_btnSkip->setFocusPolicy(Qt::NoFocus);
    m_btnReplace->setFocusPolicy(Qt::NoFocus);

    m_btnCoexist->setCheckable(true);
    m_btnCoexist->setChecked(true);

    m_btnSkip->setFixedWidth(80);
    m_btnReplace->setFixedWidth(80);
    m_btnCoexist->setFixedWidth(160);

    buttonLayout->addStretch(1);
    buttonLayout->addWidget(m_btnSkip);
    buttonLayout->addWidget(m_btnReplace);
    buttonLayout->addWidget(m_btnCoexist);

    buttonLayout->setContentsMargins(0, 0, 0, 0);

    m_chkboxNotAskAgain = new QCheckBox(DFMTaskWidget::tr("Do not ask again"));
    QVBoxLayout *btnMainLayout = new QVBoxLayout;
    btnMainLayout->addSpacing(0);
    btnMainLayout->addWidget(m_chkboxNotAskAgain);
    btnMainLayout->addSpacing(0);
    btnMainLayout->addLayout(buttonLayout);
    buttonWidget->setLayout(btnMainLayout);
    return buttonWidget;
}

void DFMTaskWidgetPrivate::initUI()
{
    Q_Q(DFMTaskWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    q->setLayout(mainLayout);

    m_progress = new DWaterProgress(q);
    m_progress->setFixedSize(64, 64);
    m_progress->setValue(1);  // fix：使一开始就有一个进度显示
    m_progress->setValue(0);
    QHBoxLayout *normalLayout = new QHBoxLayout;
    normalLayout->setContentsMargins(20, 10, 20, 10);
    normalLayout->addWidget(m_progress, Qt::AlignLeft);
    normalLayout->addSpacing(20);

    m_lbSrcPath = new DFMElidedLable;
    m_lbSpeed = new QLabel;
    m_lbDstPath = new DFMElidedLable;
    m_lbRmTime = new QLabel;
    m_lbSrcPath->setFixedWidth(MSG_LABEL_WITH);
    m_lbDstPath->setFixedWidth(MSG_LABEL_WITH);
    m_lbSpeed->setFixedWidth(SPEED_LABEL_WITH);
    m_lbRmTime->setFixedWidth(SPEED_LABEL_WITH);

    QVBoxLayout *rVLayout = new QVBoxLayout;
    QHBoxLayout *hLayout1 = new QHBoxLayout;
    hLayout1->addWidget(m_lbSrcPath, Qt::AlignLeft);
    hLayout1->addSpacing(10);
    hLayout1->addWidget(m_lbSpeed, Qt::AlignRight);
    hLayout1->addStretch();

    QHBoxLayout *hLayout2 = new QHBoxLayout;
    hLayout2->addWidget(m_lbDstPath, Qt::AlignLeft);
    hLayout2->addSpacing(10);
    hLayout2->addWidget(m_lbRmTime, Qt::AlignRight);
    hLayout2->addStretch();

    rVLayout->addLayout(hLayout1);
    rVLayout->addLayout(hLayout2);

    m_lbErrorMsg = new DFMElidedLable;
    m_lbErrorMsg->setFixedWidth(MSG_LABEL_WITH + SPEED_LABEL_WITH);
    rVLayout->addWidget(m_lbErrorMsg);

    // conflict Widget
    m_widConfict = createConflictWidget();
    rVLayout->addWidget(m_widConfict);

    normalLayout->addLayout(rVLayout, 1);

    m_btnStop = new DIconButton(q);
    m_btnStop->setObjectName("StopButton");
    m_btnStop->setIcon(QIcon::fromTheme("dfm_task_stop"));
    m_btnStop->setFixedSize(24, 24);
    m_btnStop->setIconSize({24, 24});
    m_btnStop->setFlat(true);
    m_btnStop->setAttribute(Qt::WA_NoMousePropagation);

    m_btnPause = new DIconButton(q);
    m_btnPause->setIcon(QIcon::fromTheme("dfm_task_pause"));
    m_btnPause->setIconSize({24, 24});
    m_btnPause->setFixedSize(24, 24);
    m_btnPause->setFlat(true);



    normalLayout->addStretch();
    normalLayout->addWidget(m_btnPause, Qt::AlignRight);
    normalLayout->addSpacing(10);
    normalLayout->addWidget(m_btnStop, Qt::AlignRight);

    m_widButton = createBtnWidget();

    mainLayout->addLayout(normalLayout);
    mainLayout->addWidget(m_widButton);

    m_lbErrorMsg->setVisible(false);
    m_btnPause->setVisible(false);
    m_btnStop->setVisible(false);
    m_widConfict->setVisible(false);
    m_widButton->setVisible(false);

    m_timer = new QTimer(q);
    m_isSettingValue = false;
    m_isEnableHover = true;
}

void DFMTaskWidgetPrivate::initConnection()
{
    Q_Q(DFMTaskWidget);
    QObject::connect(m_btnSkip, &QPushButton::clicked, q, [q, this]() {
        m_widConfict->hide();
        m_widButton->hide();
        q->showConflictButtons(false);

        emit q->butonClicked(DFMTaskWidget::SKIP);
    });
    QObject::connect(m_btnReplace, &QPushButton::clicked, q, [q, this]() {
        m_widConfict->hide();
        m_widButton->hide();
        q->showConflictButtons(false);
        emit q->butonClicked(DFMTaskWidget::REPLACE);
    });
    QObject::connect(m_btnCoexist, &QPushButton::clicked, q, [q, this]() {
        m_widConfict->hide();
        m_widButton->hide();
        q->showConflictButtons(false);
        emit q->butonClicked(DFMTaskWidget::COEXIST);
    });

    QObject::connect(m_btnPause, &QPushButton::clicked, q, [q]() {
        emit q->butonClicked(DFMTaskWidget::PAUSE);
    });
    QObject::connect(m_btnStop, &QPushButton::clicked, q, [q, this]() {
        m_widConfict->hide();
        m_widButton->hide();
        q->showConflictButtons(false);
        emit q->butonClicked(DFMTaskWidget::STOP);
    }, Qt::DirectConnection);

    QObject::connect(m_timer, &QTimer::timeout, [this]() {
        m_isSettingValue = false;
    });
}


////////////////////////////////////////////
// class DFMTaskWidget
////////////////////////////////////////////
DFMTaskWidget::DFMTaskWidget(QWidget *parent)
    : QWidget(parent)
    , d_private(new DFMTaskWidgetPrivate(this))
{
    setMouseTracking(true);
    //fix bug 63004 不显示操作按钮调整widget的高度
    showConflictButtons(false);
}

DFMTaskWidget::~DFMTaskWidget()
{

}

QString DFMTaskWidget::taskId()
{
    return d_func()->taskId;
}

void DFMTaskWidget::setTaskId(const QString &taskId)
{
    d_func()->taskId = taskId;
}

void DFMTaskWidget::setMsgText(const QString &srcMsg, const QString &dstMsg)
{
    Q_D(DFMTaskWidget);
    d->m_lbSrcPath->setText(srcMsg);
    d->m_lbDstPath->setText(dstMsg);
}

void DFMTaskWidget::setSpeedText(const QString &speed, const QString &rmtime)
{
    Q_D(DFMTaskWidget);
    if(d->m_progress->value() >= 100)
    {
        //! 进度条100%后任务对话框中显示数据同步种与即将完成
        d->m_lbSpeed->setText(tr("Syncing data"));
        d->m_lbRmTime->setText(tr("Please wait"));
    }
    else {
        d->m_lbSpeed->setText(speed);
        d->m_lbRmTime->setText(rmtime);
    }
}

void DFMTaskWidget::setProgressValue(int value)
{
    Q_D(DFMTaskWidget);

    if (value > 100) {
        value = 100;
    }

    //大量快速的设置进度条会导致进度数值不刷新
    //这里通过对相等值判断和定时器降低刷新频率
    //并且手动调用update强制刷新界面
    if (d->m_isSettingValue && value != 100) { // fix : 删除文件最后要显示100%
        return;
    }

    if (value > 0 && value == d->m_progress->value()) {
        return;
    }

    d->m_timer->start(100);
    d->m_isSettingValue = true;
    if (value >= 0 && d->m_progress->value() == 0) {
        d->m_progress->start();
        d->m_progress->setValue(value);
        return;
    }

    if (value < 0) {
        d->m_progress->stop();
        d->m_isSettingValue = false;
        d->m_timer->stop();
    } else {
        d->m_progress->setValue(value);
        d->m_progress->update();
    }
}

void DFMTaskWidget::setErrorMsg(const QString &err)
{
    Q_D(DFMTaskWidget);
    d->m_lbErrorMsg->setText(err);
    d->m_lbErrorMsg->setHidden(err.isEmpty());
    d->m_widButton->setHidden(err.isEmpty());

    if (!err.isEmpty()) {
        d->m_widConfict->hide();
        hideButton(COEXIST);
        setButtonText(REPLACE, tr("Retry","button"));
        showConflictButtons(true, false);
    }
}

void DFMTaskWidget::setConflictMsg(const DUrl &src, const DUrl &dst)
{
    Q_D(DFMTaskWidget);
    qDebug() << src << dst ;
    DAbstractFileInfoPointer originInfo = fileService->createFileInfo(nullptr, src);
    DAbstractFileInfoPointer targetInfo = fileService->createFileInfo(nullptr, dst);
    if (originInfo && targetInfo) {
        d->m_lbSrcIcon->setPixmap(originInfo->fileIcon().pixmap(48, 48));
        d->m_lbSrcModTime->setText(QString(tr("Time modified: %1")).arg(originInfo->lastModifiedDisplayName()));
        if (originInfo->isDir()) {
            d->m_lbSrcTitle->setText(tr("Original folder"));
            d->m_lbSrcFileSize->setText(QString(tr("Contains: %1")).arg(originInfo->sizeDisplayName()));
        } else {
            d->m_lbSrcTitle->setText(tr("Original file"));
            d->m_lbSrcFileSize->setText(QString(tr("Size: %1")).arg(originInfo->sizeDisplayName()));
        }

        d->m_lbDstIcon->setPixmap(targetInfo->fileIcon().pixmap(48, 48));
        d->m_lbDstModTime->setText(QString(tr("Time modified: %1")).arg(targetInfo->lastModifiedDisplayName()));

        if (targetInfo->isDir()) {
            d->m_lbDstTitle->setText(tr("Target folder"));
            d->m_lbDstFileSize->setText(QString(tr("Contains: %1")).arg(targetInfo->sizeDisplayName()));
        } else {
            d->m_lbDstTitle->setText(tr("Target file"));
            d->m_lbDstFileSize->setText(QString(tr("Size: %1")).arg(targetInfo->sizeDisplayName()));
        }

        d->m_widConfict->show();
        d->m_widButton->show();
        hideButton(COEXIST, false);
        showConflictButtons();
    }
}

void DFMTaskWidget::setButtonText(DFMTaskWidget::BUTTON bt, const QString &text)
{
    QAbstractButton *btn = getButton(bt);
    if (btn) {
        btn->setText(text);
    }
}

void DFMTaskWidget::setHoverEnable(bool enable)
{
    Q_D(DFMTaskWidget);
    d->m_isEnableHover = enable;
}

void DFMTaskWidget::hideButton(DFMTaskWidget::BUTTON bt, bool hidden/*=true*/)
{
    Q_D(DFMTaskWidget);
    if (!d->m_widButton) {
        qWarning() << "init failed!!";
        return;
    }

    QAbstractButton *btn = getButton(bt);
    if (btn) {
        btn->setHidden(hidden);
    }
}

void DFMTaskWidget::setHandleingError(const bool &handleing)
{
    Q_D(DFMTaskWidget);

    d->m_handlingError = handleing;
}

bool DFMTaskWidget::isHandleingError() const
{
    Q_D(const DFMTaskWidget);
    return d->m_handlingError.load();
}

void DFMTaskWidget::onMouseHover(bool hover)
{
    Q_D(DFMTaskWidget);
    if (d->m_isEnableHover) {
        d->m_btnPause->setVisible(hover);
        d->m_btnStop->setVisible(hover);

        d->m_lbSpeed->setHidden(hover);
        d->m_lbRmTime->setHidden(hover);

        update(rect());
        setProperty("isHover", hover);
        emit hoverChanged(hover);
    }
}

void DFMTaskWidget::showConflictButtons(bool showBtns/*=true*/, bool showConflict/*=true*/)
{
    Q_D(DFMTaskWidget);
    if (!d->m_widConfict) {
        return;
    }

    qInfo() << showBtns << d->m_widButton->sizeHint().height() << d->m_widConfict->sizeHint().height();
    int h = 100;
    if (showBtns) {
        h += d->m_widButton->sizeHint().height();
        if (showConflict) {
            h += d->m_widConfict->sizeHint().height();
        }
    }

    setFixedHeight(h);
    emit heightChanged();
}

QAbstractButton *DFMTaskWidget::getButton(DFMTaskWidget::BUTTON bt)
{
    Q_D(DFMTaskWidget);
    QAbstractButton *btn = nullptr;
    switch (bt) {
    case PAUSE:
        btn = d->m_btnPause;
        break;
    case STOP:
        btn = d->m_btnStop;
        break;
    case SKIP:
        btn = d->m_btnSkip;
        break;
    case REPLACE:
        btn = d->m_btnReplace;
        break;
    case COEXIST:
        btn = d->m_btnCoexist;
        break;
    case CHECKBOX_NOASK:
        btn = d->m_chkboxNotAskAgain;
        break;
    }

    return btn;
}

void DFMTaskWidget::progressStart()
{
    Q_D(DFMTaskWidget);

    d->m_progress->start();
}

void DFMTaskWidget::enterEvent(QEvent *event)
{
    onMouseHover(true);

    return QWidget::enterEvent(event);
}

void DFMTaskWidget::leaveEvent(QEvent *event)
{
    onMouseHover(false);

    return QWidget::enterEvent(event);
}

void DFMTaskWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    if (opt.state & QStyle::State_MouseOver) {
        int radius = 8;
        QRectF bgRect;
        bgRect.setSize(size());
        QPainterPath path;
        path.addRoundedRect(bgRect, radius, radius);
        QColor bgColor;
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            bgColor = QColor(0, 0, 0, 13); //rgba(0,0,0,13); border-radius: 8px
        } else {
            bgColor = QColor(255, 255, 255, 13); //rgba(255,255,255,13); border-radius: 8px
        }

        // drawbackground color
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillPath(path, bgColor);
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.restore();
    } else if (property("drawSeparator").toBool()) {
        QPoint p1, p2;
        p1 = opt.rect.topLeft();
        p2 = opt.rect.topRight();
        QPen oldPen = painter.pen();
        painter.setPen(QPen(opt.palette.brush(foregroundRole()), 1));
        painter.drawLine(p1, p2);
        painter.setPen(oldPen);
    }

    QWidget::paintEvent(event);
}

void DFMTaskWidget::onProgressChanged(qreal progress, qint64 writeData)
{
    Q_UNUSED(writeData);

    setProgressValue(static_cast<int>(progress * 100));
    setProperty("progress", progress);
}

void DFMTaskWidget::onBurnProgressChanged(qreal progress, qint64 writeData)
{
    Q_UNUSED(writeData);

    //fixed:progress*100  -> progress;共性问题，在光驱刻录显示百分比时，由于底层disomaster库返回给上层就是乘以100了，所以上层不想要再乘以100来处理。
    setProgressValue(static_cast<int>(progress));
    setProperty("progress", progress);
}

void DFMTaskWidget::onSpeedUpdated(qint64 speed)
{
    Q_D(DFMTaskWidget);
    QString sp = FileUtils::formatSize(speed) + "/s";
    d->m_lbSpeed->setText(sp);

    //setProperty("speed", speed);
    // assert(speed!=0)  (totalsize - currentsize)/speed
    //m_remainLabel->setText(m_remainMessage);
}

void DFMTaskWidget::onStateChanged(int state)
{
    Q_D(DFMTaskWidget);
    if ((PausedState == state) == d->m_isPauseState) {
        return;
    }
    if (state == PausedState) {
        d->m_isPauseState = true;
        d->m_btnPause->setIcon(QIcon::fromTheme("dfm_task_start"));
        d->m_progress->stop();
    } else {
        d->m_isPauseState = false;
        d->m_btnPause->setIcon(QIcon::fromTheme("dfm_task_pause"));
        d->m_progress->start();
    }
}

