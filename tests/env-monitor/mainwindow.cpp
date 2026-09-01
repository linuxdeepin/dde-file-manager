// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QScrollBar>
#include <QTextCursor>

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
    , m_envDetector(&EnvDetector::instance())
    , m_powerMonitor(new PowerMonitor(this))
    , m_idleMonitor(new IdleMonitor(this))
    , m_loadMonitor(new LoadMonitor(this))
{
    setupUI();

    connect(m_envDetector, &EnvDetector::envStateChanged,
            this, &MainWindow::onEnvStateChanged);
    connect(m_powerMonitor, &PowerMonitor::effectivePowerChanged,
            this, &MainWindow::onPowerChanged);
    connect(m_idleMonitor, &IdleMonitor::idleChanged,
            this, &MainWindow::onIdleChanged);
    connect(m_loadMonitor, &LoadMonitor::loadChanged,
            this, &MainWindow::onLoadChanged);

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(1000);
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWindow::refreshDisplay);
    m_refreshTimer->start();
}

MainWindow::~MainWindow()
{
    if (m_running) {
        // EnvDetector is a singleton — don't stop it here, only stop the
        // monitors we own.
        m_powerMonitor->stop();
        m_idleMonitor->stop();
        m_loadMonitor->stop();
    }
}

void MainWindow::setupUI()
{
    setWindowTitle(tr("Env Monitor Test"));
    setMinimumSize(900, 700);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    QGroupBox *controlGroup = new QGroupBox(tr("Control"), this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlGroup);

    controlLayout->addWidget(new QLabel(tr("Data Path:"), this));
    m_pathEdit = new DLineEdit(this);
    m_pathEdit->setText(QDir::homePath());
    controlLayout->addWidget(m_pathEdit);

    m_startBtn = new DPushButton(tr("Start"), this);
    controlLayout->addWidget(m_startBtn);

    m_stopBtn = new DPushButton(tr("Stop"), this);
    m_stopBtn->setEnabled(false);
    controlLayout->addWidget(m_stopBtn);

    m_clearBtn = new DPushButton(tr("Clear Log"), this);
    controlLayout->addWidget(m_clearBtn);

    mainLayout->addWidget(controlGroup);

    QGroupBox *statusGroup = new QGroupBox(tr("Status"), this);
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
    m_statusView = new DTextBrowser(this);
    m_statusView->setReadOnly(true);
    m_statusView->setOpenExternalLinks(false);
    statusLayout->addWidget(m_statusView);
    statusGroup->setMinimumHeight(300);
    mainLayout->addWidget(statusGroup, 3);

    QGroupBox *logGroup = new QGroupBox(tr("Event Log"), this);
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    m_logView = new DTextBrowser(this);
    m_logView->setReadOnly(true);
    m_logView->setOpenExternalLinks(false);
    logLayout->addWidget(m_logView);
    mainLayout->addWidget(logGroup, 2);

    connect(m_startBtn, &DPushButton::clicked, this, &MainWindow::onStart);
    connect(m_stopBtn, &DPushButton::clicked, this, &MainWindow::onStop);
    connect(m_clearBtn, &DPushButton::clicked, this, &MainWindow::onClearLog);

    refreshDisplay();
}

void MainWindow::onStart()
{
    const QString dataPath = m_pathEdit->text();
    if (dataPath.isEmpty()) {
        logEvent("Error", "Data path is empty");
        return;
    }

    m_envDetector->setDataPath(dataPath);
    m_loadMonitor->setDataPath(dataPath);

    m_envDetector->start();
    m_powerMonitor->start();
    m_idleMonitor->start();
    m_loadMonitor->start();

    m_running = true;
    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);

    logEvent("System", "All monitors started (dataPath=" + dataPath + ")");
    refreshDisplay();
}

void MainWindow::onStop()
{
    // EnvDetector is a singleton — don't stop it, only stop owned monitors.
    m_powerMonitor->stop();
    m_idleMonitor->stop();
    m_loadMonitor->stop();

    m_running = false;
    m_startBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);

    logEvent("System", "All monitors stopped");
    refreshDisplay();
}

void MainWindow::onClearLog()
{
    m_logView->clear();
}

void MainWindow::refreshDisplay()
{
    const QString greenFmt = "<span style='color:green;'>%1</span>";
    const QString redFmt = "<span style='color:red;'>%1</span>";

    auto boolColor = [&](bool condition, const QString &label) -> QString {
        QString text = label + ": " + (condition ? "true" : "false");
        return condition ? greenFmt.arg(text) : redFmt.arg(text);
    };

    auto coloredValue = [&](bool ok, const QString &label, bool value) -> QString {
        QString text = label + ": " + QString(value ? "true" : "false");
        return ok ? greenFmt.arg(text) : redFmt.arg(text);
    };

    auto labeledValue = [&](bool condition, const QString &label,
                            const QString &trueDesc, const QString &falseDesc) -> QString {
        QString text = label + ": " + (condition ? "true" : "false") +
                       " (" + (condition ? trueDesc : falseDesc) + ")";
        return condition ? greenFmt.arg(text) : redFmt.arg(text);
    };

    auto numericLabeled = [&](bool condition, const QString &label,
                              const QString &detail) -> QString {
        QString text = label + " " + (condition ? "true" : "false") + " " + detail;
        return condition ? greenFmt.arg(text) : redFmt.arg(text);
    };

    QString html;
    html += "<pre style='font-family:monospace; font-size:14px;'>";

    // EnvDetector
    html += "========== EnvDetector (聚合状态) ==========<br>";
    html += "运行状态: " + QString(m_running ? "running" : "stopped") + "<br>";
    EnvState state = m_envDetector->currentState();
    html += "onBattery: " + QString(state.onBattery ? "true" : "false") + "<br>";
    html += "powerSaveMode: " + QString(state.powerSaveMode ? "true" : "false") + "<br>";
    html += "idle: " + QString(state.idle ? "true" : "false") + "<br>";
    html += labeledValue(state.isPowerOk(), "isPowerOk()",
                         "电源OK", "使用电池") + "<br>";
    html += labeledValue(state.isPowerSaveOff(), "isPowerSaveOff()",
                         "省电已关闭", "省电模式中") + "<br>";
    html += labeledValue(state.isIdleOk(), "isIdleOk()",
                         "空闲OK", "用户活动中") + "<br>";

    // PowerMonitor
    html += "<br>========== PowerMonitor (电源监控) ==========<br>";
    html += labeledValue(!m_powerMonitor->onBattery(), "onBattery()",
                         "使用市电", "使用电池") + "<br>";
    html += labeledValue(!m_powerMonitor->powerSaveMode(), "powerSaveMode()",
                         "非省电模式", "省电模式") + "<br>";

    // IdleMonitor
    html += "<br>========== IdleMonitor (空闲监控) ==========<br>";
    html += labeledValue(m_idleMonitor->idle(), "idle()",
                         "用户空闲", "用户活动中") + "<br>";

    // LoadMonitor
    html += "<br>========== LoadMonitor (负载监控) ==========<br>";
    double cpu = m_loadMonitor->cpuAvgPercent();
    double disk = m_loadMonitor->diskBusyPercent();
    bool cpuOk = m_loadMonitor->isCpuBelowThreshold();
    bool diskOk = m_loadMonitor->isDiskBelowThreshold();
    html += QString("cpuAvgPercent(): %1 % (instant: %2 %)<br>").arg(cpu, 0, 'f', 2).arg(m_loadMonitor->cpuInstantPercent(), 0, 'f', 2);
    html += QString("diskBusyPercent(): %1 % (instant: %2 %)<br>").arg(disk, 0, 'f', 2).arg(m_loadMonitor->diskInstantPercent(), 0, 'f', 2);
    html += numericLabeled(cpuOk, "isCpuBelowThreshold():",
                           QStringLiteral("(阈值: %1 %)").arg(m_loadMonitor->cpuThresholdPercent())) + "<br>";
    html += numericLabeled(diskOk, "isDiskBelowThreshold():",
                           QStringLiteral("(阈值: %1 %)").arg(m_loadMonitor->diskThresholdPercent())) + "<br>";

    html += "</pre>";

    // Skip refresh while user is selecting text — setHtml replaces the
    // entire document and destroys any active selection.
    if (m_statusView->textCursor().hasSelection()) {
        return;
    }

    // Save scroll position before setHtml resets it to top
    QScrollBar *vBar = m_statusView->verticalScrollBar();
    int scrollPos = vBar ? vBar->value() : 0;

    m_statusView->setHtml(html);

    if (vBar) {
        vBar->setValue(scrollPos);
    }
}

void MainWindow::onEnvStateChanged(const EnvState &state)
{
    logEvent("EnvDetector",
             QString("envStateChanged -> onBattery=%1 powerSaveMode=%2 idle=%3 (powerOk=%4 powerSaveOff=%5 idleOk=%6)")
                     .arg(state.onBattery)
                     .arg(state.powerSaveMode)
                     .arg(state.idle)
                     .arg(state.isPowerOk())
                     .arg(state.isPowerSaveOff())
                     .arg(state.isIdleOk()));
}

void MainWindow::onPowerChanged(bool onBattery, bool powerSave)
{
    logEvent("PowerMonitor",
             QString("effectivePowerChanged -> onBattery=%1 powerSave=%2")
                     .arg(onBattery)
                     .arg(powerSave));
}

void MainWindow::onIdleChanged(bool idle)
{
    logEvent("IdleMonitor",
             QString("idleChanged -> idle=%1").arg(idle));
}

void MainWindow::onLoadChanged(double cpuAvgPercent, double diskBusyPercent)
{
    logEvent("LoadMonitor",
             QString("loadChanged -> cpuAvg=%1% diskAvg=%2% | cpuNow=%3% diskNow=%4%")
                     .arg(cpuAvgPercent, 0, 'f', 2)
                     .arg(diskBusyPercent, 0, 'f', 2)
                     .arg(m_loadMonitor->cpuInstantPercent(), 0, 'f', 2)
                     .arg(m_loadMonitor->diskInstantPercent(), 0, 'f', 2));
}

void MainWindow::logEvent(const QString &category, const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString line = QString("[%1] [%2] %3").arg(timestamp, category, message);
    m_logView->append(line);
}
