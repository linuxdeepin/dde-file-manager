// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "env/envdetector.h"
#include "env/powermonitor.h"
#include "env/idlemonitor.h"
#include "env/loadmonitor.h"

#include <DMainWindow>
#include <DLineEdit>
#include <DPushButton>
#include <DTextBrowser>

#include <QTimer>

DWIDGET_USE_NAMESPACE

SERVICETEXTINDEX_USE_NAMESPACE

class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onStart();
    void onStop();
    void onClearLog();
    void refreshDisplay();

    void onEnvStateChanged(const EnvState &state);
    void onPowerChanged(bool onBattery, bool powerSave);
    void onIdleChanged(bool idle);
    void onLoadChanged(double cpuAvgPercent, double diskBusyPercent);

private:
    void setupUI();
    void logEvent(const QString &category, const QString &message);

    DLineEdit *m_pathEdit = nullptr;
    DPushButton *m_startBtn = nullptr;
    DPushButton *m_stopBtn = nullptr;
    DPushButton *m_clearBtn = nullptr;
    DTextBrowser *m_statusView = nullptr;
    DTextBrowser *m_logView = nullptr;
    QTimer *m_refreshTimer = nullptr;

    EnvDetector *m_envDetector = nullptr;   // singleton reference (not owned)
    PowerMonitor *m_powerMonitor = nullptr;
    IdleMonitor *m_idleMonitor = nullptr;
    LoadMonitor *m_loadMonitor = nullptr;

    bool m_running = false;
};

#endif   // MAINWINDOW_H
