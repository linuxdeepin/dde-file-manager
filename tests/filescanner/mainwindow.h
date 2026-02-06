// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QTreeView>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private Q_SLOTS:
    void onCustomContextMenu(const QPoint &pos);

private:
    void setupUI();
    QList<QFileInfo> getSelectedFileInfos() const;

    QTreeView *treeView;
    QFileSystemModel *fileSystemModel;
    QLabel *statusLabel;
};

#endif   // MAINWINDOW_H
