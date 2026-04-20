// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
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
#include <QFile>
#include <QTextStream>
#include <QQueue>

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
    void onBrowseDirectory();
    void onExtractDirectory();
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
    void collectFiles(const QString &directory);
    void processNextFile();
    void setupOutputDirectory(const QString &sourceDir);
    void saveExtractionResult(const QString &filePath, const QByteArray &data);
    void saveExtractionError(const QString &filePath, const QString &error);

    DLineEdit *m_extractorPathEdit = nullptr;
    DLineEdit *m_pluginPathEdit = nullptr;
    DLineEdit *m_directoryPathEdit = nullptr;
    DPushButton *m_startBtn = nullptr;
    DPushButton *m_stopBtn = nullptr;
    DPushButton *m_extractBtn = nullptr;
    DPushButton *m_browseDirBtn = nullptr;
    DPushButton *m_browseExtractorBtn = nullptr;
    DTextEdit *m_logView = nullptr;

    dfm_extractor::ControllerPipe *m_controllerPipe = nullptr;
    bool m_extractorRunning = false;

    // Batch processing state
    QQueue<QString> m_pendingFiles;
    QString m_currentFile;
    int m_totalFiles = 0;
    int m_processedFiles = 0;
    int m_successCount = 0;
    int m_failedCount = 0;

    // Output directory
    QString m_outputDir;
};

#endif   // MAINWINDOW_H
