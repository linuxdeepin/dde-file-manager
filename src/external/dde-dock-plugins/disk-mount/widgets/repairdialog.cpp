// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "repairdialog.h"

#include <QFrame>
#include <QVBoxLayout>
#include <QTimer>
#include <QDateTime>
#include <QFont>
#include <cmath>
#include <DLabel>
#include <DWaterProgress>

DWIDGET_USE_NAMESPACE

RepairDialog::RepairDialog(QWidget *parent)
    : DDialog(parent)
{
    setFixedSize(360, 250);
}

void RepairDialog::setDeviceInfo(const QString &deviceName, const QString &deviceSize, const QString &fsType)
{
    m_deviceName = deviceName;
    m_deviceSize = deviceSize;
    m_fsType = fsType;
}

void RepairDialog::setDevicePath(const QString &devicePath, const QString &mountPoint)
{
    m_devicePath = devicePath;
    m_mountPoint = mountPoint;
}

void RepairDialog::setErrorCode(const QString &errorCode)
{
    m_errorCode = errorCode;
}

void RepairDialog::setState(RepairState state)
{
    m_state = state;

    // Stop simulated progress timer and reset widget pointer before
    // clearing contents, otherwise the timer fires on a deleted widget.
    if (m_progressTimer)
        m_progressTimer->stop();
    m_progressWidget = nullptr;

    // 清除现有内容和按钮
    clearContents();
    clearButtons();

    switch (state) {
    case kConfirm:
        initConfirmUI();
        break;
    case kRepairing:
        initRepairingUI();
        break;
    case kSuccess:
        initSuccessUI();
        break;
    case kFailed:
        initFailedUI();
        break;
    }

    // Re-activate the window after content rebuild so buttons are
    // properly rendered and receive focus (not greyed out).
    if (isVisible()) {
        raise();
        activateWindow();
    }
}

void RepairDialog::setProgress(int percent)
{
    if (!m_progressWidget)
        return;

    if (percent >= 0 && percent <= 100) {
        m_progressWidget->setVisible(true);

        // Start DWaterProgress if not already started
        if (!m_progressWidget->property("running").toBool()) {
            m_progressWidget->start();
            m_progressWidget->setProperty("running", true);
        }
        m_progressWidget->setValue(percent);
    }
}

void RepairDialog::reject()
{
    // 修复进行中不允许关闭
    if (m_state == kRepairing) {
        return;
    }
    DDialog::reject();
}

void RepairDialog::initConfirmUI()
{
    setIcon(QIcon::fromTheme("dde-file-manager"));
    setTitle(tr("Repair Storage Device"));

    QFrame *mainFrame = new QFrame(this);
    QVBoxLayout *mainLay = new QVBoxLayout(mainFrame);

    QString subtitle = tr("Preparing to repair device: %1 (%2 %3)")
                          .arg(m_deviceName, m_deviceSize, m_fsType);
    DLabel *subtitleLabel = new DLabel(subtitle, mainFrame);
    subtitleLabel->setAlignment(Qt::AlignLeft);

    QString message = tr("The device will be unmounted during repair. "
                        "It will be automatically remounted after repair is complete. "
                        "Please do not remove the device during the repair process.");
    DLabel *msgLabel = new DLabel(message, mainFrame);
    msgLabel->setWordWrap(true);
    msgLabel->setAlignment(Qt::AlignLeft);

    mainLay->addWidget(subtitleLabel);
    mainLay->addSpacing(10);
    mainLay->addWidget(msgLabel);
    mainFrame->setLayout(mainLay);

    addContent(mainFrame);

    addButton(tr("Cancel"));
    addButton(tr("Start Repair"), true, ButtonType::ButtonRecommend);

    setCloseButtonVisible(true);
    setOnButtonClickedClose(false);
}

void RepairDialog::initRepairingUI()
{
    setIcon(QIcon::fromTheme("dde-file-manager"));
    setTitle(tr("Repairing Storage Device"));

    QFrame *mainFrame = new QFrame(this);
    QVBoxLayout *mainLay = new QVBoxLayout(mainFrame);

    QString subtitle = tr("Repairing device: %1 (%2 %3)")
                          .arg(m_deviceName, m_deviceSize, m_fsType);
    DLabel *subtitleLabel = new DLabel(subtitle, mainFrame);
    subtitleLabel->setAlignment(Qt::AlignCenter);

    // Create progress widget
    m_progressWidget = new DTK_WIDGET_NAMESPACE::DWaterProgress(mainFrame);
    m_progressWidget->setValue(0);
    m_progressWidget->setFixedSize(60, 60);

    mainLay->addWidget(subtitleLabel);
    mainLay->addSpacing(20);
    mainLay->addWidget(m_progressWidget, 0, Qt::AlignCenter);
    mainFrame->setLayout(mainLay);

    addContent(mainFrame);

    setCloseButtonVisible(false);
    setOnButtonClickedClose(false);

    // 自动启动模拟进度
    startSimulatedProgress();
}

void RepairDialog::initSuccessUI()
{
    setIcon(QIcon::fromTheme("dde-file-manager"));
    setTitle(tr("Repair Complete"));

    QFrame *mainFrame = new QFrame(this);
    QVBoxLayout *mainLay = new QVBoxLayout(mainFrame);

    QString message = tr("The device has been successfully repaired. "
                        "The device has been remounted and is now ready to use.");
    DLabel *msgLabel = new DLabel(message, mainFrame);
    msgLabel->setWordWrap(true);
    msgLabel->setAlignment(Qt::AlignCenter);

    mainLay->addSpacing(20);
    mainLay->addWidget(msgLabel);
    mainFrame->setLayout(mainLay);

    addContent(mainFrame);

    addButton(tr("Close"));
    addButton(tr("Open Device"), true, ButtonType::ButtonRecommend);

    setCloseButtonVisible(true);
    setOnButtonClickedClose(true);
}

void RepairDialog::initFailedUI()
{
    setIcon(QIcon::fromTheme("dde-file-manager"));
    setTitle(tr("Repair Failed"));

    QFrame *mainFrame = new QFrame(this);
    QVBoxLayout *mainLay = new QVBoxLayout(mainFrame);

    QString message = tr("Failed to repair the device. This may be due to serious format errors "
                        "or physical damage. To protect your data, it is recommended to stop "
                        "writing new files and try using professional data recovery software "
                        "or seek manual assistance.");
    DLabel *msgLabel = new DLabel(message, mainFrame);
    msgLabel->setWordWrap(true);
    msgLabel->setAlignment(Qt::AlignCenter);

    mainLay->addSpacing(20);
    mainLay->addWidget(msgLabel);

    // 错误码显示
    if (!m_errorCode.isEmpty()) {
        DLabel *errorCodeLabel = new DLabel(mainFrame);
        QString errorCodeText = tr("Error Code: %1").arg(m_errorCode);
        errorCodeLabel->setText(errorCodeText);
        errorCodeLabel->setAlignment(Qt::AlignCenter);

        // 设置字体样式，让它看起来像代码
        QFont errorCodeFont = errorCodeLabel->font();
        errorCodeFont.setFamily("Monospace");
        errorCodeLabel->setFont(errorCodeFont);

        // 设置文本省略和tooltip
        errorCodeLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        errorCodeLabel->setToolTip(m_errorCode);  // 鼠标悬停显示完整错误码

        // 设置固定宽度来强制省略
        errorCodeLabel->setMaximumWidth(280);
        errorCodeLabel->setWordWrap(false);

        mainLay->addSpacing(10);
        mainLay->addWidget(errorCodeLabel);
    }
    mainFrame->setLayout(mainLay);

    addContent(mainFrame);

    addButton(tr("Close"), true);

    setCloseButtonVisible(true);
    setOnButtonClickedClose(true);
}

void RepairDialog::startSimulatedProgress()
{
    // 创建定时器用于更新进度
    if (!m_progressTimer) {
        m_progressTimer = new QTimer(this);
        connect(m_progressTimer, &QTimer::timeout, this, &RepairDialog::updateSimulatedProgress);
    }

    // 记录开始时间
    m_progressStartTime = QDateTime::currentMSecsSinceEpoch();

    // 启动进度条动画
    if (m_progressWidget && !m_progressWidget->property("running").toBool()) {
        m_progressWidget->start();
        m_progressWidget->setProperty("running", true);
    }

    // 每100毫秒更新一次进度
    m_progressTimer->start(100);
}

void RepairDialog::updateSimulatedProgress()
{
    qint64 elapsedMs = QDateTime::currentMSecsSinceEpoch() - m_progressStartTime;
    double elapsedSec = elapsedMs / 1000.0;  // 转换为秒
    int progress = 0;

    if (elapsedSec <= 120) {
        // 前2分钟：使用easeOutQuad从0%涨到90%
        double t = elapsedSec / 120.0;  // 0.0 到 1.0
        double easedT = easeOutQuad(t);  // 应用缓动函数
        progress = static_cast<int>(easedT * 90);
    } else {
        // 2分钟后：使用对数增长，越到后面越慢
        // 2分钟: 90%
        // 10分钟: 95%
        // 30分钟: 98%
        // 最终趋近99%

        double timeBeyond2Min = elapsedSec - 120.0;
        double logFactor = log(1.0 + timeBeyond2Min / 60.0) / log(29.0);  // 归一化到0-1
        double additionalProgress = logFactor * 9.0;  // 从90%再增长9%
        progress = 90 + static_cast<int>(additionalProgress);

        // 如果超过60分钟，停止在99%
        if (elapsedSec >= 3600) {
            progress = kMaxProgress;
            m_progressTimer->stop();
        }
    }

    // 确保不超过最大进度
    if (progress > kMaxProgress) {
        progress = kMaxProgress;
    }

    setProgress(progress);
}
