// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileshredworker.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/dfm_base_global.h>

#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QFile>

#include <sys/stat.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_utils;

FileShredWorker::FileShredWorker(QObject *parent)
    : QObject(parent), m_totalFiles(0), m_processedFiles(0), m_shouldStop(false)
{
}

void FileShredWorker::shredFile(const QList<QUrl> &fileList)
{
    m_totalFiles = 0;
    m_processedFiles = 0;
    m_shouldStop = false;

    QStringList regularFiles;
    QStringList directories;

    // Phase 1: File statistics (0-1% progress)
    emit progressUpdated(0, tr("Analyzing files..."));

    for (const auto &file : fileList) {
        if (m_shouldStop) {
            emit finished(false, tr("Operation cancelled"));
            return;
        }

        const auto &localFile = file.toLocalFile();
        if (isPipe(localFile)) {
            QFile::remove(localFile);
            continue;
        }

        QFileInfo fileInfo(localFile);
        if (fileInfo.isSymLink()) {
            QFile::remove(localFile);
        } else if (fileInfo.isDir()) {
            directories << localFile;
            m_totalFiles += countFilesInDirectory(localFile);
        } else {
            regularFiles << localFile;
            m_totalFiles++;
        }
    }

    fmInfo() << "Total files to shred: " << m_totalFiles;
    emit progressUpdated(1, tr("Starting file shredding..."));

    // Phase 2: File shredding (1-100% progress)
    if (!regularFiles.isEmpty()) {
        if (!executeShredCommandBatch(regularFiles)) {
            return;   // Error already emitted
        }
    }

    // Process directories
    for (const QString &dir : directories) {
        if (m_shouldStop) {
            emit finished(false, tr("Operation cancelled"));
            return;
        }
        processDirectory(dir);
    }

    emit finished(true, tr("All files have been successfully shredded"));
}

void FileShredWorker::stop()
{
    m_shouldStop = true;
}

int FileShredWorker::countFilesInDirectory(const QString &dirPath)
{
    int count = 0;
    QDir dir(dirPath);

    // Use QDir::Hidden flag to include hidden files and directories
    QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden | QDir::System);

    for (const QFileInfo &entry : entries) {
        if (entry.isSymLink()) {
            count++;
        } else if (entry.isDir()) {
            count += countFilesInDirectory(entry.absoluteFilePath());
            count++;
        } else {
            count++;
        }
    }

    return count;
}

void FileShredWorker::processDirectory(const QString &dirPath)
{
    if (m_shouldStop) return;

    QDir dir(dirPath);
    // Use QDir::Hidden and QDir::System flags to include hidden and system files
    QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden | QDir::System);

    // Collect regular files and hidden files paths
    QStringList filesToShred;
    QStringList dirsToProcess;

    for (const QFileInfo &entry : entries) {
        if (m_shouldStop) return;

        if (isPipe(entry.absoluteFilePath())) {
            QFile::remove(entry.absoluteFilePath());
            continue;   // Skip pipe files after deletion
        }

        if (entry.isSymLink()) {
            QFile::remove(entry.absoluteFilePath());
        } else if (entry.isDir()) {
            dirsToProcess << entry.absoluteFilePath();
        } else {
            filesToShred << entry.absoluteFilePath();
        }
    }

    // Process files in batch first
    if (!filesToShred.isEmpty()) {
        if (!executeShredCommandBatch(filesToShred))
            return;
    }

    // Recursively process subdirectories
    for (const QString &subDir : dirsToProcess) {
        if (m_shouldStop) return;
        processDirectory(subDir);
    }

    // Finally remove the current directory
    dir.rmdir(dirPath);
}

bool FileShredWorker::executeShredCommandBatch(const QStringList &filePaths)
{
    if (filePaths.isEmpty())
        return true;

    QStringList args;
    args << "-u"
         << "-f"
         << "-v"
         << "-n"
         << "3";
    args.append(filePaths);

    QProcess process;

    // Set fixed Chinese environment
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "zh_CN.UTF-8");
    env.insert("LC_ALL", "zh_CN.UTF-8");
    env.insert("LANGUAGE", "zh_CN:zh");
    process.setProcessEnvironment(env);

    process.start("shred", args);

    if (!process.waitForStarted()) {
        fmWarning() << "Failed to start shred command for batch operation";
        emit finished(false, tr("Failed to start shred command"));
        return false;
    }

    QString savedErrorMsg;   // Used to save error information

    while (process.state() != QProcess::NotRunning) {
        if (m_shouldStop) {
            process.kill();
            process.waitForFinished(3000);
            return false;
        }

        process.waitForReadyRead(100);
        QString output = QString::fromLocal8Bit(process.readAllStandardError());

        // Only extract file path when error message hasn't been saved yet
        if (savedErrorMsg.isEmpty() && output.contains("无法以写模式打开")) {
            static QRegularExpression rx("shred:\\s*([^：]+)");   // Only match filename/path part before colon
            auto match = rx.match(output);
            if (match.hasMatch()) {
                savedErrorMsg = match.captured(1).trimmed();   // Only save the first encountered filename/path
            }
        }

        parseShredOutput(output);
    }

    if (process.exitCode() != 0) {
        QString errorMsg;
        errorMsg = savedErrorMsg + ": " + tr("Permission denied");

        if (savedErrorMsg.isEmpty())
            errorMsg = tr("The file has been moved or the process has exited");

        fmWarning() << "Batch shred command failed: " << process.exitCode() << errorMsg;
        emit finished(false, errorMsg);
        return false;
    }

    return true;
}

bool FileShredWorker::isPipe(const QString &path) const
{
    struct stat sb;
    if (stat(path.toLocal8Bit().constData(), &sb) == -1) {
        return false;
    }
    return S_ISFIFO(sb.st_mode);
}

void FileShredWorker::parseShredOutput(const QString &output)
{
    if (output.isEmpty())
        return;

    QString logOutput = output.trimmed();
    int deletedCount = output.count("已删除");
    if (deletedCount > 0) {
        m_processedFiles += deletedCount;
    }

    // Modified regex to more accurately match shred output format
    static QRegularExpression rx(R"(shred:\s*([^：]+):\s*第\s*(\d+)\s*次，共\s*(\d+)\s*次\s*\(random\)\.\.\.((?:[\.\d]+[GM]iB/[\.\d]+[GM]iB\s*)?(\d+)?%)?)");
    auto match = rx.match(output);
    if (!match.hasMatch()) {
        return;
    }

    // Extract information from regex capture groups
    QString filePath = match.captured(1).trimmed();   // Add trimmed() to remove possible whitespace
    int currentPass = match.captured(2).toInt();
    int totalPasses = match.captured(3).toInt();

    // Get filename
    QFileInfo fileInfo(filePath);
    QString currentFile = fileInfo.fileName();
    m_currentFileName = currentFile;

    fmDebug() << "Extracted file path: " << filePath;
    int totalProgress = 0;

    // If total files is 1, it's single file mode
    if (m_totalFiles == 1) {
        // Get progress percentage
        int currentPassProgress = match.captured(5).isEmpty() ? 0 : match.captured(5).toInt();

        if (!match.captured(4).isEmpty()) {
            // Large file format, use detailed progress
            // Calculate overall progress of single file
            double fileProgress = ((currentPass - 1) * 100.0 / totalPasses)   // Progress of completed passes
                    + (currentPassProgress / 100.0) * (100.0 / totalPasses);   // Progress of current pass
            totalProgress = static_cast<int>(fileProgress);
        } else {
            // Small file format, calculate only based on current pass
            totalProgress = ((currentPass - 1) * 100) / totalPasses;
        }

        if (!match.captured(4).isEmpty()) {
            fmDebug() << "\n    - Size info: " << match.captured(4).toStdString();
        }

        fmDebug() << "\n    - Total progress: " << totalProgress << "%";
    } else {
        totalProgress = calculateProgress(m_processedFiles, m_totalFiles,
                                          ((currentPass - 1) * 100) / totalPasses);

        fmDebug() << "Multi-file progress details:"
                  << "\n    - Current file: " << currentFile
                  << "\n    - Processed files: " << m_processedFiles
                  << "\n    - Total files: " << m_totalFiles
                  << "\n    - Total progress: " << totalProgress << "%";
    }

    totalProgress = std::min(totalProgress, 99);
    emit progressUpdated(totalProgress, currentFile);
}

int FileShredWorker::calculateProgress(int processedFiles, int totalFiles, int currentFileProgress)
{
    if (totalFiles <= 0) return 0;

    // Statistics phase takes 1%, shredding takes 99%
    int baseProgress = 1 + (processedFiles * 99 / totalFiles);

    // Add current file progress contribution
    if (currentFileProgress > 0 && totalFiles > 0) {
        int currentFileContribution = (90 / totalFiles) * (currentFileProgress / 100);
        baseProgress += currentFileContribution;
    }

    return std::min(baseProgress, 99);
}
