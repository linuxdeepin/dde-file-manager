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

#include "dtaskdialog.h"
#include "utils.h"
#include <dcircleprogress.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QCloseEvent>

#include <ddialog.h>

#include "dfmglobal.h"
#include "disomaster.h"
#include "dfileservices.h"
#include "dabstractfileinfo.h"
#include "fileoperations/filejob.h"

#include "xutil.h"
#include "app/define.h"
#include "shutil/fileutils.h"
#include "dialogs/dialogmanager.h"
#include "singleton.h"

class ErrorHandle : public DFileCopyMoveJob::Handle
{
public:
    ErrorHandle(MoveCopyTaskWidget *taskWidget)
        : m_taskWidget(taskWidget)
    {

    }

    DFileCopyMoveJob::Action handleError(DFileCopyMoveJob *job, DFileCopyMoveJob::Error error,
                                         const DAbstractFileInfo *sourceInfo,
                                         const DAbstractFileInfo *targetInfo) override
    {
        Q_UNUSED(sourceInfo)
        Q_UNUSED(targetInfo)

        if (m_actionOfError != DFileCopyMoveJob::NoAction) {
            DFileCopyMoveJob::Action action = m_actionOfError;
            m_actionOfError = DFileCopyMoveJob::NoAction;

            return action;
        }

        switch (error) {
        case DFileCopyMoveJob::FileExistsError:
        case DFileCopyMoveJob::DirectoryExistsError:
            if (sourceInfo->fileUrl() == targetInfo->fileUrl()) {
                return DFileCopyMoveJob::CoexistAction;
            }
            job->togglePause();
            m_taskWidget->updateMessageByJob();
            m_taskWidget->showConflict();
            break;
        case DFileCopyMoveJob::UnknowUrlError: {
            DDialog dialog("error", QCoreApplication::translate("DTaskDialog", "This action is not supported"));
            dialog.setIcon(QIcon::fromTheme("dialog-error"), QSize(64, 64));
            dialog.exec();
        }
        // fall-through
        case DFileCopyMoveJob::UnknowError:
            return DFileCopyMoveJob::CancelAction;
        default:
            job->togglePause();
            m_taskWidget->updateMessageByJob();
            m_taskWidget->showButtonFrame();
            break;
        }

        return DFileCopyMoveJob::NoAction;
    }

    MoveCopyTaskWidget *m_taskWidget;
    DFileCopyMoveJob::Action m_actionOfError = DFileCopyMoveJob::NoAction;
};

MoveCopyTaskWidget::MoveCopyTaskWidget(const QMap<QString, QString> &jobDetail, QWidget *parent):
    QFrame(parent),
    m_jobDetail(jobDetail)
{
    initUI();
    initConnect();
    if (m_jobDetail.contains("target")) {
        setTargetObj(m_jobDetail.value("target"));
    }

    if (m_jobDetail.contains("destination")) {
        setDestinationObj(m_jobDetail.value("destination"));
    }
//    this->setStyleSheet("border:1px solid green");
}

MoveCopyTaskWidget::MoveCopyTaskWidget(DFileCopyMoveJob *job, QWidget *parent)
    : QFrame(parent)
    , m_fileJob(job)
    , m_errorHandle(new ErrorHandle(this))
    , m_jobInfo(new JobInfo())
{
    initUI();

    job->setErrorHandle(m_errorHandle, thread());

    connect(job, &DFileCopyMoveJob::progressChanged, this, &MoveCopyTaskWidget::onJobProgressChanged);
    connect(job, &DFileCopyMoveJob::speedUpdated, this, &MoveCopyTaskWidget::onJobSpeedChanged);
    connect(job, &DFileCopyMoveJob::currentJobChanged, this, &MoveCopyTaskWidget::onJobCurrentJobChanged);
    connect(job, &DFileCopyMoveJob::fileStatisticsFinished, this, [this, job] {
        m_jobInfo->totalDataSize = job->totalDataSize();
        updateMessageByJob();
    });
    connect(job, &DFileCopyMoveJob::stateChanged, this, [this](DFileCopyMoveJob::State state) {
        m_animatePad->setPauseState(state == DFileCopyMoveJob::PausedState);
    });
    connect(job, &DFileCopyMoveJob::errorChanged, this, [this](DFileCopyMoveJob::Error error) {
        m_animatePad->setCanPause(error == DFileCopyMoveJob::NoError);
    }, Qt::QueuedConnection);
    connect(m_closeButton, &QPushButton::clicked, job, &DFileCopyMoveJob::stop);
//    connect(m_pauseBuuton, &QPushButton::clicked, job, &DFileCopyMoveJob::togglePause);
    connect(m_skipButton, &QPushButton::clicked, this, [this] {
        disposeJobError(DFileCopyMoveJob::SkipAction);
    });
    connect(m_keepBothButton, &QPushButton::clicked, this, [this] {
        disposeJobError(DFileCopyMoveJob::CoexistAction);
    });
    connect(m_replaceButton, &QPushButton::clicked, this, [this] {
        if (m_fileJob->error() == DFileCopyMoveJob::DirectoryExistsError)
        {
            disposeJobError(DFileCopyMoveJob::MergeAction);
        } else if (m_fileJob->error() == DFileCopyMoveJob::FileExistsError)
        {
            disposeJobError(DFileCopyMoveJob::ReplaceAction);
        } else
        {
            disposeJobError(DFileCopyMoveJob::RetryAction);
        }
    });
    connect(m_animatePad, &CircleProgressAnimatePad::clicked, job, &DFileCopyMoveJob::togglePause);

    m_jobInfo->totalDataSize = job->totalDataSize();

    if (!m_fileJob->fileStatisticsIsFinished()) {
        m_animatePad->startAnimation();
    }

    m_animatePad->setCanPause(m_fileJob->error() == DFileCopyMoveJob::NoError);
}

MoveCopyTaskWidget::~MoveCopyTaskWidget()
{
    if (m_errorHandle) {
        delete m_errorHandle;
    }

    if (m_jobInfo) {
        delete m_jobInfo;
    }
}

void MoveCopyTaskWidget::initUI()
{

    m_bgLabel = new QLabel(this);
    m_bgLabel->setObjectName("Background");
    m_bgLabel->setAutoFillBackground(true);
    m_bgLabel->setWindowFlags(Qt::WindowStaysOnBottomHint);

    m_closeButton = new QPushButton;
    m_closeButton->setObjectName("StopButton");
    m_closeButton->setFixedSize(24, 24);
    m_closeButton->setAttribute(Qt::WA_NoMousePropagation);

    m_animatePad = new CircleProgressAnimatePad;
    m_animatePad->setFixedSize(54, 54);
    m_animatePad->setBackgroundColor(QColor("#E9E9E9"));
    m_animatePad->setChunkColor(QColor("#3cadff"));
    m_animatePad->setLineWidth(3);
    m_animatePad->setFontSize(14);
    m_animatePad->setCurrentValue(0);

    m_closeButton->hide();
    setMouseTracking(true);

    m_speedLabel = new QLabel;
    m_remainLabel = new QLabel;
    m_speedLabel->setFixedHeight(18);
    m_remainLabel->setFixedHeight(18);
    m_speedLabel->setObjectName("TaskTipMessageLabel");
    m_remainLabel->setObjectName("TaskTipMessageLabel");

    m_msg1Label = new QLabel;
    m_msg2Label = new QLabel;
    m_msg1Label->setFixedHeight(22);
    m_msg2Label->setFixedHeight(22);
    m_msg1Label->setObjectName("MessageLabel1");
    m_msg2Label->setObjectName("MessageLabel2");

    if (m_fileJob) {
        m_errorLabel = new QLabel(this);
        m_errorLabel->setObjectName("ErrorLabel");
    }

    QGridLayout *msgGridLayout = new QGridLayout;
    msgGridLayout->addWidget(m_msg1Label, 0, 0, Qt::AlignVCenter);

    msgGridLayout->addWidget(m_speedLabel, 0, 1, Qt::AlignRight | Qt::AlignVCenter);
    msgGridLayout->addWidget(m_msg2Label, 1, 0, Qt::AlignVCenter);

    if (m_errorLabel) {
        msgGridLayout->addWidget(m_errorLabel, 2, 0, Qt::AlignVCenter);
    }

    msgGridLayout->addWidget(m_remainLabel, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
    msgGridLayout->setColumnMinimumWidth(0, 385);
    msgGridLayout->setColumnStretch(0, 1);
    msgGridLayout->setHorizontalSpacing(5);

    initConflictDetailFrame();
    initButtonFrame();

    m_buttonFrame->setAttribute(Qt::WA_AlwaysStackOnTop);

    m_lineLabel = new QFrame;
    m_lineLabel->setFixedHeight(1);
    m_lineLabel->setObjectName("LineLabel");
    m_lineLabel->hide();

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addStretch(1);
    rightLayout->addLayout(msgGridLayout);
    rightLayout->addWidget(m_conflictFrame);
    rightLayout->addSpacing(20);

    rightLayout->addWidget(m_buttonFrame);

    rightLayout->addStretch(1);
    rightLayout->addWidget(m_lineLabel, 0, Qt::AlignBottom);
    rightLayout->setSpacing(0);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_animatePad);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(rightLayout);
    mainLayout->addSpacing(5);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_closeButton, 0, Qt::AlignCenter);
    mainLayout->addSpacing(20);
    setLayout(mainLayout);
    setFixedHeight(80);

    m_conflictFrame->hide();
    m_buttonFrame->hide();
    m_conflictFrame->hide();
}


void MoveCopyTaskWidget::initConflictDetailFrame()
{

    m_conflictFrame = new QFrame(this);

    m_originIconLabel = new QLabel(this);
//    m_originIconLabel->setStyleSheet("background-color: green");
    m_originIconLabel->setFixedSize(48, 48);
    m_originIconLabel->setScaledContents(true);

    m_originTitleLabel = new QLabel(this);
//    m_originTitleLabel->setStyleSheet("background-color: yellow");
    m_originTitleLabel->setFixedHeight(20);

    m_originTimeLabel = new QLabel(this);
//    m_originTimeLabel->setStyleSheet("background-color: gray");
    m_originTimeLabel->setFixedHeight(20);

    m_originSizeLabel = new QLabel(this);
//    m_originSizeLabel->setStyleSheet("background-color: green");
    m_originSizeLabel->setFixedSize(90, 20);

    m_targetIconLabel = new QLabel(this);
//    m_targetIconLabel->setStyleSheet("background-color: red");
    m_targetIconLabel->setFixedSize(48, 48);
    m_targetIconLabel->setScaledContents(true);

    m_targetTitleLabel = new QLabel(this);
//    m_targetTitleLabel->setStyleSheet("background-color: yellow");
    m_targetTitleLabel->setFixedHeight(20);

    m_targetTimeLabel = new QLabel(this);
//    m_targetTimeLabel->setStyleSheet("background-color: gray");
    m_targetTimeLabel->setFixedHeight(20);

    m_targetSizeLabel = new QLabel(this);
//    m_targetSizeLabel->setStyleSheet("background-color: green");
    m_targetSizeLabel->setFixedSize(90, 20);

    QGridLayout *conflictMainLayout = new QGridLayout(this);

    conflictMainLayout->addWidget(m_originIconLabel, 0, 0, 2, 1, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_originTitleLabel, 0, 1, 1, 2, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_originTimeLabel, 1, 1, 1, 1, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_originSizeLabel, 1, 2, 1, 1, Qt::AlignVCenter);


    conflictMainLayout->addWidget(m_targetIconLabel, 2, 0, 2, 1, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_targetTitleLabel, 2, 1, 1, 2, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_targetTimeLabel, 3, 1, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_targetSizeLabel, 3, 2, Qt::AlignVCenter);

    conflictMainLayout->setHorizontalSpacing(4);
    conflictMainLayout->setVerticalSpacing(4);
    conflictMainLayout->setContentsMargins(0, 0, 0, 0);

    m_conflictFrame->setLayout(conflictMainLayout);
    m_conflictFrame->setFixedHeight(120);

}



void MoveCopyTaskWidget::initButtonFrame()
{
    m_buttonFrame = new QFrame;

    m_buttonGroup = new QButtonGroup;
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);
    m_keepBothButton = new QPushButton(tr("Keep both"));
    m_skipButton = new QPushButton(tr("Skip"));
    m_replaceButton = new QPushButton(tr("Replace"));
    m_skipButton->setFocusPolicy(Qt::NoFocus);
    m_replaceButton->setFocusPolicy(Qt::NoFocus);

    m_keepBothButton->setProperty("code", 0);
    m_replaceButton->setProperty("code", 1);
    m_skipButton->setProperty("code", 2);

    m_keepBothButton->setObjectName("OptionButton");
    m_replaceButton->setObjectName("OptionButton");
    m_skipButton->setObjectName("OptionButton");
    m_keepBothButton->setCheckable(true);
    m_keepBothButton->setChecked(true);

    buttonLayout->addWidget(m_skipButton);
    buttonLayout->addWidget(m_replaceButton);
    buttonLayout->addWidget(m_keepBothButton);
    buttonLayout->addStretch(1);

    buttonLayout->setContentsMargins(0, 0, 0, 0);

    m_checkBox = new QCheckBox(tr("Do not ask again"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addSpacing(0);
    mainLayout->addWidget(m_checkBox);
    mainLayout->addSpacing(0);
    mainLayout->addLayout(buttonLayout);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonFrame->setLayout(mainLayout);
    m_buttonFrame->setFixedHeight(60);
}

DFileCopyMoveJob::Handle *MoveCopyTaskWidget::errorHandle() const
{
    return m_errorHandle;
}

void MoveCopyTaskWidget::initConnect()
{
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(handleClose()));
    connect(m_keepBothButton, &QPushButton::clicked, this, &MoveCopyTaskWidget::handleResponse);
    connect(m_skipButton, &QPushButton::clicked, this, &MoveCopyTaskWidget::handleResponse);
    connect(m_replaceButton, &QPushButton::clicked, this, &MoveCopyTaskWidget::handleResponse);
}

void MoveCopyTaskWidget::updateMessage(const QMap<QString, QString> &data)
{
    QString file, destination, speed, remainTime, progress, status, srcPath, targetPath;
    QString msg1, msg2;

    if (data.contains("optical_op_type")) {
        m_animatePad->setCanPause(false);
        status = data["optical_op_status"];
        progress = data["optical_op_progress"];

        msg1 = (data["optical_op_type"] == QString::number(FileJob::JobType::OpticalBlank)
               ? tr("Erasing disc %1, please wait...")
               : tr("Burning disc %1, please wait...")).arg(data["optical_op_dest"]);
        msg2 = "";
        if (data["optical_op_type"] != QString::number(FileJob::JobType::OpticalBlank)) {
            const QHash<QString, QString> msg2map = {
                {"0", ""}, // unused right now
                {"1", tr("Writing data...")},
                {"2", tr("Verifying data...")}
            };
            msg2 = msg2map.value(data["optical_op_phase"], "");
        }
        setMessage(msg1, msg2);
        setTipMessage(data["optical_op_speed"], "");

        qDebug() << status << progress;
        if (status == QString::number(DISOMasterNS::DISOMaster::JobStatus::Stalled)) {
            m_animatePad->startAnimation();
        }
        else if (status == QString::number(DISOMasterNS::DISOMaster::JobStatus::Running)) {
            m_animatePad->stopAnimation();
            setProgress(progress);
        }
        return;
    }

    if (data.contains("file")) {
        file = data.value("file");
    }
    if (data.contains("destination")) {
        destination = data.value("destination");
    }
    if (data.contains("speed")) {
        speed = data.value("speed");
    }
    if (data.contains("remainTime")) {
        remainTime = data.value("remainTime");
    }

    if (data.contains("progress")) {
        progress = data.value("progress");
    }

    if (data.contains("sourcePath")) {
        srcPath = data.value("sourcePath");
    }

    if (data.contains("targetPath")) {
        targetPath = data.value("targetPath");
    }

    if (data.contains("status")) {
        status = data.value("status");
    }

    QString speedStr = "%1";
    QString remainStr = "%1";

    if (m_jobDetail.contains("type")) {
        if (!file.isEmpty()) {
            if (m_jobDetail.value("type") == "copy") {
                msg1 = tr("Copying %1").arg(file);
                msg2 = tr("Copy to %2").arg(destination);

            } else if (m_jobDetail.value("type") == "move") {
                msg1 = tr("Moving %1").arg(file);
                msg2 = tr("Move to %2").arg(destination);
            } else if (m_jobDetail.value("type") == "restore") {
                msg1 = tr("Restoring %1").arg(file);
                msg2 = tr("Restore to %2").arg(destination);
            } else if (m_jobDetail.value("type") == "delete") {
                msg1 = tr("Deleting %1").arg(file);
                msg2 = tr("");
            } else if (m_jobDetail.value("type") == "trash") {
                msg1 = tr("Trashing %1").arg(file);
                msg2 = tr("");
            }
        }

        if (status == "restoring") {
            m_animatePad->startAnimation();
        } else if (status == "calculating") {
            msg2 = tr("Calculating space, please wait");
            m_animatePad->startAnimation();
        } else if (status == "conflict") {
            msg1 = QString(tr("File named %1 already exists in target folder")).arg(file);
            msg2 = QString(tr("Original path %1 target path %2")).arg(QFileInfo(srcPath).absolutePath(), QFileInfo(targetPath).absolutePath());
            updateConflictDetailFrame(DUrl::fromLocalFile(srcPath), DUrl::fromLocalFile(targetPath));

            if (QFileInfo(srcPath).isDir() &&
                    QFileInfo(targetPath).isDir()) {
                m_replaceButton->setText(tr("merge"));
            } else {
                m_replaceButton->setText(tr("Replace"));
            }

            m_replaceButton->show();
            m_keepBothButton->show();
        } else if (status == "error") {
            if (m_fileJob) {
                m_replaceButton->setVisible(m_fileJob->supportActions(m_fileJob->error()).testFlag(DFileCopyMoveJob::RetryAction));
                m_replaceButton->setText(tr("Retry"));
                m_keepBothButton->hide();
                m_errorLabel->setText(m_fileJob->errorString());
            }
        } else if (!status.isEmpty()) {
            m_animatePad->stopAnimation();
        } else if (m_fileJob) {
            m_errorLabel->setText(QString());
        }

        QFontMetrics fm(m_msg1Label->font());
        msg1 = fm.elidedText(msg1, Qt::ElideMiddle, m_msg1Label->width());
        msg2 = fm.elidedText(msg2, Qt::ElideMiddle, m_msg2Label->width());

        speedStr = speedStr.arg(speed);
        remainStr = remainStr.arg(remainTime);
        setMessage(msg1, msg2);
        setTipMessage(speedStr, remainStr);
    }

    if (!progress.isEmpty()) {
        setProgress(progress);
    }
}

void MoveCopyTaskWidget::updateTipMessage()
{
//    QString tipMessage = tr("Current speed:%1 time left:%2 ")
//               .arg(QString::number(m_speed), QString::number(m_timeLeft));
//    setTipMessage(tipMessage);
    setTipMessage(QString::number(m_speed), QString::number(m_timeLeft));
}

void MoveCopyTaskWidget::handleLineDisplay(const int &row, const bool &hover, const int &taskNum)
{
    if ((row - 1) == property("row").toInt() || row == property("row").toInt()) {
        if (hover) {
            m_lineLabel->hide();
        } else {
            m_lineLabel->show();
        }
    } else {
        if (m_lineLabel->isHidden()) {
            m_lineLabel->show();
        }
    }

    if (property("row").toInt() == taskNum - 1) {
        m_lineLabel->hide();
    }
}

void MoveCopyTaskWidget::showConflict()
{
    qDebug() << m_buttonFrame->height() << m_conflictFrame->height();
    setFixedHeight(100 + m_buttonFrame->height() + m_conflictFrame->height());
    m_conflictFrame->show();
    m_buttonFrame->show();
    emit heightChanged();
    emit conflictShowed(m_jobDetail);
}

void MoveCopyTaskWidget::hideConflict()
{
    setFixedHeight(100);
    m_conflictFrame->hide();
    m_buttonFrame->hide();
    emit heightChanged();
    emit conflictHided(m_jobDetail);
}

void MoveCopyTaskWidget::showButtonFrame()
{
    setFixedHeight(100 + m_buttonFrame->height());
    m_buttonFrame->show();
    emit heightChanged();
}

void MoveCopyTaskWidget::hideButtonFrame()
{
    setFixedHeight(100);
    m_buttonFrame->hide();
    emit heightChanged();
}

void MoveCopyTaskWidget::updateConflictDetailFrame(const DUrl originFilePath, const DUrl targetFilePath)
{
    qDebug() << originFilePath << targetFilePath << m_originIconLabel << m_targetIconLabel;
    DAbstractFileInfoPointer originInfo = fileService->createFileInfo(NULL, originFilePath);
    DAbstractFileInfoPointer targetInfo = fileService->createFileInfo(NULL, targetFilePath);
    if (originInfo && targetInfo) {
        QFontMetrics fm(m_originTitleLabel->font());

        m_originIconLabel->setPixmap(originInfo->fileIcon().pixmap(48, 48));
        m_originTimeLabel->setText(QString(tr("Time modified:%1")).arg(originInfo->lastModifiedDisplayName()));
        if (originInfo->isDir()) {
            m_originTitleLabel->setText(tr("Original folder"));
            m_originSizeLabel->setText(QString(tr("Contains:%1")).arg(originInfo->sizeDisplayName()));
        } else {
            m_originTitleLabel->setText(tr("Original file"));
            m_originSizeLabel->setText(QString(tr("Size:%1")).arg(originInfo->sizeDisplayName()));
        }
        QString originMsg = fm.elidedText(m_originTitleLabel->text(), Qt::ElideRight, 300);
        m_originTitleLabel->setText(originMsg);

        m_targetIconLabel->setPixmap(targetInfo->fileIcon().pixmap(48, 48));
        m_targetTimeLabel->setText(QString(tr("Time modified:%1")).arg(targetInfo->lastModifiedDisplayName()));

        if (originInfo->isDir()) {
            m_targetTitleLabel->setText(tr("Target folder"));
            m_targetSizeLabel->setText(QString(tr("Contains:%1")).arg(targetInfo->sizeDisplayName()));
        } else {
            m_targetTitleLabel->setText(tr("Target file"));
            m_targetSizeLabel->setText(QString(tr("Size:%1")).arg(targetInfo->sizeDisplayName()));
        }
        QString targetMsg = fm.elidedText(m_targetTitleLabel->text(), Qt::ElideRight, 300);
        m_targetTitleLabel->setText(targetMsg);
    }

}

void MoveCopyTaskWidget::onJobCurrentJobChanged(const DUrl from, const DUrl to)
{
    m_jobInfo->currentJob = qMakePair(from, to);
    updateMessageByJob();
}

void MoveCopyTaskWidget::onJobSpeedChanged(qint64 speed)
{
    m_jobInfo->speed = speed;
    updateMessageByJob();
}

bool MoveCopyTaskWidget::event(QEvent *e)
{
    if (e->type() == QEvent::Enter) {
        m_closeButton->show();
        m_speedLabel->hide();
        m_remainLabel->hide();
        m_bgLabel->setStyleSheet("QLabel#Background{"
                                 "background-color: #f3f3f3;"
                                 "border: 1px solid #f3f3f3;"
                                 "border-radius: 8px;"
                                 "}");
        m_bgLabel->setFixedSize(size() - QSize(20, 0));
        m_bgLabel->move(10, 0);
        emit hovereChanged(true);
    } else if (e->type() == QEvent::Leave) {
        hovereChanged(false);
        m_speedLabel->show();
        m_remainLabel->show();
        m_closeButton->hide();
        m_bgLabel->setStyleSheet("QLabel#Background{"
                                 "background-color: #fff;"
                                 "border: 1px solid #fff;"
                                 "border-radius: 8px;"
                                 "}");
    }

    return QFrame::event(e);
}

void MoveCopyTaskWidget::onJobProgressChanged(qreal progress)
{
    if (m_animatePad->animationRunning()) {
        m_animatePad->stopAnimation();
    }

    m_jobInfo->progress = progress;
    setProgress(progress * 100);
}

void MoveCopyTaskWidget::disposeJobError(DFileCopyMoveJob::Action action)
{
    m_errorHandle->m_actionOfError = action;

    if (m_checkBox->isChecked()) {
        m_fileJob->setActionOfErrorType(m_fileJob->error(), action);
    }

    hideConflict();

    if (m_fileJob->state() == DFileCopyMoveJob::PausedState) {
        m_fileJob->togglePause();
    }
}

void MoveCopyTaskWidget::handleClose()
{
    if (m_fileJob) {
        m_fileJob->stop();
    }

    emit closed(m_jobDetail);
}

void MoveCopyTaskWidget::handleResponse()
{
    QObject *who = sender();
    const int &code = who->property("code").toInt();
    m_response.insert("code", code);
    m_response.insert("applyToAll", m_checkBox->isChecked());
    hideConflict();
    emit conflictResponseConfirmed(m_jobDetail, m_response);
}

static QString formatTime(int second)
{
    quint8 s = second % 60;
    quint8 m = second / 60;
    quint8 h = m / 60;
    quint8 d = h / 24;

    m = m % 60;
    h = h % 24;

    QString time_string;

    if (d > 0) {
        time_string.append(QString::number(d)).append(" d");
    }

    if (h > 0) {
        if (!time_string.isEmpty()) {
            time_string.append(' ');
        }

        time_string.append(QString::number(h)).append(" h");
    }

    if (m > 0) {
        if (!time_string.isEmpty()) {
            time_string.append(' ');
        }

        time_string.append(QString::number(m)).append(" m");
    }

    if (s > 0 || time_string.isEmpty()) {
        if (!time_string.isEmpty()) {
            time_string.append(' ');
        }

        time_string.append(QString::number(s)).append(" s");
    }

    return time_string;
}

void MoveCopyTaskWidget::updateMessageByJob()
{
    QMap<QString, QString> datas;

    if (m_fileJob->mode() == DFileCopyMoveJob::CopyMode) {
        m_jobDetail["type"] = "copy";
    } else if (m_fileJob->mode() == DFileCopyMoveJob::MoveMode) {
        m_jobDetail["type"] = m_fileJob->targetUrl().isValid() ? "move" : "delete";
    }

    datas["sourcePath"] = m_jobInfo->currentJob.first.path();
    datas["file"] = m_jobInfo->currentJob.first.fileName();
    datas["targetPath"] = m_jobInfo->currentJob.second.path();
    datas["destination"] = m_jobInfo->currentJob.second.isValid() ? m_jobInfo->currentJob.second.parentUrl().path() : QString();
    datas["speed"] = FileUtils::formatSize(m_jobInfo->speed) + "/s";

    if (m_jobInfo->totalDataSize >= 0 && m_jobInfo->speed) {
        datas["remainTime"] = formatTime(int(m_jobInfo->totalDataSize * (1 - m_jobInfo->progress) / m_jobInfo->speed));
    }

    if (m_fileJob->state() != DFileCopyMoveJob::RunningState) {
        if (m_fileJob->error() == DFileCopyMoveJob::FileExistsError
                || m_fileJob->error() == DFileCopyMoveJob::DirectoryExistsError) {
            datas["status"] = "conflict";
        } else if (m_fileJob->error() != DFileCopyMoveJob::NoError) {
            datas["status"] = "error";
        }
    }

    updateMessage(datas);
}

QString MoveCopyTaskWidget::getTargetObj()
{
    return m_targetObj;
}

void MoveCopyTaskWidget::setTargetObj(QString targetObj)
{
    m_targetObj = targetObj;
}

QString MoveCopyTaskWidget::getDestinationObj()
{
    return m_destinationObj;
}

void MoveCopyTaskWidget::setDestinationObj(QString destinationObj)
{
    m_destinationObj = destinationObj;
}

int MoveCopyTaskWidget::getProgress()
{
    return m_progress;
}

void MoveCopyTaskWidget::setProgress(int value)
{
    m_progress = value;
    m_animatePad->setCurrentValue(value);
}

void MoveCopyTaskWidget::setProgress(QString value)
{
    m_progress = value.toInt();
    m_animatePad->setCurrentValue(value.toInt());
}

float MoveCopyTaskWidget::getSpeed()
{
    return m_speed;
}

void MoveCopyTaskWidget::setSpeed(float speed)
{
    m_speed = speed;
}

int MoveCopyTaskWidget::getTimeLeft()
{
    return m_timeLeft;
}

void MoveCopyTaskWidget::setTimeLeft(int time)
{
    m_timeLeft = time;
}

void MoveCopyTaskWidget::setMessage(const QString &operateStr, const QString &destinateStr)
{
    m_operateMessage = operateStr;
    m_destinationMessage = destinateStr;
    m_msg1Label->setText(m_operateMessage);
    m_msg2Label->setText(m_destinationMessage);
}

void MoveCopyTaskWidget::setTipMessage(const QString &speedStr, const QString &remainStr)
{
    m_speedMessage = speedStr;
    m_remainMessage = remainStr;
    m_speedLabel->setText(m_speedMessage);
    m_remainLabel->setText(m_remainMessage);
}


int DTaskDialog::MaxHeight = 0;

DTaskDialog::DTaskDialog(QWidget *parent) :
    DAbstractDialog(parent)
{
    initUI();
    initConnect();
}

void DTaskDialog::initUI()
{
    setWindowFlags((windowFlags() & ~ Qt::WindowSystemMenuHint & ~Qt::Dialog) | Qt::Window);
    setFixedWidth(m_defaultWidth);

    m_titlebar = new DTitlebar(this);
    m_titlebar->setBackgroundTransparent(true);
    m_titlebar->layout()->setContentsMargins(0, 0, 0, 0);
    m_titlebar->setMenuVisible(false);

    m_taskListWidget = new QListWidget;
    m_taskListWidget->setSelectionMode(QListWidget::NoSelection);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_titlebar);
    mainLayout->addSpacing(3);
    mainLayout->addWidget(m_taskListWidget);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    moveToCenter();
}

void DTaskDialog::initConnect()
{

}

QListWidget *DTaskDialog::getTaskListWidget()
{
    return m_taskListWidget;
}

void DTaskDialog::setTitle(QString title)
{
    m_titlebar->setTitle(title);
}

void DTaskDialog::setTitle(int taskCount)
{
    QString title;
    if (taskCount == 1) {
        title = QObject::tr("1 task in progress");
    } else {
        title = QObject::tr("%1 tasks in progress").arg(QString::number(taskCount));
    }
    setTitle(title);
}

void DTaskDialog::addTask(const QMap<QString, QString> &jobDetail)
{
    if (jobDetail.contains("jobId")) {
        MoveCopyTaskWidget *moveWidget = new MoveCopyTaskWidget(jobDetail);
        moveWidget->setFixedHeight(80);
        connect(moveWidget, SIGNAL(closed(QMap<QString, QString>)),
                this, SLOT(handleTaskClose(QMap<QString, QString>)));
        connect(moveWidget, SIGNAL(conflictResponseConfirmed(QMap<QString, QString>, QMap<QString, QVariant>)),
                this, SLOT(handleConflictResponse(QMap<QString, QString>, QMap<QString, QVariant>)));
        connect(moveWidget, SIGNAL(heightChanged()), this, SLOT(adjustSize()));
        connect(moveWidget, SIGNAL(conflictShowed(QMap<QString, QString>)),
                this, SIGNAL(conflictShowed(QMap<QString, QString>)));
        connect(moveWidget, SIGNAL(conflictHided(QMap<QString, QString>)),
                this, SIGNAL(conflictHided(QMap<QString, QString>)));

        //handle item line display logic
        connect(moveWidget, &MoveCopyTaskWidget::hovereChanged, this, &DTaskDialog::onItemHovered);
        connect(this, &DTaskDialog::currentHoverRowChanged, moveWidget, &MoveCopyTaskWidget::handleLineDisplay);
        QListWidgetItem *item = new QListWidgetItem();
        item->setFlags(Qt::NoItemFlags);
        item->setSizeHint(QSize(item->sizeHint().width(), 60));
        m_taskListWidget->addItem(item);
        m_taskListWidget->setItemWidget(item, moveWidget);
        m_jobIdItems.insert(jobDetail.value("jobId"), item);
        setTitle(m_taskListWidget->count());
        adjustSize();
        show();
        QTimer::singleShot(100, this, &DTaskDialog::raise);

        moveWidget->setProperty("row", m_taskListWidget->count() - 1);
        emit currentHoverRowChanged(1, false, m_taskListWidget->count());
    }
}

MoveCopyTaskWidget *DTaskDialog::addTaskJob(DFileCopyMoveJob *job)
{
    MoveCopyTaskWidget *moveWidget = new MoveCopyTaskWidget(job);
    moveWidget->setFixedHeight(80);
    connect(moveWidget, SIGNAL(closed(QMap<QString, QString>)),
            job, SLOT(stop()));
    connect(moveWidget, SIGNAL(heightChanged()), this, SLOT(adjustSize()));

    //handle item line display logic
    connect(moveWidget, &MoveCopyTaskWidget::hovereChanged, this, &DTaskDialog::onItemHovered);
    connect(this, &DTaskDialog::currentHoverRowChanged, moveWidget, &MoveCopyTaskWidget::handleLineDisplay);
    QListWidgetItem *item = new QListWidgetItem();
    item->setFlags(Qt::NoItemFlags);
    item->setSizeHint(QSize(item->sizeHint().width(), 60));
    m_taskListWidget->addItem(item);
    m_taskListWidget->setItemWidget(item, moveWidget);
    m_jobIdItems.insert(QString::number(quintptr(job), 16), item);
    setTitle(m_taskListWidget->count());
    adjustSize();
    show();
    QTimer::singleShot(100, this, &DTaskDialog::raise);

    moveWidget->setProperty("row", m_taskListWidget->count() - 1);
    emit currentHoverRowChanged(1, false, m_taskListWidget->count());

    return moveWidget;
}

void DTaskDialog::addConflictTask(const QMap<QString, QString> &jobDetail)
{
    if (jobDetail.contains("jobId")) {
        MoveCopyTaskWidget *moveWidget = new MoveCopyTaskWidget(jobDetail);
        moveWidget->setFixedHeight(85);
        connect(moveWidget, SIGNAL(closed(QMap<QString, QString>)),
                this, SLOT(handleTaskClose(QMap<QString, QString>)));
        connect(moveWidget, SIGNAL(conflictResponseConfirmed(QMap<QString, QString>, QMap<QString, QVariant>)),
                this, SLOT(handleConflictResponse(QMap<QString, QString>, QMap<QString, QVariant>)));
        connect(moveWidget, SIGNAL(heightChanged()), this, SLOT(adjustSize()));
        connect(moveWidget, SIGNAL(conflictShowed(QMap<QString, QString>)),
                this, SIGNAL(conflictShowed(QMap<QString, QString>)));
        connect(moveWidget, SIGNAL(conflictHided(QMap<QString, QString>)),
                this, SIGNAL(conflictHided(QMap<QString, QString>)));
        QListWidgetItem *item = new QListWidgetItem();
        item->setFlags(Qt::NoItemFlags);
        item->setSizeHint(QSize(item->sizeHint().width(), 85));
        m_taskListWidget->addItem(item);
        m_taskListWidget->setItemWidget(item, moveWidget);
        m_jobIdItems.insert(jobDetail.value("jobId"), item);
        setTitle(m_taskListWidget->count());
        adjustSize();
        show();
        QTimer::singleShot(100, this, &DTaskDialog::raise);
    }
}

void DTaskDialog::adjustSize()
{
    int listHeight = 2;
    for (int i = 0; i < m_taskListWidget->count(); i++) {
        QListWidgetItem *item = m_taskListWidget->item(i);
        int h = m_taskListWidget->itemWidget(item)->height();
        item->setSizeHint(QSize(item->sizeHint().width(), h));
        listHeight += h;
    }

    if (listHeight < qApp->desktop()->availableGeometry().height() - 60) {
        m_taskListWidget->setFixedHeight(listHeight);
        setFixedHeight(listHeight + 60);
        MaxHeight = height();
    } else {
        setFixedHeight(MaxHeight);
    }

    layout()->setSizeConstraint(QLayout::SetNoConstraint);
    moveYCenter();
}

void DTaskDialog::moveYCenter()
{
    QRect qr = frameGeometry();
    QPoint cp;
    if (parent()) {
        cp = static_cast<QWidget *>(parent())->geometry().center();
    } else {
        cp = qApp->desktop()->availableGeometry().center();
    }
    qr.moveCenter(cp);
    move(x(), qr.y());
}

void DTaskDialog::removeTaskByPath(QString jobId)
{
    if (m_jobIdItems.contains(jobId)) {
        QListWidgetItem *item = m_jobIdItems.value(jobId);
        m_taskListWidget->removeItemWidget(item);
        m_taskListWidget->takeItem(m_taskListWidget->row(item));
        m_jobIdItems.remove(jobId);
        setTitle(m_taskListWidget->count());
        if (m_taskListWidget->count() == 0) {
            close();
        }

        for (int i = 0; i < m_taskListWidget->count(); i++) {
            QListWidgetItem *item = m_taskListWidget->item(i);
            MoveCopyTaskWidget *w =  qobject_cast<MoveCopyTaskWidget *>(m_taskListWidget->itemWidget(item));
            if (w) {
                w->setProperty("row", i);
            }
        }

        emit currentHoverRowChanged(0, false, m_taskListWidget->count());
    }
}

void DTaskDialog::showConflictDiloagByJob(const QMap<QString, QString> &jobDetail)
{
    qDebug() << jobDetail;
    if (jobDetail.contains("jobId")) {
        QString jobId = jobDetail.value("jobId");
        if (m_jobIdItems.contains(jobId)) {
            QListWidgetItem *item = m_jobIdItems.value(jobId);
            MoveCopyTaskWidget *w = static_cast<MoveCopyTaskWidget *>(m_taskListWidget->itemWidget(item));
            w->showConflict();
        }
    }
}

void DTaskDialog::handleConflictResponse(const QMap<QString, QString> &jobDetail, const QMap<QString, QVariant> &response)
{

    emit conflictRepsonseConfirmed(jobDetail, response);
}

void DTaskDialog::handleMinimizeButtonClick()
{
    showMinimized();
}

void DTaskDialog::onItemHovered(const bool &hover)
{
    MoveCopyTaskWidget *w = qobject_cast<MoveCopyTaskWidget *>(sender());
    int row = w->property("row").toInt();
    if (row >= 0) {
        emit currentHoverRowChanged(row, hover, m_taskListWidget->count());
    }
}

void DTaskDialog::handleTaskClose(const QMap<QString, QString> &jobDetail)
{
    qDebug() << jobDetail;
    removeTask(jobDetail, false);
    setTitle(m_taskListWidget->count());
    if (jobDetail.contains("type")) {
        emit abortTask(jobDetail);
    }
}

void DTaskDialog::removeTask(const QMap<QString, QString> &jobDetail, bool adjustSize)
{
    if (jobDetail.contains("jobId")) {
        removeTaskByPath(jobDetail.value("jobId"));

        if (adjustSize) {
            this->adjustSize();
        }
    }
}

void DTaskDialog::removeTaskJob(void *job)
{
    removeTaskByPath(QString::number(quintptr(job), 16));
    adjustSize();
}

void DTaskDialog::removeTaskImmediately(const QMap<QString, QString> &jobDetail)
{
    if (m_taskListWidget->count() > 1) {
        delayRemoveTask(jobDetail);
    } else {
        removeTask(jobDetail);
    }
}

void DTaskDialog::delayRemoveTask(const QMap<QString, QString> &jobDetail)
{
    QTimer::singleShot(2000, this, [ = ]() {
        removeTask(jobDetail);
    });
}

void DTaskDialog::handleUpdateTaskWidget(const QMap<QString, QString> &jobDetail,
        const QMap<QString, QString> &data)
{
    if (jobDetail.contains("jobId")) {
        QString jobId = jobDetail.value("jobId");
        if (m_jobIdItems.contains(jobId)) {
            QListWidgetItem *item = m_jobIdItems.value(jobId);
            MoveCopyTaskWidget *w = static_cast<MoveCopyTaskWidget *>(m_taskListWidget->itemWidget(item));
            w->updateMessage(data);
        }
    }
}


void DTaskDialog::closeEvent(QCloseEvent *event)
{
    foreach (QListWidgetItem *item, m_jobIdItems.values()) {
        if (item) {
            if (m_taskListWidget->itemWidget(item)) {
                MoveCopyTaskWidget *w = static_cast<MoveCopyTaskWidget *>(m_taskListWidget->itemWidget(item));
                w->handleClose();
            }
        }
    }
    QDialog::closeEvent(event);
    emit closed();
}

void DTaskDialog::keyPressEvent(QKeyEvent *event)
{
    //handle escape key press event for emitting close event
    if (event->key() == Qt::Key_Escape) {
        emit close();
    }
    QDialog::keyPressEvent(event);
}
