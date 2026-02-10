// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESHREDWORKER_H
#define FILESHREDWORKER_H

#include "dfmplugin_utils_global.h"

#include <QObject>

namespace dfmplugin_utils {

class FileShredWorker : public QObject
{
    Q_OBJECT
public:
    explicit FileShredWorker(QObject *parent = nullptr);

public Q_SLOTS:
    void shredFile(const QList<QUrl> &fileList);
    void stop();

Q_SIGNALS:
    void progressUpdated(int percentage, const QString &currentFileName);
    void finished(bool success, const QString &message);

private:
    int countFilesInDirectory(const QString &dirPath);
    bool processDirectory(const QString &dirPath, QString &msg);
    bool executeShredCommandBatch(const QStringList &filePaths, QString &msg);
    void parseShredOutput(const QString &output);
    bool isPipe(const QString &path) const;
    int calculateProgress(int processedFiles, int totalFiles, int currentFileProgress = 0);

    // Member variables for state tracking
    int m_totalFiles;
    int m_processedFiles;
    QString m_currentFileName;
    bool m_shouldStop;
};

}   // namespace dfmplugin_utils

#endif   // FILESHREDWORKER_H
