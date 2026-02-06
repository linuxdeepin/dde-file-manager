// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "statisticsdialog.h"

#include <QVBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QStatusBar>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), treeView(nullptr), fileSystemModel(nullptr), statusLabel(nullptr)
{
    setupUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("FileScanner 测试 - 目录统计");
    resize(800, 600);

    // 创建中心部件
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // 创建文件系统模型（只显示目录，不显示隐藏目录）
    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setRootPath(QDir::rootPath());
    fileSystemModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System);

    // 创建树视图
    treeView = new QTreeView(this);
    treeView->setModel(fileSystemModel);
    treeView->setColumnHidden(1, true);   // 隐藏大小列
    treeView->setColumnHidden(2, true);   // 隐藏类型列
    treeView->setColumnHidden(3, true);   // 隐藏修改时间列
    treeView->setHeaderHidden(false);
    treeView->setAlternatingRowColors(true);
    treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);   // 支持Ctrl多选
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    // 设置根目录
    treeView->setRootIndex(fileSystemModel->index(QDir::rootPath()));

    // 连接右键菜单信号
    connect(treeView, &QTreeView::customContextMenuRequested,
            this, &MainWindow::onCustomContextMenu);

    layout->addWidget(treeView);

    // 状态栏
    statusLabel = new QLabel("提示: Ctrl+点击可多选目录，右键点击进行统计", this);
    statusBar()->addWidget(statusLabel);

    setCentralWidget(centralWidget);
}

void MainWindow::onCustomContextMenu(const QPoint &pos)
{
    QMenu menu(this);

    QAction *statTogetherAction = menu.addAction("统一统计选中目录");
    QAction *statSeparateAction = menu.addAction("分别统计选中目录");

    QAction *selectedAction = menu.exec(treeView->viewport()->mapToGlobal(pos));

    QList<QFileInfo> selectedInfos = getSelectedFileInfos();

    if (selectedInfos.isEmpty()) {
        return;
    }

    if (selectedAction == statTogetherAction) {
        // 统一统计模式：所有目录在一个对话框中统计
        QList<QUrl> urls;
        QStringList paths;
        for (const QFileInfo &info : selectedInfos) {
            urls.append(QUrl::fromLocalFile(info.absoluteFilePath()));
            paths.append(info.absoluteFilePath());
        }

        // 创建并显示统计对话框（非模态）
        StatisticsDialog *dialog = new StatisticsDialog(paths, urls, this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
        dialog->startScan();

    } else if (selectedAction == statSeparateAction) {
        // 分别统计模式：每个目录独立对话框，独立线程
        for (const QFileInfo &info : selectedInfos) {
            QList<QUrl> urls;
            urls.append(QUrl::fromLocalFile(info.absoluteFilePath()));

            QStringList paths;
            paths.append(info.absoluteFilePath());

            // 为每个目录创建独立的统计对话框
            StatisticsDialog *dialog = new StatisticsDialog(paths, urls, this);
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->show();
            dialog->startScan();
        }
    }
}

QList<QFileInfo> MainWindow::getSelectedFileInfos() const
{
    QList<QFileInfo> result;

    // 获取所有选中的索引
    QModelIndexList selectedIndexes = treeView->selectionModel()->selectedRows();

    for (const QModelIndex &index : selectedIndexes) {
        if (index.isValid()) {
            QFileInfo info = fileSystemModel->fileInfo(index);
            result.append(info);
        }
    }

    return result;
}
