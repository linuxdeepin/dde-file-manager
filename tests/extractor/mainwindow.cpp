// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "config.h"

#include <DFileDialog>
#include <DLog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QCryptographicHash>

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

    // Directory selection group
    QGroupBox *dirGroup = new QGroupBox(tr("Directory Extraction"), this);
    QVBoxLayout *dirLayout = new QVBoxLayout(dirGroup);

    QHBoxLayout *directoryPathLayout = new QHBoxLayout();
    directoryPathLayout->addWidget(new QLabel(tr("Directory Path:"), this));
    m_directoryPathEdit = new DLineEdit(this);
    m_directoryPathEdit->setPlaceholderText(tr("Directory containing files to extract"));
    directoryPathLayout->addWidget(m_directoryPathEdit);
    m_browseDirBtn = new DPushButton(tr("Browse..."), this);
    directoryPathLayout->addWidget(m_browseDirBtn);
    dirLayout->addLayout(directoryPathLayout);

    m_extractBtn = new DPushButton(tr("Extract All Files"), this);
    m_extractBtn->setEnabled(false);
    dirLayout->addWidget(m_extractBtn);

    mainLayout->addWidget(dirGroup);

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
    connect(m_browseDirBtn, &DPushButton::clicked, this, &MainWindow::onBrowseDirectory);
    connect(m_extractBtn, &DPushButton::clicked, this, &MainWindow::onExtractDirectory);

    // Set default paths
    QString defaultExtractorPath(DFM_EXTRACTOR_TOOL);
    if (!defaultExtractorPath.isEmpty()) {
        m_extractorPathEdit->setText(defaultExtractorPath);
    }
}

void MainWindow::updateButtonStates()
{
    bool hasPath = !m_extractorPathEdit->text().isEmpty();
    m_startBtn->setEnabled(hasPath && !m_extractorRunning);
    m_stopBtn->setEnabled(m_extractorRunning);
    m_extractBtn->setEnabled(m_extractorRunning && !m_directoryPathEdit->text().isEmpty() && m_pendingFiles.isEmpty());
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

    log(tr("Starting extractor..."));
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

void MainWindow::onBrowseDirectory()
{
    QString path = DFileDialog::getExistingDirectory(this, tr("Select Directory for Extraction"));
    if (!path.isEmpty()) {
        m_directoryPathEdit->setText(path);
        updateButtonStates();
    }
}

void MainWindow::onExtractDirectory()
{
    if (!m_controllerPipe || !m_extractorRunning) {
        log(tr("Error: Extractor not running"));
        return;
    }

    const QString directoryPath = m_directoryPathEdit->text();
    if (directoryPath.isEmpty()) {
        log(tr("Error: Directory path is empty"));
        return;
    }

    QDir dir(directoryPath);
    if (!dir.exists()) {
        log(tr("Error: Directory does not exist"));
        return;
    }

    // Collect all files
    m_pendingFiles.clear();
    collectFiles(directoryPath);

    if (m_pendingFiles.isEmpty()) {
        log(tr("No files found in directory"));
        return;
    }

    // Initialize counters
    m_totalFiles = m_pendingFiles.size();
    m_processedFiles = 0;
    m_successCount = 0;
    m_failedCount = 0;

    // Setup output directory
    setupOutputDirectory(directoryPath);

    log(tr("Found %1 files to process").arg(m_totalFiles));
    log(tr("Output directory: %1").arg(m_outputDir));

    // Start processing
    processNextFile();
    updateButtonStates();
}

void MainWindow::collectFiles(const QString &directory)
{
    QDirIterator it(directory, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        m_pendingFiles.enqueue(it.next());
    }
}

void MainWindow::processNextFile()
{
    if (m_pendingFiles.isEmpty()) {
        log(tr("Extraction completed: %1/%2 success, %3 failed")
                    .arg(m_successCount)
                    .arg(m_totalFiles)
                    .arg(m_failedCount));
        log(tr("Results saved to: %1").arg(m_outputDir));
        updateButtonStates();
        return;
    }

    m_currentFile = m_pendingFiles.dequeue();

    // Extract one file at a time
    if (m_controllerPipe->extractBatch({ m_currentFile })) {
        // Progress logged in onExtractionStarted
    } else {
        log(tr("Failed to send extraction request for: %1").arg(m_currentFile));
        saveExtractionError(m_currentFile, tr("Failed to send request"));
        m_failedCount++;
        m_processedFiles++;
        processNextFile();
    }
}

void MainWindow::setupOutputDirectory(const QString &sourceDir)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString baseName = QFileInfo(sourceDir).baseName();

    // Create output directory under Documents
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    m_outputDir = QString("%1/%2_extracted_%3").arg(documentsPath).arg(baseName).arg(timestamp);

    QDir dir(m_outputDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Create index file
    QString indexPath = m_outputDir + "/_index.txt_content";
    QFile indexFile(indexPath);
    if (indexFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&indexFile);
        stream.setEncoding(QStringConverter::Utf8);
        stream << "Source Directory: " << sourceDir << "\n";
        stream << "Extraction Time: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
        stream << "Total Files: " << m_totalFiles << "\n";
        stream << "\n--- Results ---\n";
        indexFile.close();
    }
}

void MainWindow::saveExtractionResult(const QString &filePath, const QByteArray &data)
{
    if (m_outputDir.isEmpty()) return;

    QFileInfo fi(filePath);
    QString relativePath = fi.absolutePath();
    QString baseName = fi.completeBaseName();
    QString suffix = fi.suffix();

    // Generate unique filename: basename.suffix.txt_content
    // For files with same name in different directories, add hash
    QString outputFileName = QString("%1.%2.txt_content").arg(baseName).arg(suffix);

    // Check for conflicts and add hash if needed
    QString outputPath = m_outputDir + "/" + outputFileName;
    if (QFile::exists(outputPath)) {
        // Add path hash to differentiate
        QByteArray pathHash = QCryptographicHash::hash(filePath.toUtf8(), QCryptographicHash::Md5).toHex().left(8);
        outputFileName = QString("%1_%2.%3.txt_content").arg(baseName).arg(QString(pathHash)).arg(suffix);
        outputPath = m_outputDir + "/" + outputFileName;
    }

    // Write content
    QFile outFile(outputPath);
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&outFile);
        stream.setEncoding(QStringConverter::Utf8);
        stream << QString::fromUtf8(data);
        outFile.close();
    }

    // Append to index
    QString indexPath = m_outputDir + "/_index.txt_content";
    QFile indexFile(indexPath);
    if (indexFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&indexFile);
        stream << "[OK] " << filePath << " -> " << outputFileName << "\n";
        indexFile.close();
    }
}

void MainWindow::saveExtractionError(const QString &filePath, const QString &error)
{
    // Append error to index
    if (m_outputDir.isEmpty()) return;

    QString indexPath = m_outputDir + "/_index.txt_content";
    QFile indexFile(indexPath);
    if (indexFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&indexFile);
        stream << "[FAIL] " << filePath << " - " << error << "\n";
        indexFile.close();
    }
}

void MainWindow::onExtractionStarted(const QString &filePath)
{
    log(tr("[%1/%2] Processing: %3")
                .arg(m_processedFiles + 1)
                .arg(m_totalFiles)
                .arg(QFileInfo(filePath).fileName()));
}

void MainWindow::onExtractionFinished(const QString &filePath, const QByteArray &data)
{
    saveExtractionResult(filePath, data);

    m_successCount++;
    m_processedFiles++;
    processNextFile();
}

void MainWindow::onExtractionFailed(const QString &filePath, const QString &error)
{
    saveExtractionError(filePath, error);

    m_failedCount++;
    m_processedFiles++;
    processNextFile();
}

void MainWindow::onBatchFinished()
{
    // Called when batch is done, we process files sequentially
}

void MainWindow::onProcessCrashed()
{
    log(tr("Extractor process crashed!"));
    m_extractorRunning = false;

    // Record remaining files as failed
    while (!m_pendingFiles.isEmpty()) {
        QString file = m_pendingFiles.dequeue();
        saveExtractionError(file, tr("Extractor crashed"));
        m_failedCount++;
        m_processedFiles++;
    }

    updateButtonStates();
}

void MainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    log(tr("Extractor process finished - code: %1, status: %2").arg(exitCode).arg(exitStatus));
    m_extractorRunning = false;
    updateButtonStates();
}
