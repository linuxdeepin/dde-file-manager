// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <DFileDialog>
#include <DLog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QDateTime>
#include <QStandardPaths>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent), m_controllerPipe(new dfm_extractor::ControllerPipe(this))
{
    setupUI();

    connect(m_controllerPipe, &dfm_extractor::ControllerPipe::extractionStarted,
            this, &MainWindow::onExtractionStarted);
    connect(m_controllerPipe, &dfm_extractor::ControllerPipe::extractionFinished,
            this, &MainWindow::onExtractionFinished);
    connect(m_controllerPipe, &dfm_extractor::ControllerPipe::extractionFailed,
            this, &MainWindow::onExtractionFailed);
    connect(m_controllerPipe, &dfm_extractor::ControllerPipe::batchFinished,
            this, &MainWindow::onBatchFinished);
    connect(m_controllerPipe, &dfm_extractor::ControllerPipe::processCrashed,
            this, &MainWindow::onProcessCrashed);
    connect(m_controllerPipe, &dfm_extractor::ControllerPipe::processFinished,
            this, &MainWindow::onProcessFinished);
    connect(m_controllerPipe, &dfm_extractor::ControllerPipe::errorOccurred,
            this, [this](const QString &error) {
                log(tr("Extractor error: %1").arg(error));
            });

    updateButtonStates();
}

MainWindow::~MainWindow()
{
    if (m_controllerPipe && m_controllerPipe->isRunning()) {
        m_controllerPipe->stop();
    }
}

void MainWindow::setupUI()
{
    setWindowTitle(tr("Extractor Test"));
    setMinimumSize(800, 600);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Extractor path group
    QGroupBox *extractorGroup = new QGroupBox(tr("Extractor Settings"), this);
    QVBoxLayout *extractorLayout = new QVBoxLayout(extractorGroup);

    // Extractor path
    QHBoxLayout *extractorPathLayout = new QHBoxLayout();
    extractorPathLayout->addWidget(new QLabel(tr("Extractor Path:"), this));
    m_extractorPathEdit = new DLineEdit(this);
    m_extractorPathEdit->setPlaceholderText(tr("Path to dde-file-manager-extractor executable"));
    extractorPathLayout->addWidget(m_extractorPathEdit);
    m_browseExtractorBtn = new DPushButton(tr("Browse..."), this);
    extractorPathLayout->addWidget(m_browseExtractorBtn);
    extractorLayout->addLayout(extractorPathLayout);

    // Plugin path
    QHBoxLayout *pluginPathLayout = new QHBoxLayout();
    pluginPathLayout->addWidget(new QLabel(tr("Plugin Path:"), this));
    m_pluginPathEdit = new DLineEdit(this);
    m_pluginPathEdit->setPlaceholderText(tr("Optional: Path to extractor plugins"));
    pluginPathLayout->addWidget(m_pluginPathEdit);
    extractorLayout->addLayout(pluginPathLayout);

    // Start/Stop buttons
    QHBoxLayout *controlLayout = new QHBoxLayout();
    m_startBtn = new DPushButton(tr("Start Extractor"), this);
    m_startBtn->setEnabled(true);
    controlLayout->addWidget(m_startBtn);

    m_stopBtn = new DPushButton(tr("Stop Extractor"), this);
    m_stopBtn->setEnabled(false);
    controlLayout->addWidget(m_stopBtn);
    controlLayout->addStretch();
    extractorLayout->addLayout(controlLayout);

    mainLayout->addWidget(extractorGroup);

    // File selection group
    QGroupBox *fileGroup = new QGroupBox(tr("File Extraction"), this);
    QVBoxLayout *fileLayout = new QVBoxLayout(fileGroup);

    QHBoxLayout *filePathLayout = new QHBoxLayout();
    filePathLayout->addWidget(new QLabel(tr("File Path:"), this));
    m_filePathEdit = new DLineEdit(this);
    m_filePathEdit->setPlaceholderText(tr("Path to file for extraction"));
    filePathLayout->addWidget(m_filePathEdit);
    m_browseFileBtn = new DPushButton(tr("Browse..."), this);
    filePathLayout->addWidget(m_browseFileBtn);
    fileLayout->addLayout(filePathLayout);

    m_extractBtn = new DPushButton(tr("Extract Content"), this);
    m_extractBtn->setEnabled(false);
    fileLayout->addWidget(m_extractBtn);

    mainLayout->addWidget(fileGroup);

    // Log view
    QGroupBox *logGroup = new QGroupBox(tr("Log"), this);
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);

    m_logView = new DTextEdit(this);
    m_logView->setReadOnly(true);
    logLayout->addWidget(m_logView);

    mainLayout->addWidget(logGroup);

    // Connect signals
    connect(m_browseExtractorBtn, &DPushButton::clicked, this, &MainWindow::onBrowseExtractor);
    connect(m_startBtn, &DPushButton::clicked, this, &MainWindow::onStartExtractor);
    connect(m_stopBtn, &DPushButton::clicked, this, &MainWindow::onStopExtractor);
    connect(m_browseFileBtn, &DPushButton::clicked, this, &MainWindow::onBrowseFile);
    connect(m_extractBtn, &DPushButton::clicked, this, &MainWindow::onExtractFile);

    // Set default paths
    QString defaultExtractorPath = QStandardPaths::findExecutable("dde-file-manager-extractor");
    if (!defaultExtractorPath.isEmpty()) {
        m_extractorPathEdit->setText(defaultExtractorPath);
    }
}

void MainWindow::updateButtonStates()
{
    bool hasPath = !m_extractorPathEdit->text().isEmpty();
    m_startBtn->setEnabled(hasPath && !m_extractorRunning);
    m_stopBtn->setEnabled(m_extractorRunning);
    m_extractBtn->setEnabled(m_extractorRunning);
}

void MainWindow::log(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    m_logView->append(QString("[%1] %2").arg(timestamp).arg(message));
}

void MainWindow::onBrowseExtractor()
{
    QString path = DFileDialog::getOpenFileName(this, tr("Select Extractor Executable"));
    if (!path.isEmpty()) {
        m_extractorPathEdit->setText(path);
        updateButtonStates();
    }
}

void MainWindow::onStartExtractor()
{
    const QString extractorPath = m_extractorPathEdit->text();
    if (extractorPath.isEmpty()) {
        log(tr("Error: Extractor path is empty"));
        return;
    }

    QStringList args;
    const QString pluginPath = m_pluginPathEdit->text();
    if (!pluginPath.isEmpty()) {
        args << "--plugin-path" << pluginPath;
    }

    log(tr("Starting extractor: %1 %2").arg(extractorPath).arg(args.join(" ")));
    if (m_controllerPipe->start(extractorPath, pluginPath)) {
        log(tr("Extractor started successfully (PID: %1)").arg(m_controllerPipe->processId()));
        m_extractorRunning = true;
    } else {
        log(tr("Failed to start extractor"));
    }

    updateButtonStates();
}

void MainWindow::onStopExtractor()
{
    if (m_controllerPipe && m_extractorRunning) {
        log(tr("Stopping extractor..."));
        m_controllerPipe->stop();
        m_extractorRunning = false;
        updateButtonStates();
    }
}

void MainWindow::onBrowseFile()
{
    QString path = DFileDialog::getOpenFileName(this, tr("Select File for Extraction"));
    if (!path.isEmpty()) {
        m_filePathEdit->setText(path);
    }
}

void MainWindow::onExtractFile()
{
    if (!m_controllerPipe || !m_extractorRunning) {
        log(tr("Error: Extractor not running"));
        return;
    }

    const QString filePath = m_filePathEdit->text();
    if (filePath.isEmpty()) {
        log(tr("Error: File path is empty"));
        return;
    }

    if (m_controllerPipe->extractBatch({ filePath })) {
        log(tr("Sent extraction request for: %1").arg(filePath));
    } else {
        log(tr("Failed to send extraction request"));
    }
}

void MainWindow::onExtractionStarted(const QString &filePath)
{
    log(tr("Extraction started: %1").arg(filePath));
}

void MainWindow::onExtractionFinished(const QString &filePath, const QByteArray &data)
{
    log(tr("Extraction finished: %1 (size: %2 bytes)").arg(filePath).arg(data.size()));

    // Show first 500 characters of content
    const QString preview = QString::fromUtf8(data);
    log(tr("Content preview:\n%1").arg(preview));
}

void MainWindow::onExtractionFailed(const QString &filePath, const QString &error)
{
    log(tr("Extraction failed: %1 - %2").arg(filePath).arg(error));
}

void MainWindow::onBatchFinished()
{
    log(tr("Batch extraction completed"));
}

void MainWindow::onProcessCrashed()
{
    log(tr("Extractor process crashed!"));
    m_extractorRunning = false;
    updateButtonStates();
}

void MainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    log(tr("Extractor process finished - code: %1, status: %2").arg(exitCode).arg(exitStatus));
    m_extractorRunning = false;
    updateButtonStates();
}
