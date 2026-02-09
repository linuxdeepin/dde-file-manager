// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QStringList>
#include <QUrl>
#include <QElapsedTimer>

#include <dfm-base/utils/filescanner.h>

DFMBASE_USE_NAMESPACE

class StatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatisticsDialog(const QStringList &paths, const QList<QUrl> &urls, QWidget *parent = nullptr);
    ~StatisticsDialog();

    void startScan();

private Q_SLOTS:
    void onProgressChanged(const FileScanner::ScanResult &result);
    void onFinished(const FileScanner::ScanResult &result);
    void onStopClicked();

private:
    void setupUI(const QStringList &paths);
    QString formatSize(qint64 size) const;

    QList<QUrl> scanUrls;
    FileScanner *scanner;

    QLabel *pathLabel;
    QLabel *fileCountLabel;
    QLabel *dirCountLabel;
    QLabel *totalCountLabel;
    QLabel *progressSizeLabel;
    QLabel *totalSizeLabel;
    QLabel *timeLabel;
    QProgressBar *progressBar;
    QPushButton *stopButton;
    QPushButton *closeButton;

    QElapsedTimer scanTimer;
};

#endif   // STATISTICSDIALOG_H
