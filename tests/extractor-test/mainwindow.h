// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <DLineEdit>
#include <DPushButton>
#include <DTextEdit>

#include <controllerpipe.h>

#include <QProcess>

DWIDGET_USE_NAMESPACE

/**
 * @brief MainWindow provides a simple UI for testing the extractor.
 */
class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onBrowseExtractor();
    void onStartExtractor();
    void onStopExtractor();
    void onBrowseFile();
    void onExtractFile();
    void onExtractionStarted(const QString &filePath);
    void onExtractionFinished(const QString &filePath, const QByteArray &data);
    void onExtractionFailed(const QString &filePath, const QString &error);
    void onBatchFinished();
    void onProcessCrashed();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void setupUI();
    void updateButtonStates();
    void log(const QString &message);

    DLineEdit *m_extractorPathEdit = nullptr;
    DLineEdit *m_pluginPathEdit = nullptr;
    DLineEdit *m_filePathEdit = nullptr;
    DPushButton *m_startBtn = nullptr;
    DPushButton *m_stopBtn = nullptr;
    DPushButton *m_extractBtn = nullptr;
    DPushButton *m_browseFileBtn = nullptr;
    DPushButton *m_browseExtractorBtn = nullptr;
    DTextEdit *m_logView = nullptr;

    dfm_extractor::ControllerPipe *m_controllerPipe = nullptr;
    bool m_extractorRunning = false;
};

#endif   // MAINWINDOW_H
