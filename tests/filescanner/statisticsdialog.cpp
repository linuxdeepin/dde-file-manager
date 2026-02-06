// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "statisticsdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

StatisticsDialog::StatisticsDialog(const QStringList &paths, const QList<QUrl> &urls, QWidget *parent)
    : QDialog(parent), scanUrls(urls), scanner(nullptr)
{
    setupUI(paths);
}

StatisticsDialog::~StatisticsDialog()
{
    if (scanner) {
        scanner->stop();
        scanner->deleteLater();
    }
}

void StatisticsDialog::setupUI(const QStringList &paths)
{
    QString displayPath;
    if (paths.size() == 1) {
        displayPath = paths.first();
    } else {
        displayPath = QString("%1 个目录").arg(paths.size());
    }

    setWindowTitle("目录统计 - " + displayPath);
    setMinimumWidth(450);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 路径显示
    QString pathText;
    if (paths.size() <= 3) {
        pathText = "路径: " + paths.join(", ");
    } else {
        pathText = "路径: " + paths.mid(0, 3).join(", ") + QString(" ... (共%1个)").arg(paths.size());
    }
    pathLabel = new QLabel(pathText, this);
    pathLabel->setWordWrap(true);
    mainLayout->addWidget(pathLabel);

    // 统计结果分组
    QGroupBox *resultGroup = new QGroupBox("统计结果", this);
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);

    fileCountLabel = new QLabel("文件数量: 计算中...", this);
    resultLayout->addWidget(fileCountLabel);

    dirCountLabel = new QLabel("目录数量: 计算中...", this);
    resultLayout->addWidget(dirCountLabel);

    totalCountLabel = new QLabel("总数量: 计算中...", this);
    resultLayout->addWidget(totalCountLabel);

    progressSizeLabel = new QLabel("进度大小: 计算中...", this);
    resultLayout->addWidget(progressSizeLabel);

    totalSizeLabel = new QLabel("总大小: 计算中...", this);
    resultLayout->addWidget(totalSizeLabel);

    mainLayout->addWidget(resultGroup);

    // 进度条
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 0);   // 不确定进度
    progressBar->setTextVisible(false);
    mainLayout->addWidget(progressBar);

    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    stopButton = new QPushButton("停止", this);
    connect(stopButton, &QPushButton::clicked, this, &StatisticsDialog::onStopClicked);
    buttonLayout->addWidget(stopButton);

    closeButton = new QPushButton("关闭", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(closeButton);

    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
}

void StatisticsDialog::startScan()
{
    // 创建扫描器（默认排除源目录本身）
    scanner = new FileScanner(this);
    scanner->setOptions(FileScanner::ScanOption::NoOption);

    // 连接信号
    connect(scanner, &FileScanner::progressChanged,
            this, &StatisticsDialog::onProgressChanged);
    connect(scanner, &FileScanner::finished,
            this, &StatisticsDialog::onFinished);

    // 开始扫描
    scanner->start(scanUrls);

    // 更新UI状态
    stopButton->setEnabled(true);
    closeButton->setEnabled(false);
    progressBar->setRange(0, 0);
}

QString StatisticsDialog::formatSize(qint64 size) const
{
    // 格式化数字为千分位分隔
    QString sizeStr = QString("%L1").arg(size);

    if (size < 1024) {
        return QString("%1 B (%2)").arg(size).arg(sizeStr);
    } else if (size < 1024 * 1024) {
        double kb = size / 1024.0;
        return QString("%1 KB (%2)").arg(kb, 0, 'f', 2).arg(sizeStr);
    } else if (size < 1024 * 1024 * 1024) {
        double mb = size / (1024.0 * 1024.0);
        return QString("%1 MB (%2)").arg(mb, 0, 'f', 2).arg(sizeStr);
    } else {
        double gb = size / (1024.0 * 1024.0 * 1024.0);
        return QString("%1 GB (%2)").arg(gb, 0, 'f', 2).arg(sizeStr);
    }
}

void StatisticsDialog::onProgressChanged(const FileScanner::ScanResult &result)
{
    fileCountLabel->setText(QString("文件数量: %L1").arg(result.fileCount));
    dirCountLabel->setText(QString("目录数量: %L1").arg(result.directoryCount));

    int totalCount = result.fileCount + result.directoryCount;
    totalCountLabel->setText(QString("总数量: %L1").arg(totalCount));

    progressSizeLabel->setText("进度大小: " + formatSize(result.progressSize));
    totalSizeLabel->setText("总大小: " + formatSize(result.totalSize));
}

void StatisticsDialog::onFinished(const FileScanner::ScanResult &result)
{
    // 更新最终结果
    onProgressChanged(result);

    // 更新UI状态
    progressBar->setRange(0, 1);
    progressBar->setValue(1);
    stopButton->setEnabled(false);
    closeButton->setEnabled(true);
}

void StatisticsDialog::onStopClicked()
{
    if (scanner) {
        scanner->stop();
    }
    stopButton->setEnabled(false);
}
