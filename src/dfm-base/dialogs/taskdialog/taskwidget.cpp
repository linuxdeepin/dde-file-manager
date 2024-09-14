// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskwidget.h"
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/thumbnail/thumbnailhelper.h>

#include <DWaterProgress>
#include <DIconButton>
#include <DGuiApplicationHelper>

#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QUrl>
#include <QApplication>
#include <QPointer>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE
using namespace dfmbase;

static constexpr int kMsgLabelWidth { 390 };
static constexpr int kMsgLabelHoverWidth { 452 };
static constexpr int kSpeedLabelWidth { 100 };
static constexpr uint8_t kVirtualValue { 30 };
static constexpr char kBtnPropertyActionName[] { "btnType" };
static constexpr AbstractJobHandler::JobState kPausedState = AbstractJobHandler::JobState::kPauseState;

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

TaskWidget::~TaskWidget()
{
    disconnect();
}
/*!
 * \brief TaskWidget::setTaskHandle 连接当前任务处理器的信号，处理当前拷贝信息的显示
 * \param handle 任务处理控制器
 */
void TaskWidget::setTaskHandle(const JobHandlePointer handle)
{
    if (!handle)
        return;

    JobInfoPointer info { nullptr };
    info = handle->getTaskInfoByNotifyType(AbstractJobHandler::NotifyType::kNotifyStateChangedKey);
    if (!info.isNull()) {
        onHandlerTaskStateChange(info);
    }
    info = handle->getTaskInfoByNotifyType(AbstractJobHandler::NotifyType::kNotifyCurrentTaskKey);
    if (!info.isNull()) {
        onShowTaskInfo(info);
    }
    info = handle->getTaskInfoByNotifyType(AbstractJobHandler::NotifyType::kNotifyProccessChangedKey);
    if (!info.isNull()) {
        onShowTaskProccess(info);
    }
    info = handle->getTaskInfoByNotifyType(AbstractJobHandler::NotifyType::kNotifySpeedUpdatedTaskKey);
    if (!info.isNull()) {
        onShowSpeedUpdatedInfo(info);
    }
    info = handle->getTaskInfoByNotifyType(AbstractJobHandler::NotifyType::kNotifyErrorTaskKey);
    if (!info.isNull()) {
        onShowErrors(info);
    }

    connect(handle.get(), &AbstractJobHandler::proccessChangedNotify, this, &TaskWidget::onShowTaskProccess, Qt::QueuedConnection);
    connect(handle.get(), &AbstractJobHandler::stateChangedNotify, this, &TaskWidget::onHandlerTaskStateChange, Qt::QueuedConnection);
    connect(handle.get(), &AbstractJobHandler::errorNotify, this, &TaskWidget::onShowErrors, Qt::QueuedConnection);
    connect(handle.get(), &AbstractJobHandler::currentTaskNotify, this, &TaskWidget::onShowTaskInfo, Qt::QueuedConnection);
    connect(handle.get(), &AbstractJobHandler::speedUpdatedNotify, this, &TaskWidget::onShowSpeedUpdatedInfo, Qt::QueuedConnection);

    connect(this, &TaskWidget::buttonClicked, handle.get(), &AbstractJobHandler::operateTaskJob, Qt::QueuedConnection);
}

void TaskWidget::resetPauseStute()
{
    if (btnPause)
        btnPause->setEnabled(true);
}
/*!
 * \brief TaskWidget::onButtonClicked 处理所有按钮按下
 */
void TaskWidget::onButtonClicked()
{
    QObject *obj { sender() };
    if (!obj) {
        qCWarning(logDFMBase) << "the button is null or the button is release!";
        return;
    }
    if (infoTimer.isActive())
        infoTimer.stop();
    if (btnPause)
        btnPause->setEnabled(true);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    isShowError.storeRelaxed(false);
#else
    isShowError.store(false);
#endif
    AbstractJobHandler::SupportActions actions = obj->property(kBtnPropertyActionName).value<AbstractJobHandler::SupportAction>();
    showConflictButtons(actions.testFlag(AbstractJobHandler::SupportAction::kPauseAction));
    actions = chkboxNotAskAgain && chkboxNotAskAgain->isChecked() ? actions | AbstractJobHandler::SupportAction::kRememberAction : actions;
    lbErrorMsg->setText("");
    lbErrorMsg->hide();
    emit buttonClicked(actions);
}
void TaskWidget::parentClose()
{
    if (btnPause)
        btnPause->setEnabled(true);
}
/*!
 * \brief TaskWidget::onShowErrors 处理和显示错误信息
 * \param info 这个Varint信息map
 * 在我们自己提供的dailog服务中，这个VarintMap必须有存在kSpeedKey（显示任务的右第一个label的显示，类型：QString）、
 * kRemindTimeKey（（显示任务的右第二个label的显示，类型：QString）
 */
void TaskWidget::onShowErrors(const JobInfoPointer jobInfo)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    isShowError.storeRelaxed(true);
#else
    isShowError.store(true);
#endif

    AbstractJobHandler::JobErrorType errorType = jobInfo->value(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey).value<AbstractJobHandler::JobErrorType>();
    QString sourceMsg = jobInfo->value(AbstractJobHandler::NotifyInfoKey::kSourceMsgKey).toString();
    QString targetMsg = jobInfo->value(AbstractJobHandler::NotifyInfoKey::kTargetMsgKey).toString();
    AbstractJobHandler::SupportActions actions = jobInfo->value(AbstractJobHandler::NotifyInfoKey::kActionsKey).value<AbstractJobHandler::SupportActions>();
    lbSrcPath->setText(sourceMsg);
    lbDstPath->setText(targetMsg);
    if (errorType == AbstractJobHandler::JobErrorType::kFileExistsError || errorType == AbstractJobHandler::JobErrorType::kDirectoryExistsError) {
        QUrl source = jobInfo->value(AbstractJobHandler::NotifyInfoKey::kSourceUrlKey).value<QUrl>();
        QUrl target = jobInfo->value(AbstractJobHandler::NotifyInfoKey::kTargetUrlKey).value<QUrl>();
        return onShowConflictInfo(source, target, actions);
    }
    QString errorMsg = jobInfo->value(AbstractJobHandler::NotifyInfoKey::kErrorMsgKey).toString();
    lbErrorMsg->setText(errorMsg);
    lbErrorMsg->setHidden(errorMsg.isEmpty());
    if (!widButton) {
        widButton = createBtnWidget();
        mainLayout->addWidget(widButton);
    }
    widButton->setHidden(false);

    if (!widConfict) {
        widConfict = createConflictWidget();
        rVLayout->addWidget(widConfict);
    }

    if (widConfict)
        widConfict->hide();

    showBtnByAction(actions);
    showConflictButtons(true, false);

    if (btnPause)
        btnPause->setEnabled(false);
}
/*!
 * \brief TaskWidget::onShowConflictInfo 显示冲突界面信息
 * \param source 源文件的url
 * \param target 目标文件的url
 * \param action 支持的操作
 */
void TaskWidget::onShowConflictInfo(const QUrl source, const QUrl target, const AbstractJobHandler::SupportActions action)
{
    if (!widButton) {
        widButton = createBtnWidget();
        mainLayout->addWidget(widButton);
    }
    if (!widConfict) {
        widConfict = createConflictWidget();
        rVLayout->addWidget(widConfict);
    }

    adjustSize();
    QString error;
    const FileInfoPointer &originInfo = InfoFactory::create<FileInfo>(source, Global::CreateFileInfoType::kCreateFileInfoAuto, &error);
    if (!originInfo) {
        lbErrorMsg->setText(QString(tr("create source file %1 Info failed in show conflict Info function!")).arg(source.path()));
        showBtnByAction(AbstractJobHandler::SupportAction::kCancelAction);
        lbErrorMsg->show();
        showConflictButtons(true, false);
        qCWarning(logDFMBase) << QString("create source file %1 Info failed in show conflict Info function!").arg(source.path());
        return;
    }
    error.clear();
    FileInfoPointer targetInfo = InfoFactory::create<FileInfo>(target, Global::CreateFileInfoType::kCreateFileInfoAuto, &error);
    if (!targetInfo) {
        lbErrorMsg->setText(QString(tr("create target file %1 Info failed in show conflict Info function!")).arg(target.path()));
        lbErrorMsg->show();
        showConflictButtons(true, false);
        showBtnByAction(AbstractJobHandler::SupportAction::kCancelAction);
        qCWarning(logDFMBase) << QString("create source file %1 Info failed in show conflict Info function!").arg(target.path());
        return;
    }

    showBtnByAction(action);

    auto needRetry = showFileInfo(originInfo, true);
    needRetry = showFileInfo(targetInfo, false) || needRetry;
    if (needRetry)
        infoTimer.start();

    widConfict->show();
    widButton->show();
    btnCoexist->setHidden(false);
    showConflictButtons();

    if (btnPause)
        btnPause->setEnabled(false);
}
/*!
 * \brief TaskWidget::onHandlerTaskStateChange 处理和显示当前拷贝任务的状态变化
 * \param info 这个Varint信息map
 * 在我们自己提供的dailog服务中，这个VarintMap必须存在 kJobStateKey （当前任务执行的状态，类型：JobState）用来展示暂停和开始按钮状态
 */
void TaskWidget::onHandlerTaskStateChange(const JobInfoPointer JobInfo)
{
    bool hide = JobInfo->value(AbstractJobHandler::NotifyInfoKey::kJobStateHideKey).value<bool>();

    if (hide)
        isBtnHidden = true;

    if (JobInfo->count(AbstractJobHandler::NotifyInfoKey::kJobStateKey) == 0)
        return;

    AbstractJobHandler::JobState state = JobInfo->value(AbstractJobHandler::NotifyInfoKey::kJobStateKey).value<AbstractJobHandler::JobState>();
    bool isCurPaused = kPausedState == state;
    if (isCurPaused == isPauseState) {
        return;
    }
    if (state == kPausedState) {
        isPauseState = true;
        btnPause->setIcon(QIcon::fromTheme("dfm_task_start"));
        QVariant variantPause;
        variantPause.setValue<AbstractJobHandler::SupportAction>(AbstractJobHandler::SupportAction::kResumAction);
        btnPause->setProperty(kBtnPropertyActionName, variantPause);
        progress->stop();
    } else {
        isPauseState = false;
        btnPause->setIcon(QIcon::fromTheme("dfm_task_pause"));
        QVariant variantPause;
        variantPause.setValue<AbstractJobHandler::SupportAction>(AbstractJobHandler::SupportAction::kPauseAction);
        btnPause->setProperty(kBtnPropertyActionName, variantPause);
        progress->start();
    }
}
/*!
 * \brief TaskWidget::onShowTaskInfo 显示当前任务的任务信息
 * \param info 这个Varint信息map
 * 在我们自己提供的dailog服务中，这个VarintMap必须有存在kSourceMsgKey（显示任务的左第一个label的显示，类型：QString）
 * 和kTargetMsgKey显示任务的左第二个label的显示，类型：QString）
 */
void TaskWidget::onShowTaskInfo(const JobInfoPointer JobInfo)
{
    if (isShowError)
        return;

    QString source = JobInfo->value(AbstractJobHandler::NotifyInfoKey::kSourceMsgKey).toString();
    QString target = JobInfo->value(AbstractJobHandler::NotifyInfoKey::kTargetMsgKey).toString();
    lbSrcPath->setText(source);
    lbDstPath->setText(target);
    auto oldheight = height();
    if (lbErrorMsg->isVisible()) {
        lbErrorMsg->setText("");
        lbErrorMsg->hide();
    }
    if (widConfict)
        widConfict->hide();
    if (widButton)
        widButton->hide();

    adjustSize();
    auto newhight = height();

    if (oldheight != newhight)
        emit heightChanged(newhight);
}
/*!
 * \brief TaskWidget::showTaskProccess 显示当前任务进度
 * \param info 这个Varint信息map
 * 在我们自己提供的dailog服务中，这个VarintMap必须有kCurrentProccessKey（当前任务执行的进度，类型qint64）和
 * kTotalSizeKey（当前任务文件的总大小，如果统计文件数量没有完成，值为-1，类型qint64）值来做文件进度的展示
 */
void TaskWidget::onShowTaskProccess(const JobInfoPointer JobInfo)
{
    if (isPauseState)
        return;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    if (isShowError.loadRelaxed())
#else
    if (isShowError.load())
#endif
        return;

    int preValue = progress->value();

    AbstractJobHandler::StatisticState state = JobInfo->value(AbstractJobHandler::NotifyInfoKey::kStatisticStateKey).value<AbstractJobHandler::StatisticState>();
    qint64 current = JobInfo->value(AbstractJobHandler::NotifyInfoKey::kCurrentProgressKey).value<qint64>();
    qint64 total = JobInfo->value(AbstractJobHandler::NotifyInfoKey::kTotalSizeKey).value<qint64>();

    qint64 value = 1;

    if (total > 0 && current > 0) {
        value = static_cast<qint64>((static_cast<qreal>(current) / total) * 100);
        if (current * 100 % total > 0)
            value += 1;   // +1: round up value，maybe 99% when finished
    }

    if (state == AbstractJobHandler::StatisticState::kRunningState) {
        value = value > kVirtualValue ? kVirtualValue : value;
        value = value > preValue ? preValue + 1 : value;
    }

    if (value > 100) {
        value = 100;
    }

    if (value > 0 && value == preValue) {
        // value not change, return
        return;
    } else if (value >= 0 && preValue == 0) {
        // first value arrived, start progress
        progress->start();
        progress->setValue(static_cast<int>(value));
        return;
    } else {
        // update progress
        progress->setValue(static_cast<int>(value));
        progress->update();
    }
}
/*!
 * \brief sendDataSyncing 数据同步中信号
 * \param info 这个Varint信息map
 * 在我们自己提供的dailog服务中，这个VarintMap必须有存在kSpeedKey（显示任务的右第一个label的显示，类型：QString）、
 * kRemindTimeKey（（显示任务的右第二个label的显示，类型：QString）
 */
void TaskWidget::onShowSpeedUpdatedInfo(const JobInfoPointer JobInfo)
{
    if (isPauseState)
        return;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    if (isShowError.loadRelaxed())
#else
    if (isShowError.load())
#endif
        return;

    if (progress->value() >= 100) {
        lbSpeed->setText(tr("Syncing data"));
        lbRmTime->setText(tr("Please wait"));
        return;
    }

    const QVariant &speedValue = JobInfo->value(AbstractJobHandler::NotifyInfoKey::kSpeedKey);
    const QVariant &remindValue = JobInfo->value(AbstractJobHandler::NotifyInfoKey::kRemindTimeKey);
    if (speedValue.isValid()) {
        QString speedStr = QString();
        bool ok = false;
        qint64 speed = speedValue.toLongLong(&ok);
        if (ok)
            speedStr = FileUtils::formatSize(speed) + "/s";
        else
            speedStr = speedValue.toString();
        lbSpeed->setText(speedStr);
    }

    if (remindValue.isValid()) {
        QString rmTimeStr = QString();
        bool ok = false;
        qint64 rmTime = remindValue.toInt(&ok);
        if (ok)
            rmTimeStr = formatTime(rmTime);
        else
            rmTimeStr = remindValue.toString();
        if (rmTime < 0)
            rmTimeStr = "";
        lbRmTime->setText(rmTimeStr);
    }
}

void TaskWidget::onInfoTimer()
{
    auto stop = !originInfo || !showFileInfo(originInfo, true);
    stop = (!targetInfo || !showFileInfo(targetInfo, false)) && stop;
    if (stop)
        infoTimer.stop();
}

/*!
 * \brief TaskWidget::initUI 初始化当前任务的界面
 */
void TaskWidget::initUI()
{
    mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    setFixedWidth(685);

    progress = new DWaterProgress(this);
    progress->setFixedSize(64, 64);
    progress->setValue(1);   // fix：使一开始就有一个进度显示
    progress->setValue(0);
    QHBoxLayout *normalLayout = new QHBoxLayout;
    normalLayout->setContentsMargins(20, 10, 20, 0);
    normalLayout->addWidget(progress, Qt::AlignLeft);
    normalLayout->addSpacing(20);

    lbSrcPath = new ElidedLable;
    lbSpeed = new QLabel;
    lbDstPath = new ElidedLable;
    lbRmTime = new QLabel;
    lbSrcPath->setFixedWidth(kMsgLabelWidth);
    lbSrcPath->setText(tr("In data statistics ..."));
    lbDstPath->setFixedWidth(kMsgLabelWidth);
    lbSpeed->setFixedWidth(kSpeedLabelWidth);
    lbRmTime->setFixedWidth(kSpeedLabelWidth);

    rVLayout = new QVBoxLayout;
    QHBoxLayout *hLayout1 = new QHBoxLayout;
    hLayout1->addSpacing(15);
    hLayout1->addWidget(lbSrcPath, Qt::AlignLeft);
    hLayout1->addSpacing(10);
    hLayout1->addWidget(lbSpeed, Qt::AlignRight);
    hLayout1->addStretch();

    QHBoxLayout *hLayout2 = new QHBoxLayout;
    hLayout2->addSpacing(15);
    hLayout2->addWidget(lbDstPath, Qt::AlignLeft);
    hLayout2->addSpacing(10);
    hLayout2->addWidget(lbRmTime, Qt::AlignRight);
    hLayout2->addStretch();

    lbErrorMsg = new ElidedLable;
    lbErrorMsg->setStyleSheet("color:red;");
    lbErrorMsg->setFixedWidth(kMsgLabelWidth + kSpeedLabelWidth);
    QHBoxLayout *hLayout3 = new QHBoxLayout;
    hLayout3->addSpacing(15);
    hLayout3->addWidget(lbErrorMsg, Qt::AlignLeft);
    hLayout3->addStretch();

    rVLayout->addLayout(hLayout1);
    rVLayout->addLayout(hLayout2);
    rVLayout->addLayout(hLayout3);

    normalLayout->addLayout(rVLayout);

    btnStop = new DIconButton(this);
    btnStop->setObjectName("TaskWidgetStopButton");
    QVariant variantStop;
    variantStop.setValue<AbstractJobHandler::SupportAction>(AbstractJobHandler::SupportAction::kStopAction);
    btnStop->setProperty(kBtnPropertyActionName, variantStop);
    btnStop->setIcon(QIcon::fromTheme("dfm_task_stop"));
    btnStop->setFixedSize(24, 24);
    btnStop->setIconSize({ 24, 24 });
    btnStop->setFlat(true);
    btnStop->setAttribute(Qt::WA_NoMousePropagation);

    btnPause = new DIconButton(this);
    btnPause->setObjectName("TaskWidgetPauseButton");
    QVariant variantPause;
    variantPause.setValue<AbstractJobHandler::SupportAction>(AbstractJobHandler::SupportAction::kPauseAction);
    btnPause->setProperty(kBtnPropertyActionName, variantPause);
    btnPause->setIcon(QIcon::fromTheme("dfm_task_pause"));
    btnPause->setIconSize({ 24, 24 });
    btnPause->setFixedSize(24, 24);
    btnPause->setFlat(true);

    normalLayout->addStretch();
    normalLayout->addWidget(btnPause, Qt::AlignRight);
    normalLayout->addSpacing(10);
    normalLayout->addWidget(btnStop, Qt::AlignRight);

    mainLayout->addLayout(normalLayout);
    mainLayout->setSpacing(0);
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    lbErrorMsg->setVisible(false);
    btnPause->setVisible(false);
    btnStop->setVisible(false);

    initConnection();
}
/*!
 * \brief TaskWidget::initConnection 初始化信号连接
 */
void TaskWidget::initConnection()
{
    connect(btnPause, &QPushButton::clicked, this, &TaskWidget::onButtonClicked);
    connect(btnStop, &QPushButton::clicked, this, &TaskWidget::onButtonClicked);
    connect(&infoTimer, &QTimer::timeout, this, &TaskWidget::onInfoTimer);
    infoTimer.setInterval(200);
}
/*!
 * \brief TaskWidget::createConflictWidget 创建任务显示错误信息的widget
 * \return
 */
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
    conflictWidget->setMaximumWidth(565);

    return conflictWidget;
}
/*!
 * \brief TaskWidget::createBtnWidget 创建错误信息显示的按钮
 * \return
 */
QWidget *TaskWidget::createBtnWidget()
{
    QWidget *buttonWidget = new QWidget;
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);
    buttonLayout->setSpacing(12);

    QVariant variantCoexit;
    variantCoexit.setValue<AbstractJobHandler::SupportAction>(AbstractJobHandler::SupportAction::kCoexistAction);
    btnCoexist = new QPushButton(TaskWidget::tr("Keep both", "button"));
    btnCoexist->setProperty(kBtnPropertyActionName, variantCoexit);

    btnSkip = new QPushButton(TaskWidget::tr("Skip", "button"));
    QVariant variantSkip;
    variantSkip.setValue<AbstractJobHandler::SupportAction>(AbstractJobHandler::SupportAction::kSkipAction);
    btnSkip->setProperty(kBtnPropertyActionName, variantSkip);

    btnReplace = new QPushButton(TaskWidget::tr("Replace", "button"));
    QVariant variantReplace;
    variantReplace.setValue<AbstractJobHandler::SupportAction>(AbstractJobHandler::SupportAction::kReplaceAction);
    btnReplace->setProperty(kBtnPropertyActionName, variantReplace);

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

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    chkboxNotAskAgain = new QCheckBox(TaskWidget::tr("Do not ask again"));
    layout->addSpacing(120);
    layout->addWidget(chkboxNotAskAgain);

    QVBoxLayout *btnMainLayout = new QVBoxLayout;
    btnMainLayout->setContentsMargins(0, 0, 0, 10);
    btnMainLayout->setSpacing(0);
    btnMainLayout->addLayout(layout);
    btnMainLayout->addLayout(buttonLayout);
    buttonWidget->setLayout(btnMainLayout);
    buttonWidget->setFixedWidth(670);

    connect(btnSkip, &QPushButton::clicked, this, &TaskWidget::onButtonClicked);
    connect(btnReplace, &QPushButton::clicked, this, &TaskWidget::onButtonClicked);
    connect(btnCoexist, &QPushButton::clicked, this, &TaskWidget::onButtonClicked);

    return buttonWidget;
}
/*!
 * \brief TaskWidget::showBtnByAction 根据不同的操作显示不同的按钮
 * \param action
 */
void TaskWidget::showBtnByAction(const AbstractJobHandler::SupportActions &actions)
{
    btnSkip->setHidden(!actions.testFlag(AbstractJobHandler::SupportAction::kSkipAction));
    btnCoexist->setHidden(!actions.testFlag(AbstractJobHandler::SupportAction::kCoexistAction));
    QString btnTxt;
    QVariant variantReplace;
    if (actions.testFlag(AbstractJobHandler::SupportAction::kRetryAction)) {
        btnReplace->setText(tr("Retry", "button"));
        variantReplace.setValue<AbstractJobHandler::SupportAction>(AbstractJobHandler::SupportAction::kRetryAction);
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kReplaceAction)) {
        btnReplace->setText(tr("Replace", "button"));
        variantReplace.setValue<AbstractJobHandler::SupportAction>(AbstractJobHandler::SupportAction::kReplaceAction);
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kMergeAction)) {
        btnReplace->setText(tr("Merge", "button"));
        variantReplace.setValue<AbstractJobHandler::SupportAction>(AbstractJobHandler::SupportAction::kMergeAction);
    } else {
        btnReplace->setHidden(true);
        return;
    }

    btnReplace->setHidden(false);
    btnReplace->setProperty(kBtnPropertyActionName, variantReplace);
}
/*!
 * \brief TaskWidget::showConflictButtons 显示或者隐藏错误信息的界面
 * \param showBtns 是否显示按钮
 * \param showConflict 是否显示错误信息
 */
void TaskWidget::showConflictButtons(bool showBtns, bool showConflict)
{
    Q_UNUSED(showConflict);
    if (!widConfict) {
        return;
    }

    if (!showBtns) {
        widButton->hide();
        widConfict->hide();
    }

    adjustSize();
    emit heightChanged(this->height());
}

void TaskWidget::onMouseHover(const bool hover)
{
    if (isBtnHidden) {
        btnPause->setVisible(false);
        btnStop->setVisible(false);
    } else {
        btnPause->setVisible(hover);
        btnStop->setVisible(hover);
    }

    lbSpeed->setHidden(hover);
    lbRmTime->setHidden(hover);
    lbSrcPath->setFixedWidth(hover ? kMsgLabelHoverWidth : kMsgLabelWidth);
    lbDstPath->setFixedWidth(hover ? kMsgLabelHoverWidth : kMsgLabelWidth);
    adjustSize();
}

QString TaskWidget::formatTime(qint64 second) const
{
    quint8 s = second % 60;
    quint8 m = static_cast<quint8>(second / 60);
    quint8 h = m / 60;
    quint8 d = h / 24;

    m = m % 60;
    h = h % 24;

    QString timeString;

    if (d > 0) {
        timeString.append(QString::number(d)).append(" d");
    }

    if (h > 0) {
        if (!timeString.isEmpty()) {
            timeString.append(' ');
        }

        timeString.append(QString::number(h)).append(" h");
    }

    if (m > 0) {
        if (!timeString.isEmpty()) {
            timeString.append(' ');
        }

        timeString.append(QString::number(m)).append(" m");
    }

    if (s > 0 || timeString.isEmpty()) {
        if (!timeString.isEmpty()) {
            timeString.append(' ');
        }

        timeString.append(QString::number(s)).append(" s");
    }

    return timeString;
}

bool TaskWidget::showFileInfo(const FileInfoPointer info, const bool isOrg)
{
    if (!info)
        return false;

    if (isOrg) {
        originInfo = nullptr;
    } else {
        targetInfo = nullptr;
    }

    bool needRetry = false;

    needRetry = !info->timeOf(TimeInfoType::kLastModifiedSecond).toULongLong();
    auto thumImage = ThumbnailHelper::thumbnailImage(info->urlOf(UrlInfoType::kUrl),
                                                     DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge);

    if (!needRetry)
        info->customData(Global::ItemRoles::kItemFileRefreshIcon);
    auto icon = thumImage.isNull() ? info->fileIcon().pixmap(48, 48) : QPixmap::fromImage(thumImage);

    auto modifyTimeStr = QString(tr("Time modified: %1"))
                                 .arg(info->timeOf(TimeInfoType::kLastModified).value<QDateTime>().isValid()
                                              ? info->timeOf(TimeInfoType::kLastModified).value<QDateTime>().toString("yyyy/MM/dd HH:mm:ss")
                                              : qApp->translate("MimeTypeDisplayManager", "Unknown"));
    auto sizeStr = tr("In data statistics ...");
    auto titleStr = isOrg ? tr("Original folder") : tr("Target folder");
    if (info->isAttributes(OptInfoType::kIsDir)) {
        if (info->countChildFile() < 0) {
            needRetry = true;
        } else {
            QString filecount = info->countChildFile() <= 1 ? QObject::tr("%1 item").arg(info->countChildFile())
                                                            : QObject::tr("%1 items").arg(info->countChildFile());
            sizeStr = QString(tr("Contains: %1")).arg(filecount);
        }
    } else {
        titleStr = isOrg ? tr("Original file") : tr("Target file");
        sizeStr = QString(tr("Size: %1")).arg(info->extendAttributes(ExtInfoType::kSizeFormat).toString());
    }
    if (isOrg) {
        lbSrcIcon->setPixmap(icon);
        lbSrcModTime->setText(modifyTimeStr);
        lbSrcTitle->setText(titleStr);
        lbSrcFileSize->setText(sizeStr);
        originInfo = needRetry ? info : nullptr;
    } else {
        lbDstIcon->setPixmap(icon);
        lbDstModTime->setText(modifyTimeStr);
        lbDstTitle->setText(titleStr);
        lbDstFileSize->setText(sizeStr);
        targetInfo = needRetry ? info : nullptr;
    }

    return needRetry;
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void TaskWidget::enterEvent(QEnterEvent *event)
#else
void TaskWidget::enterEvent(QEvent *event)
#endif
{
    onMouseHover(true);

    return QWidget::enterEvent(event);
}

void TaskWidget::leaveEvent(QEvent *event)
{
    onMouseHover(false);

    return QWidget::leaveEvent(event);
}

void TaskWidget::paintEvent(QPaintEvent *event)
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
            bgColor = QColor(0, 0, 0, 13);   //rgba(0,0,0,13); border-radius: 8px
        } else {
            bgColor = QColor(255, 255, 255, 13);   //rgba(255,255,255,13); border-radius: 8px
        }
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
